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

k = list(res.keys())
v = list(res.values())

pyplot.subplot(2,1,1)
pyplot.plot(k,v)

est = dict({200: 0.01576285,
            300: 0.02364425,
            400: 0.03152565,
            500: 0.03940705,
            600: 0.04728845,
            700: 0.05516985,
            800: 0.06305125,
            900: 0.07093265,
            1000: 0.07881405})

k2 = list(est.keys())
v2 = list(est.values())
v2 = [1.2 * x for x in v2]

pyplot.plot(k2,v2,"g")

ratios = [ e / m for [m,e] in zip(v,v2)]

pyplot.subplot(2,1,2)
pyplot.plot(k2, ratios)


            
pyplot.show()
