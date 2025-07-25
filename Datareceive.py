import serial
import csv
import queue
import re
import sys
import threading
import time
from datetime import datetime
from math import sqrt

shared_data = queue.Queue()

time= 0.0
num = 0
mag_X = 0.0
mag_Y= 0.0
mag_Z = 0.0
def addXBeeInput(input, data):  # Takes input (string) and appends it to data (list of lists)
    input = input.split(',')
    tempLine = []
    for item in input:
        tempLine.append(item)
    data.append(tempLine)
    return data
telemetryData = []  # Add this line to define telemetryData as a global list

def readXBeeData():  # Read data from serial port
        global f_raw  # Add this line to use the global f_raw variable
        global telemetryData  # Add this line to use the global telemetryData variable
        global w_csv  # Add this line to use the global w_csv variable
        global f_csv  # Add this line to use the global f_csv variable
        if not shared_data.empty():
            data = shared_data.get()
            if data:
                f_raw.write(data)
                f_raw.write("\n")
                f_raw.flush()
                processedData = data
                processedData = processedData.split(',')
                # 如果不是第一筆資料，沿用上一筆資料的數值
                if telemetryData:
                    prev = telemetryData[-1]
                else:
                    prev = None
                # 針對每個欄位做檢查
                for idx in range(len(processedData)):
                    if processedData[idx] == "" and prev is not None and idx < len(prev):
                        processedData[idx] = prev[idx]
                telemetryData.append(processedData)
                w_csv.writerow(processedData)
                f_csv.flush()
                try:
                    t_check=float(processedData[1][0:2])
                    t_check=float(processedData[1][3:5])
                    t_check=float(processedData[1][6:8])
                    t_check=int(processedData[2])
                    t_check=float(processedData[3])
                    t_check=float(processedData[4])
                    t_check=float(processedData[5])
                    t_check=float(processedData[6])
                    t_check=float(processedData[7])
                    
                    return True
                except:
                    print("Telemetry_error")
                    print(processedData)
                    return False
# 設定序列埠參數
SERIAL_PORT = 'COM8'  # 根據實際情況修改
BAUD_RATE = 9600
def main():
    global f_raw  # Add this line to make f_raw accessible globally

    save_path = '.'  # Set to current directory or specify your desired path, e.g., 'C:\\CFD\\ITRI\\magplot\\data'
    current_time = datetime.now().strftime('%Y%m%d_%H%M%S')
    header = ['Time', 'num', 'Mag_X', 'Mag_Y', 'Mag_Z', 'magnitude', 'heading']

    f_csv = open(save_path+'\\Flight_itrik300' + current_time +'.csv', 'w', encoding='UTF8', newline='')
    f_raw = open(save_path+'\\Flight_itrik300' + current_time +'_Raw.txt', 'w')
    w_csv = csv.writer(f_csv)
    w_csv.writerow(header)
    f_csv.flush()         
    ports = serial.tools.list_ports.comports()
    port_num = 0
    window_txt = ""
    for i, onePort in enumerate(ports):
        window_txt += str(i) + ":  " + str(onePort) + "\n"
        window_txt += str(i) + ":  " + str(onePort) + "\n"