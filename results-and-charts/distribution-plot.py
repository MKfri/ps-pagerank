

import math
import sys
import numpy as np
import matplotlib.pyplot as plt


file_name = "nnz-distrib-uk-2002.txt"

data = np.loadtxt(file_name, delimiter=",")

#x = data[0:100000, 0]
num_rows = np.sum(data[:, 1])
print(num_rows)
print(np.dot(data[:,1], data[:,0]))


row_data = []

n = len(data)
for i in range(n):
    this_row = data[n - 1 - i]
    el_num = int(this_row[0])
    row_count = int(this_row[1])

    for k in range(row_count):
        row_data.append(el_num)

print(len(row_data))

print(row_data[0])

#my_x = row_data
#my_y = []
"""

base = 1000
a = 100
b = 1

for k in range(len(row_data)):
    " ""
    if k > a:
        a *= base
        b *= base
    if k % b == 0:
        my_x.append(k)
        my_y.append(row_data[k])
    " ""
    #if k < 100 or k % 10 == 0:
    if True:
        my_x.append(k+1)
        my_y.append(row_data[k])
"""









"""
plt.loglog(x, y)
plt.show()
"""


y = np.array(row_data)

fig, ax = plt.subplots()
#ax.set_xticks(x + dimw / 2, labels=map(str, x))
#b = ax.plot(my_x, my_y, 1) #, bottom=0.001)
b = ax.plot(y) #, bottom=0.001)
#ax.set_yscale('log')
#ax.set_xscale('log')
#ax.fill_between(my_x, my_y)
#ax.fill_between(y.index, y)

ax.set_xlabel('x')
ax.set_ylabel('y')

plt.show()


##avg = np.average(data[:, 7])
#paral = avg / (avg+AVG_SEQUENTIAL_TIME) * 100

#parallel_percentages.append(paral)
#sequential_percentages.append(100 - paral)