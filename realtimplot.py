import serial
import serial.tools.list_ports
import csv
import queue
import re
import sys
import threading
import time
import os
from datetime import datetime
from math import sqrt
import matplotlib.pyplot as plt
import msvcrt
import multiprocessing
from collections import deque

shared_data = queue.Queue()

BAUD_RATE = 9600
telemetryData = deque(maxlen=50)
shared_data = queue.Queue(maxsize=50)  # 最多保留200筆


# 繪圖
batch_buffer = []

plot_time = deque(maxlen=20)
plot_x = deque(maxlen=20)
plot_y = deque(maxlen=20)
plot_z = deque(maxlen=20)
plot_mag = deque(maxlen=20)


def update_plot():
  
    if not telemetryData or len(telemetryData[-1]) < 9:
        return False
    try:
        recent_data = list(telemetryData)[-20:]
    
        plot_time.clear()
        plot_x.clear()
        plot_y.clear()
        plot_z.clear()
        plot_mag.clear()

        for latest in recent_data:
            if len(latest) < 9:  # 資料不完整就跳過p
                continue
            plot_time.append(latest[4])
            plot_x.append(float(latest[5]))
            plot_y.append(float(latest[6]))
            plot_z.append(float(latest[7]))
            plot_mag.append(float(latest[8]))

        # 清除舊圖
        plt.cla()
        # 畫四條線
        plt.plot(plot_time, plot_x, label='Mag_X')
        plt.plot(plot_time, plot_y, label='Mag_Y')
        plt.plot(plot_time, plot_z, label='Mag_Z')
        plt.plot(plot_time, plot_mag, label='Magnitude', linestyle='--')

        plt.legend(loc='upper right')
        plt.xlabel("num")
        plt.ylabel("magnitude(μT)")
        plt.title("Real-time Magnetometer Data")
        plt.xticks(rotation=45)
        plt.tight_layout()
        plt.pause(0.2)  # 讓圖表更新

    except Exception as e:
        print(f"Plot update error: {e}")

def serialReader(ser):
    while True:
        try:
            line = ser.readline().decode('utf-8').strip()
            if line:
                if shared_data.full():
                    shared_data.get_nowait()  
                shared_data.put(line)
        except Exception as e:
            print(f"Serial read error: {e}")
            break

MAX_DATA_LEN = 200
def readXBeeData():
    global f_raw, f_csv, w_csv, telemetryData
    
    if not shared_data.empty():
        data = shared_data.get()
        if data:
            f_raw.write(data + "\n")
            f_raw.flush()
            processedData = data.split(',')
            
            if len(processedData) < 10:  # 檢查數據長度
                print(processedData)
                return False
                    
            telemetryData.append(processedData)
            if len(telemetryData) > MAX_DATA_LEN:
                telemetryData = telemetryData[-MAX_DATA_LEN:]

            w_csv.writerow(processedData)
            f_csv.flush()
            
            try:
                float(processedData[0][0:2])  # 時間小時
                float(processedData[0][3:5])  # 時間分鐘
                float(processedData[0][6:8])  # 時間秒
                int(processedData[1])         # num
                int(processedData[2])         # x
                int(processedData[3])         # y
                int(processedData[4])         # z
                float(processedData[5])       # Mag_X
                float(processedData[6])       # Mag_Y
                float(processedData[7])       # Mag_Z
                float(processedData[8])       # magnitude
                float(processedData[9])       # heading
                
                #print("收到資料：", data)
                return True
            except (ValueError, IndexError) as e:
                print(f"Telemetry_error: {e}")
                print(processedData)
                return False



def processDataThread():
    global batch_buffer, telemetryData
    BATCH_SIZE = 20
    while True:
        try:
            data = shared_data.get(timeout=1)
            if not data:
                continue
            processedData = data.split(',')
            telemetryData.append(processedData)
            batch_buffer.append(processedData)
            if len(batch_buffer) >= BATCH_SIZE:
                w_csv.writerows(batch_buffer)
                f_csv.flush()
                batch_buffer.clear()
        except queue.Empty:
            continue
        except Exception as e:
            print(f"Process error: {e}")

def xbeecmd(ser, cmd):
        try:
            if cmd== "started":
                ser.write(b"y")
                print("XBee command sent: started")
                time.sleep(0.1)
                pos_input = input("input position : ")
                
                if re.match(r"^-?\d+\s+-?\d+\s+-?\d+$", pos_input.strip()):
                    ser.write((pos_input.strip() + '\n').encode('utf-8'))
                    print(f"position sent：{pos_input}")
                else:
                    print("error1")
            elif cmd == "paused":
                ser.write(b"n")
                print("XBee command sent: paused")
                time.sleep(0.1)
            
        except Exception as e:
            print(f"Error sending XBee command: {e}")
        


def main():
    global plotting_enabled
    global f_raw, f_csv, w_csv
    global sending_enabled
    plotting_enabled =False 
    sending_enabled = True  
    last_update = 0
    UPDATE_INTERVAL = 0.2
    threading.Thread(target=processDataThread, daemon=True).start()

    ports = serial.tools.list_ports.comports()
    for i, onePort in enumerate(ports):
        print(f"{i}: {onePort}")
    port_num = input("Input the serial port number a: ")

    try:
        SERIAL_PORT = ports[int(port_num)].device
    except (IndexError, ValueError):
        print("Invalid port number. Exiting.")
        sys.exit(1)

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        threading.Thread(target=serialReader, args=(ser,), daemon=True).start()
        

        save_path = '.'
        current_time = datetime.now().strftime('%m%d_%H%M%S')
        header = ['Time','x','y','z', 'num', 'Mag_X', 'Mag_Y', 'Mag_Z', 'magnitude', 'heading']

        with open(f'{save_path}\\Flight_itrik300_3D.csv', 'w', encoding='UTF8', newline='') as f_csv, \
             open(f'{save_path}\\Flight_itrik300_3D_raw.txt', 'w') as f_raw:

            w_csv = csv.writer(f_csv)
            w_csv.writerow(header)
            f_csv.flush()

            plt.ion()  # 啟動互動模式
            fig = plt.figure()
            

            print("按 y 啟動資料傳送 按 n 停止資料傳送按 Ctrl+C 結束程式。。")
            try:
                while True:
                    if sys.platform == "win32":    
                        if msvcrt.kbhit():
                            key = msvcrt.getch().decode('utf-8').lower()
                            if key == 'y':
                                if not sending_enabled:
                                    sending_enabled = True
                                    print("資料傳送啟動")
                                    xbeecmd(ser, "started")
                                  
                                    plotting_enabled = True
                            elif key == 'n':
                                if sending_enabled:
                                    sending_enabled = False
                                    print("資料傳送暫停")
                                    xbeecmd(ser, "paused")
                                    plotting_enabled = False

                    if sending_enabled and readXBeeData():
                        now = time.time()
                        if now - last_update > UPDATE_INTERVAL:
                            update_plot()  
                            last_update = now
            except KeyboardInterrupt:
                print("Stopping...")
            finally:
                plt.ioff()  # 關閉互動模式
                plt.show()

    except Exception as e:
        print(f"Error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
        print("Program ended.")

if __name__ == "__main__":
    main()
