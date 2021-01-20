gemver = {
    "ldr": 5590564,
    "vmov": 4550041,
    "vldr": 2337122,
    "str": 2079112,
    "bl": 1690959,
    "b": 1563495,
    "add.w": 1558441,
    "mov": 1428881,
    "vstr": 1168923,
    "adds": 522001,
    "cmp": 521646,
    "mov.w": 518762,
    "bge": 391325,
    "mla": 388801,
    "ldr.w": 259214,
    "movs": 2173,
    "str.w": 1828,
    "add": 26,
    "pop": 21,
    "push": 21,
    "sub": 21,
    "movt": 3,
    "pop.w": 2,
    "push.w": 2,
    "movw": 2,
    "mul": 1,
    "muls": 1,
    "nop": 1,
    "sdiv": 1,
    "bge.w": 1,
    "bgt.w": 1,
    "mls": 1}

_2mm = {
"ldr": 36373231,
"vmov": 20749501,
"str": 16563339,
"add.w": 15459102,
"mov": 15064522,
"vldr": 9648502,
"b": 8530667,
"bl": 6990522,
"mla": 4081001,
"adds": 2893931,
"vstr": 2856003,
"cmp": 2851767,
"bge": 2851666,
"mov.w": 2774201,
"ldr.w": 2640009,
"rsb": 1320001,
"mls": 40501,
"sdiv": 40501,
"muls": 27306,
"movs": 25078,
"str.w": 11014,
"add": 16,
"push": 13,
"pop": 13,
"sub": 13,
"movw": 6,
"movt": 2,
"mul": 1,
"bgt": 1}

def getTot(d):
    tot = 0
    for s in d:
        tot += d[s]
    return tot

def getPct(d):
    tot = getTot(d)
    res = dict()
    for s in d:
        res[s] = float(d[s]) / tot
    return res

def pr(d):
    for s in d:
        print(s, " ", d[s])
    return

pr(getPct(gemver))
print()
pr(getPct(_2mm))
