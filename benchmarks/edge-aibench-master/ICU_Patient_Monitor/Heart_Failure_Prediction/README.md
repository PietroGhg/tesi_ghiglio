# References:
https://github.com/mp2893/retain
# Cut data:
```
python div.py
```
# Data generator:
```
python data_generator.py numpatients timeinterval
```
such as 
```
python data_generator.py 10 1
```
# Receive data:
```
python rec.py numpatients
```
such as
```
python rec.py 10
```
# Process data:
```
python ADMISSIONS.csv DIAGNOSES_ICD.csv PATIENTS.csv test
```
# Train:
```
python retain.py data.3digitICD9.seqs 942 data.morts result --simple_load --n_epochs 15 --keep_prob_context 0.8 --keep_prob_emb 0.5
```
# Test:
```
python test_retain.py result.10.npz test.3digitICD9.seqs test.morts test.3digitICD9.types res
```
