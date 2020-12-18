from matplotlib import pyplot
file1 = open('measures')
lines=file1.readlines()

i = 0
start = 0
end = 0
label = ''
res = dict()
for l in lines:
    c = i % 5
    if(c == 0):
        label = int(l[:-1].split()[-1])
    if(c == 1):
        start = float(l.split()[2])
    if(c == 2):
        end = float(l.split()[2])
    if(c == 3):
        delta = end - start
        res[label] = delta
        start = 0
        end = 0
        label = ''
    if(c == 4):
        pass
    i = i + 1

    """
for k in res:
    pyplot.plot(k,res[k], 'go--', linewidth=2, markersize=12)
"""
k = list(res.keys())
v = list(res.values())
print(k)
print(v)
pyplot.plot(k,v)


pyplot.show()
