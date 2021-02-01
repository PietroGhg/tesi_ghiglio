from matplotlib import pyplot
import functools
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

file2 = open('est_01_02_1')
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
pyplot.subplot(2,1,1)
pyplot.plot(x,v)

#estimates may not be in the same order as the measures
print(est.keys())
est_fixed = dict()
for key in res:
    est_fixed[key] = est[key]
k2 = list(est_fixed.keys())
v2 = list(est_fixed.values())

pyplot.plot(x,v2,"g")
ratios = [ abs(est_fixed[key]-res[key])/res[key] for key in k]

for [i,key,e] in zip(range(len(k)), k, ratios):
    print(i,": ",key,e)

pyplot.subplot(2,1,2)
pyplot.ylim(0,1)
pyplot.plot(x,ratios)

total_error = functools.reduce(lambda x,y: x+y, ratios, 0)
print(total_error)
pyplot.show()

