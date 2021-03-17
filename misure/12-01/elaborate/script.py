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

estname = 'est_iio'
file2 = open(estname)
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

res['symm 10 100'] = res['symm 10 100'] / 4.0
k = list(res.keys())
v = list(res.values())
x = [str(i) for i in range(len(k))]

p1 = pyplot.subplot(2,1,1)
#pyplot.plot(x,v,label='measure')
pyplot.bar(x,v,color='blue',label='measure')
pyplot.xticks(x, k, rotation='vertical')
pyplot.subplots_adjust(bottom=0.05, hspace=0.99999)
p1.set_ylabel('energy', loc='center')

#estimates may not be in the same order as the measures
est_fixed = dict()
for key in res:
    est_fixed[key] = est[key]
est_fixed['symm 10 100'] = est_fixed['symm 10 100'] / 4.0
k2 = list(est_fixed.keys())
v2 = list(est_fixed.values())

#pyplot.plot(x,v2,"g",label='estimate')
pyplot.bar(x,v2,color='red', label='estimate')
p1.legend()
ratios = [ abs(est_fixed[key]-res[key])/res[key] for key in k]

def pretty(n):
    return '{0:.4f}'.format(n)

for [i,key,mes, est, err] in zip(range(len(k)), k, v, v2, ratios):
    """
    s = key.split()
    print(s[0], ' & ', s[1], ' & ', s[2], ' & ', pretty(mes), ' & ', pretty(est), ' & ', pretty(err), '\\\\')
    print('\\hline')
    """
    print(key, err)

p2 = pyplot.subplot(2,1,2)
pyplot.ylim(0,1)
#pyplot.plot(x,ratios)
pyplot.bar(x,ratios)
pyplot.xticks(x, ['' for e in x])
p2.set_ylabel('error %', loc='center')



total_error = float(functools.reduce(lambda x,y: x+y, ratios, 0)) / len(k)
print(total_error)
pyplot.savefig(estname + '.png')
pyplot.show()

