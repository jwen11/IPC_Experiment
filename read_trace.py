# read trace results for IPC Experiments
# extract functions that run for a long time
# by Jiyang Chen
# 2016.9.14

import sys
import os

if len(sys.argv) > 2:
	print "argument too many"
	sys.exit()

with open(sys.argv[1]) as dFile:
    data = dFile.readlines()

start = 0
start_flag = 0
end = 0
i = 0
count = 1
stack = []
stack_print = []
directory = "pro_result"

# need more initilization here

for i in range(len(data)):
	line = data[i]

	if "about to write" in line:			# find the trace marker
		start_flag = 1
		# print line
	if "sys_write" in line:
		# print i
		if start_flag:
			start = i						# find the start point
			start_flag = 0
		end = i
	if "finished write" in line:			# the previous sys_write is the finish point
		break

# print start
# print end

data = data[start:end]

data.reverse()					# start from end, so I can know which lines take too long


for line in data:
	if line[4] == '+' or line[4] == '!':				# get lines with long time
		stack.append([i, count])
		i = data.index(line)
		stack_print.append(i)			# add index to print stack
		count = 1
		continue
	if line[-2] == '}':
		count += 1
		# print line
	if line[-2] == '{':
		count -= 1
		# print line
		if count == 0:					# find the corresponding bracket
			stack_print.append(data.index(line))
			# stack_print.append(data.index(line))
			if len(stack) > 0:
				[i, count] = stack.pop()	# finished one frame
stack_print.reverse()

if not os.path.exists(directory):
    os.makedirs(directory)

sys.stdout = open(directory + '/' + sys.argv[1] + '.txt', 'w')
sys.stdout.write("Trace result for " + sys.argv[1] + "\n\n")
for i in stack_print:
	sys.stdout.write(data[i])				# print the stack
	# print data[i]
print ''

sys.stdout.close()



