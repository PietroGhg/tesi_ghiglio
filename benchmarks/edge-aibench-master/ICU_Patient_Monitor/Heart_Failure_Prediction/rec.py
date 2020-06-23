import threading
from socket import *
import sys
import os
HOST = "10.206.10.98"
PORT = 23333
ADDR = (HOST,PORT)

client = socket(AF_INET,SOCK_STREAM)
client.connect(ADDR)
def rec_static(client,fileout):
    with open(fileout+".csv","ab") as f:
        while True:
            data = client.recv(1024)
            if not data:
                break;
            f.write(data)
    f.close()
    print(fileout+" finished")
    client.close()
if os.path.exists("PATIENTS.csv"):
    os.remove("PATIENTS.csv")
if os.path.exists("ADMISSIONS.csv"):
    os.remove("ADMISSIONS.csv")
if os.path.exists("DIAGNOSES_ICD.csv"):
    os.remove("DIAGNOSES_ICD.csv")

rec_static(client,"PATIENTS")
client = socket(AF_INET,SOCK_STREAM)
client.connect(ADDR)
rec_static(client,"ADMISSIONS")
client = socket(AF_INET,SOCK_STREAM)
client.connect(ADDR)
rec_static(client,"DIAGNOSES_ICD")
nad=0
with open("ADMISSIONS.csv") as f:
    for i in f:
        nad=nad+1
nad=nad-1
for i in range(nad):
#threads num depend on
    client = socket(AF_INET,SOCK_STREAM)
    client.connect(ADDR)
    thread=threading.Thread(target=rec_static,args=(client,"DIAGNOSES_ICD"))
    thread.start()
    #rec_static(client,"DIAGNOSES_ICD")
