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

est = dict({100: 0.01115136,
            200: 0.02230266,
            300: 0.03345396,
            400: 0.04460526,
            500: 0.05575656,
            600: 0.06690786,
            700: 0.07805916,
            800: 0.08921046,
            900: 0.1003618,
            1000: 0.115131})
k2 = list(est.keys())
v2 = list(est.values())

pyplot.plot(k2,v2,"g")

ratios = [ e / m for [m,e] in zip(v,v2)]

pyplot.subplot(2,1,2)
pyplot.plot(k2, ratios)


            
pyplot.show()
