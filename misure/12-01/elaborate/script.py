from matplotlib import pyplot
file1 = open('measures')
lines=file1.readlines()
file1.close()

i = 0
start = 0
end = 0
label = ''
res = dict()
for l in lines:
    c = i % 5
    if(c == 0):
        #label = int(l[:-1].split()[-1])
        label = l[:-1]
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

file2 = open('estimates2')
lines = file2.readlines()
file2.close()
est = dict()
label = ''
i = 0
for l in lines:
    c = i % 3
    if(c == 0):
        label = l[:-1]
    if(c == 1):
        est[label] = float(l)
    if(c == 2):
        pass
    i = i + 1


k = list(res.keys())
v = list(res.values())
x = [str(i) for i in range(len(k))]
print(x)
pyplot.subplot(2,1,1)
pyplot.plot(x,v)


k2 = list(est.keys())
v2 = list(est.values())

pyplot.plot(x,v2,"g")

ratios = [ abs(e-m) / m for [m,e] in zip(v,v2)]
for [k,e] in zip(k, ratios):
    print(k,e)

pyplot.subplot(2,1,2)
pyplot.plot(x,ratios)


            
pyplot.show()

