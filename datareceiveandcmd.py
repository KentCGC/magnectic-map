import serial
import serial.tools.list_ports  # 添加這行
import csv
import queue
import re
import sys
import threading
import time
import os
from datetime import datetime
from math import sqrt

shared_data = queue.Queue()

num = 0
mag_X = 0.0
mag_Y= 0.0
mag_Z = 0.0
magnitude = 0.0
heading = 0.0
BAUD_RATE = 9600
telemetryData = []

def xbeecmd(ser, cmd):
        try:
            if cmd== "started":
                ser.write(b"y")
                print("XBee command sent: started")
            elif cmd == "paused":
                ser.write(b"n")
                print("XBee command sent: paused")
        except Exception as e:
            print(f"Error sending XBee command: {e}")
        
def readXBeeData():
    global f_raw, f_csv, w_csv, telemetryData
    
    if not shared_data.empty():
        data = shared_data.get()
        if data:
            f_raw.write(data + "\n")
            f_raw.flush()
            processedData = data.split(',')
            
            if len(processedData) < 7:  # 檢查數據長度
                print("Incomplete data received")
                return False
                
            if telemetryData:
                prev = telemetryData[-1]
            else:
                prev = None
                
            for idx in range(len(processedData)):
                if processedData[idx] == "" and prev is not None and idx < len(prev):
                    processedData[idx] = prev[idx]
                    
            telemetryData.append(processedData)
            w_csv.writerow(processedData)
            f_csv.flush()
            
            try:
                # 檢查數據格式
                float(processedData[0][0:2])  # 時間小時
                float(processedData[0][3:5])  # 時間分鐘
                float(processedData[0][6:8])  # 時間秒
                int(processedData[1])        # num
                float(processedData[2])       # Mag_X
                float(processedData[3])       # Mag_Y
                float(processedData[4])       # Mag_Z
                float(processedData[5])       # magnitude
                float(processedData[6])       # heading
                print("收到資料：", data)  # 新增這行，印出目前收到的資料
                return True
            except (ValueError, IndexError) as e:
                print(f"Telemetry_error: {e}")
                print(processedData)
                return False

def serialReader(ser):
    while True:
        try:
            line = ser.readline().decode('utf-8').strip()
            if line:
                shared_data.put(line)
        except Exception as e:
            print(f"Serial read error: {e}")
            break

def main():
    global f_raw, f_csv, w_csv

    sending_enabled = True  # 新增：資料傳送開關

    ports = serial.tools.list_ports.comports()
    for i, onePort in enumerate(ports):
        print(f"{i}: {onePort}")
    port_num = input("Input the serial port number: ")

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
        header = ['Time', 'num', 'Mag_X', 'Mag_Y', 'Mag_Z', 'magnitude', 'heading']

        with open(f'{save_path}\\Flight_itrik300_{current_time}.csv', 'w', encoding='UTF8', newline='') as f_csv, \
             open(f'{save_path}\\Flight_itrik300_{current_time}_Raw.txt', 'w') as f_raw:

            w_csv = csv.writer(f_csv)
            w_csv.writerow(header)
            f_csv.flush()
            print("按 y 啟動資料傳送，按 n 停止資料傳送，按 Ctrl+C 結束程式。")
            try:
                while True:
                    # 偵測鍵盤輸入切換開關
                    if sys.platform == "win32":
                        import msvcrt
                        if msvcrt.kbhit():
                            key = msvcrt.getch().decode('utf-8').lower()
                            if key == 'y':
                                if not sending_enabled:
                                    sending_enabled = True
                                    print("資料傳送啟動")
                                    xbeecmd(ser, "started")
                            elif key == 'n':
                                if sending_enabled:
                                    sending_enabled = False
                                    print("資料傳送暫停")
                                    xbeecmd(ser, "paused")
                    # 只有開啟時才處理資料
                    if sending_enabled and readXBeeData():
                        pass
                    time.sleep(0.01)
            except KeyboardInterrupt:
                print("Stopping...")

    except Exception as e:
        print(f"Error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
        print("Program ended.")

if __name__ == "__main__":
    main()

