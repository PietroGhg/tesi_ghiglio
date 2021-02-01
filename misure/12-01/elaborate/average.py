import csv


def avg(path, start_time, end_time):
    f = open(path)
    reader = csv.reader(f, delimiter = ',')
    tot_current = 0
    n = 0
    i = 0
    for row in reader:
        if i == 0:
            i = 1
            continue
        timestamp = float(row[0])
        current = float(row[1])
        if timestamp >= start_time:
            tot_current += current
            n += 1
        if timestamp >= end_time:
            break
        i += 1
            
    return tot_current / n

def ratio(path, start, mid, end):
    avg1 = avg(path, start, mid)
    avg2 = avg(path, mid, end)
    print(avg1, avg2, avg1/avg2)
    return

ratio('../2mm', 6.035, 6.254, 8.470)
ratio('../gemver_300_1', 5.742, 6.140, 6.574)
ratio('../symm_100_1', 6.717, 6.832, 8.147)
ratio('../syr2k_100_1', 6.254, 6.389, 7.924)

