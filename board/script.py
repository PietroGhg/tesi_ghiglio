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

def mkjson(l):
    return "{ \"opname\": " + "\"" + l[0] + "\", \"cost\": " + "\"" + l[1] + "\"}"



        
with open(args.f) as f:
    content = f.readlines()
    content = [x.replace('\t', ' ').strip() for x in content]
    content = [x.replace(' + ', '+') for x in content]
    content = [fix(x) for x in content]
    content = [x for x in content if len(x) > 0]
    content = [mkjson(x) for x in content]
    s = '{['
    for c in content:
        s += ',\n\t' + c
    s += '\n]}'
    print(s)
    #print(json.dumps(content))
