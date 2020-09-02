import math
def pi(x,y):
    return (1/2)*(x+y)*(x+y+1) + y

def pi_inv(z):
    w = math.floor( (math.sqrt(8.0*float(z) + 1.0) - 1.0) / 2.0 )
    t = (w*w + w) / 2.0
    y_new = z - t;
    x_new = w - y_new;
    return (x_new, y_new)

def triple_to_single(x,y,z):
    return pi(pi(x,y), z)

def single_to_triple(n):
    (xy, z) = pi_inv(n)
    (x,y) = pi_inv(xy)
    return (x,y,z)

for i in range(1000000):
    triple = single_to_triple(i)
    if(triple_to_single(triple[0], triple[1], triple[2]) != i):
        print("argh")
