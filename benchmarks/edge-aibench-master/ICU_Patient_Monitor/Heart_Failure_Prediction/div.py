import pandas as pd
#divide data into patients
def file_div(filein,groupkey):
    tmp=pd.read_csv(filein+".csv",dtype=object)
    for name,group in tmp.groupby(groupkey):
        group.to_csv(filein+"/"+str(name)+".csv",index=False)
#print(diagnose_icd)
file_div("DIAGNOSES_ICD","HADM_ID")
