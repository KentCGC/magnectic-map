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

shared_data = queue.Queue()

num = 0
mag_X = 0.0
mag_Y= 0.0
mag_Z = 0.0
magnitude = 0.0
heading = 0.0
BAUD_RATE = 9600
telemetryData = []



def readXBeeData():
    global f_raw, f_csv, w_csv, telemetryData
    
    if not shared_data.empty():
        data = shared_data.get()
        if data:
            f_raw.write(data + "\n")
            f_raw.flush()
            processedData = data.split(',')
            
            if len(processedData) < 7: 
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
                float(processedData[0][0:2])  # hour
                float(processedData[0][3:5])  # minute
                float(processedData[0][6:8])  # second
                int(processedData[1])        # num
                float(processedData[2])       # Mag_X
                float(processedData[3])       # Mag_Y
                float(processedData[4])       # Mag_Z
                float(processedData[5])       # magnitude
                float(processedData[6])       # heading
                print("收到資料：", data)  
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
            try:
                while True:
                    if readXBeeData():
                        pass  #
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
               
