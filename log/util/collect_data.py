import os,sys
import numpy as np
out  =open(sys.argv[1] + '.result', 'w')
out.write('idx\tfilename\tmaxLat\tavgLat\tmedianLat\t999PLat\tCV')
os.chdir('../'+ sys.argv[1])
filename = os.listdir('.')
filename.sort()
for fi in filename:
    if fi[0] is '.':
        continue
    out.write('\n')
    for a in fi:
        if a.isdigit():
            out.write(a)

    log = open(fi,'r')
    out.write('\t' +fi + '\t')
    line = log.readline()
    while line[0][0] is '#':
        line = log.readline()
    
    num = int(line)
    data = np.arange(num)
    for i in range (0,num):
        data[i] = log.readline()
    _mean = np.mean(data)
    _cv = np.std(data) /_mean
    _mid = np.median(data)
    _max = np.amax(data)
    data.sort()
    _999p = data[int(num*0.999)-1]
    out.write(str(_max) + "\t%.2f\t" %_mean +"%.2f\t" %_mid +str(_999p) +"\t%.2f" %_cv)
out.close() 


             

