#! /usr/bin/env python

#Paste the median column of all the results

import os,sys

col = 5 
res = {}
files = os.listdir('.')
out = open("summary.result", 'w')

for file_name in files:
    if file_name[0:3] != 'log':
        continue
    for line_nr, line in enumerate(open(file_name)):
        res.setdefault(line_nr, []).append(line.strip().split('\t')[col-1])

    out.write (file_name[4:-7]+'\t')
out.write ('\n')
for line_nr in sorted(res):
    out.write ('\t'.join(res[line_nr]))
    out.write ('\n')

out.close()
