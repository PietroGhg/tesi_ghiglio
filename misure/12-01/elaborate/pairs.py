file1 = open('pairs')
lines = file1.readlines()
file1.close()

i = 0
key = ""
tot = 0
res = dict()
for l in lines:
    if i == 0:
        key = l
        tot = 0
        i = 1
        d = dict()
    elif i == 1:
        if l == "\n":
            for entry in d:
                d[entry] = float(d[entry]) / tot
            res[key] = d
            i = 0
        else:
            s = l.split(':')
            k = s[0]
            v = int(s[1])
            tot = tot + v
            d[k] = v

acc = dict()
for k in res.keys():
    d = res[k]
    for pair in d.keys():
        if pair in acc:
            acc[pair] += d[pair]
        else:
            acc[pair] = d[pair]
#put entries with 0
for k in acc:
    for pair in res:
        if not(k in res[pair]):
            res[pair][k] = 0
            
N = len(res.keys())
acc_sorted = sorted(acc.items(), key = lambda kv : kv[1], reverse=True)
print("Average:")
avg = [[k[0], float(k[1])/N] for k in acc_sorted]
for k in avg:
    print(k[0], k[1])

var = dict()
for k in avg:
    s = 0
    for bench in res:
        d = res[bench]
        x = d[k[0]]
        xx = k[1]
        s = s + (x - xx)**2
    var[k[0]] = (1 / float(N-1))*s
var_sorted = sorted(var.items(), key = lambda kv : kv[1], reverse=True)
print("Variance:")
for k in var_sorted:
    print(k[0], k[1])

atax = res["atax 10 900\n"]
mm3 = res["3mm 100 1\n"]
def pp(t,d):
    print(t)
    for k in d:
        print(k, d[k])
    return

pp("\natax",atax)
pp("\n3mm",mm3)
