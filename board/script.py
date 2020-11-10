import argparse
import json

parser = argparse.ArgumentParser(description='cpi file to json')
parser.add_argument('-f')

args = parser.parse_args()

def fix(s):
    if(s == '' or s == ' ' or s == '\n'):
        return []

    splitted = s.split()
    l = len(splitted)-1
    if(l == 0):
        return []
    while(l > 0):
        if(splitted[l].isupper() and not('+' in splitted[l])):
            break
        l = l - 1
    return [splitted[l], splitted[-1]]

def addCond():
    terms = ["EQ","NE","GT","LT","GE","LE","CS","HS","CC","LO","MI","PL","AL","NV","VS","VC","HI","LS"]
    l = [["B"+c, "1+P"] for c in terms] 
    return l
    
def mkjson(l):
    return "{ \"opname\": " + "\"" + l[0] + "\", \"cost\": " + "\"" + l[1] + "\"}"

def worstCase(instr):
    cost = instr[1]
    newcost = ""
    if cost =="1+P":
        newcost = "4"
    elif cost == "2+P":
        newcost = "5"
    elif cost == "1+B":
        if instr[0] == "DSB" or instr[0] == "DMB" :
            newcost = "1"
        else:
            newcost = "4"
    elif cost == "1+W":
        newcost = "1"
    elif cost == "1+N":
        newcost = "2"
    elif cost == "1+N+P":
        newcost = "5"
    elif cost == "12" or len(cost) == 1:
        newcost = cost
    else:
        raise Exception('oh no')

    return [instr[0].lower(), newcost]
    

        
with open(args.f) as f:
    content = f.readlines()
    content = [x.replace('\t', ' ').strip() for x in content]
    content = [x.replace(' + ', '+') for x in content]
    content = [fix(x) for x in content]
    content = [x for x in content if len(x) > 0]
    content = content + addCond()
    content = [x for x in content if x[0] != 'B<CC>']
    content = [worstCase(x) for x in content]
    content = [mkjson(x) for x in content]
    s = '['
    for c in content:
        s += ',\n\t' + c
    s += '\n]'
    print(s)

