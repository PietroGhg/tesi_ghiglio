# -*- coding: utf-8 -*-
"""
Created on Mon Jun 17 11:11:46 2019

@author: 温旭
"""

from tkinter import *
import paramiko
#import rec
import os 
import time
import threading 
import pandas as pd
from tkinter import ttk
from tkinter.scrolledtext import ScrolledText

#connect with remote server
cwd=os.getcwd()
ssh = paramiko.SSHClient()
key = paramiko.AutoAddPolicy()
ssh.set_missing_host_key_policy(key)
ssh.connect('39.105.144.147', 2333, 'root', '123456' ,timeout=5)

class Scrollable(ttk.Frame):
    """
       Make a frame scrollable with scrollbar on the right.
       After adding or removing widgets to the scrollable frame, 
       call the update() method to refresh the scrollable area.
    """

    def __init__(self, frame, width=20):

        scrollbar = tk.Scrollbar(frame, width=width)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y, expand=False)

        self.canvas = tk.Canvas(frame, yscrollcommand=scrollbar.set)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        scrollbar.config(command=self.canvas.yview)

        self.canvas.bind('<Configure>', self.__fill_canvas)

        # base class initialization
        tk.Frame.__init__(self, frame)         

        # assign this obj (the inner frame) to the windows item of the canvas
        self.windows_item = self.canvas.create_window(0,0, window=self, anchor=tk.NW)


    def __fill_canvas(self, event):
        "Enlarge the windows item to the canvas width"

        canvas_width = event.width
        self.canvas.itemconfig(self.windows_item, width = canvas_width)        

    def update(self):
        "Update the canvas and the scrollregion"

        self.update_idletasks()
        self.canvas.config(scrollregion=self.canvas.bbox(self.windows_item))
        
def print_patient():
    #read csv files
    patient=pd.read_csv("PATIENTS.csv")
    admission=pd.read_csv("AMDISSIONS.csv")
    diagnose=pd.read_csv("DIAGNOSES_ICD.csv")
    tabControl = ttk.Notebook(win)
    for i in patient['SUBJECT_ID'] :
        p=patient[patient['SUBJECT_ID']==i]
        a=admission[admission['SUBJECT_ID']==i]
        d=diagnose[diagnose['SUBJECT_ID']==i]
        #print patient information to doctors
        tab=ttk.Frame(tabControl)
        tabControl.add(tab, text='patient'+str(i))  
        
        pi = ttk.LabelFrame(tab, text='patient'+str(i), width=800, height=800)
        pi.pack(fill='both',expand=1)
        patientinfo=Scrollable(pi, width=20)
        ttk.Label(patientinfo, text="Gender:"+p['GENDER'].iloc[0]).grid(row=0,column=0)
        ttk.Label(patientinfo, text="Birth time:"+str(p['DOB'].iloc[0])).grid(row=1,column=0)         
        ttk.Label(patientinfo, text="Death time:"+str(p['DOD'].iloc[0])).grid(row=2,column=0)
        n=0
        #one patient may have many hospital admissions
        for j in a['HADM_ID']: 
            aa = a[a['HADM_ID']==j]
            dd = d[d['HADM_ID']==j]            
            n=n+1                      
            admissioninfo = ttk.LabelFrame(patientinfo, text='hospital admission'+str(n))
            admissioninfo.grid()
            ttk.Label(admissioninfo, text="diagnose sequence").grid(row=0,column=1)
            
            ttk.Label(admissioninfo, text="Admission time:"+aa['ADMITTIME'].iloc[0]).grid(row=0,column=0)          
            ttk.Label(admissioninfo, text="Discharge time:"+aa['DISCHTIME'].iloc[0]).grid(row=1,column=0)         
            ttk.Label(admissioninfo, text="Admission type:"+aa['ADMISSION_TYPE'].iloc[0]).grid(row=2,column=0)
            ttk.Label(admissioninfo, text="Admission location:"+aa['ADMISSION_LOCATION'].iloc[0]).grid(row=3,column=0)
            ttk.Label(admissioninfo, text="Discharge location:"+aa['DISCHARGE_LOCATION'].iloc[0]).grid(row=4,column=0)
            ttk.Label(admissioninfo, text="Fast diagnose:"+aa['DIAGNOSIS'].iloc[0]).grid(row=5,column=0)
            #one admission has many diagnoses
            
            num=0
            for k in dd['ICD9_CODE']:
                num=num+1
                ttk.Label(admissioninfo,text=k).grid(row=num,column=1)
        patientinfo.update()
    tabControl.pack(expand=1, fill="both")
    
def run_generator():
    #run data_generator.py remotely and rec.py locally 
    npatient=pentry.get()
    print(npatient)
    time_interval=tentry.get()
    print(time_interval)
    #stdin1, stdout1, stderr1=ssh.exec_command('python3 /home/wx/physionet.org/works/MIMICIIIClinicalDatabase/files/version_1_4/data_generator.py 10 0.01')
    #stdin2, stdout2, stderr2=ssh.exec_command('python3 /home/wx/ICU_IOT/rec.py')          
    """
    transport = paramiko.Transport(('39.105.144.147',2333))
    transport.connect(username='root',password='123456')
    sftp = paramiko.SFTPClient.from_transport(transport)
    sftp.get('/home/wx/ICU_IOT/ADMISSIONS.csv',cwd+"/AMDISSIONS.csv")
    sftp.get('/home/wx/ICU_IOT/DIAGNOSES_ICD.csv',cwd+"/DIAGNOSES_ICD.csv")
    sftp.get('/home/wx/ICU_IOT/PATIENTS.csv',cwd+"/PATIENTS.csv")
    """
    print_patient()
    #print_patient()
    

win = Tk()
win.title("ICU patient generator")
win.geometry("600x600")

#title
label = Label(win,
                      text="ICU patient generator",
                     # bg="pink", fg="red",
                      font=("黑体", 20),
                     # width=20,
                     # height=10,
                     # wraplength=100,
                      #justify="left",
                     # anchor="ne"
                      )
label.pack()

frame = Frame(win)
frame.pack()

frame_pnum=Frame(frame)
frame_pnum.pack(side='left')
#patient number
Label(frame_pnum,text='number of patients:',font=10).grid(row=0,column=0)
pentry=Entry(frame_pnum,show=None,font=('Arial', 10))
pentry.grid(row=0,column=1)


#time interval
Label(frame_pnum,text='time interval:',font=10).grid(row=1,column=0)
tentry=Entry(frame_pnum, show=None,font=('Arial', 10))
tentry.grid(row=1,column=1)

#generate button
generate_button=Frame(frame)
generate_button.pack(side='right')
button = Button(generate_button,text="generate",command=run_generator,width=10,height=2).pack()


win.mainloop()
