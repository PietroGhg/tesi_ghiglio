from socket import *
import _thread
import sys
import time
import pandas as pd
import os
import threading

def send_seqindex(skt,filein,ID):
    #send index of sequence data
    with open(filein+'/'+str(ID)+'.csv', 'rb') as f:
        first_line=next(f)
        skt.send(first_line)
    f.close()

def transfer_seq(skt,filein,ID,timeinterval):
    #ID refers to Hospital admission ID
    #transfer sequence data with a time interval
    skt,addr = server.accept()
    print(filein+str(ID)+' started')
    with open(filein+'/'+str(ID)+'.csv', 'rb') as f:
        next(f)
        for data in f:
            skt.send(data)
            time.sleep(timeinterval)
    f.close()
    skt.close()
    
    #transfer data unchanged at the beginning
def transfer_static(server,filein):
    
    skt,addr = server.accept()
    print(filein+' started')
    with open(filein+".csv",'rb')as f:
        for data in f:
            skt.send(data)
    f.close()
    skt.close()

if __name__ == '__main__':
    HOST = "0.0.0.0"
    PORT = 23333
    ADDR = (HOST,PORT)
    npatients = int(sys.argv[1])
    #number of patients
    timeinterval = float(sys.argv[2])
    #time interval
    server = socket(AF_INET,SOCK_STREAM)
    server.bind(ADDR)
    #server.listen(npatients)
    
    
    print("generating patients")
    patient=pd.read_csv("PATIENTS.csv",dtype=object)
    admission=pd.read_csv("ADMISSIONS.csv",dtype=object)
    patient_selected = patient.sample(n=npatients,random_state=1)
    SUBJECT_ID=patient_selected['SUBJECT_ID']
    #use SUBJECT_ID to selecet admission
    #notice that a patient has only one SUBJECT_ID but maybe more than one HADM_ID
    #skt,addr = server.accept()
    admission_selected = admission[admission['SUBJECT_ID'].isin(SUBJECT_ID)]
    
    HADM_ID = admission_selected['HADM_ID']
    nad=admission_selected.shape[1]
    server.listen(nad)
    #use HADM_ID to find location of seqence data
    patient_selected.to_csv("patient_selected.csv",index=False)
    admission_selected.to_csv("admission_selected.csv",index=False)

    transfer_static(server,'patient_selected')
    transfer_static(server,'admission_selected')
    if os.path.exists("patient_selected.csv"):
        os.remove("patient_selected.csv")
    if os.path.exists("admission_selected.csv"):
        os.remove("admission_selected.csv")
    #_thread.start_new_thread(transfer_admission,(skt,addr,patient_selected))
    skt,addr = server.accept()
    send_seqindex(skt,'DIAGNOSES_ICD',100001)
    skt.close()
    for hid in HADM_ID:
        #skt,addr = server.accept()
        #_thread.start_new_thread(transfer_seq,(skt,'DIAGNOSES_ICD',hid,timeinterval))
        thread=threading.Thread(target=transfer_seq,args=(skt,'DIAGNOSES_ICD',hid,timeinterval))
        thread.start()
        #transfer_seq(skt,'DIAGNOSES_ICD',hid,timeinterval)
    
    
