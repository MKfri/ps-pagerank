

import math
import sys
import numpy as np
import matplotlib.pyplot as plt



# UK-2002
# Average: 17
# ___________________________________
# Threshold: 17 (Multiplier = 1)
# All elements: 298113762
# Ell elements: 80097750 <=> 3.518 GB
# Coo elements: 218016012
# % in ELL: 26.87 %
# Ell is 25.45 % full
# ___________________________________
# Threshold: 34 (Multiplier = 2)
# All elements: 298113762
# Ell elements: 102362754 <=> 7.037 GB
# Coo elements: 195751008
# % in ELL: 34.34 %
# Ell is 16.26 % full
# ___________________________________
# Threshold: 68 (Multiplier = 4)
# All elements: 298113762
# Ell elements: 125922273 <=> 14.074 GB
# Coo elements: 172191489
# % in ELL: 42.24 %
# Ell is 10.0 % full
# ___________________________________
# Threshold: 136 (Multiplier = 8)
# All elements: 298113762
# Ell elements: 152169013 <=> 28.149 GB
# Coo elements: 145944749
# % in ELL: 51.05 %
# Ell is 6.05 % full



# WEB-GOOGLE
# Average: 6
# ___________________________________
# Threshold: 6 (Multiplier = 1)
# All elements: 5105039
# Ell elements: 2116903 <=> 0.061 GB
# Coo elements: 2988136
# % in ELL: 41.47 %
# Ell is 38.5 % full

# Threshold: 12 (Multiplier = 2)
# All elements: 5105039
# Ell elements: 2877081 <=> 0.122 GB
# Coo elements: 2227958
# % in ELL: 56.36 %
# Ell is 26.17 % full

# Threshold: 24 (Multiplier = 4)
# All elements: 5105039
# Ell elements: 3543280 <=> 0.245 GB
# Coo elements: 1561759
# % in ELL: 69.41 %
# Ell is 16.12 % full

# Threshold: 48 (Multiplier = 8)
# All elements: 5105039
# Ell elements: 4040837 <=> 0.491 GB
# Coo elements: 1064202
# % in ELL: 79.16 %
# Ell is 9.19 % full



DRAW_LINES = True
LOG_SCALE = True



#ELL_THRESHOLD = 128
#file_name = "nnz-distrib-uk-2002.txt"
file_name = "nnz-distrib-web-google.txt"


data = np.loadtxt(file_name, delimiter=",")


num_rows = np.sum(data[:, 1])
NUM_ELEMENTS = int(np.dot(data[:,1], data[:,0]))

MULTIPLIER = 1
avg = math.ceil(NUM_ELEMENTS / num_rows)
ELL_THRESHOLD = avg * MULTIPLIER



elements_in_ell = 0

row_data = []

n = len(data)
for i in range(n):
    this_row = data[n - 1 - i]
    el_num = int(this_row[0])
    row_count = int(this_row[1])

    for k in range(row_count):
        row_data.append(el_num)
        elements_in_ell += min(ELL_THRESHOLD, el_num)

ellPerc = math.ceil(10000*elements_in_ell / NUM_ELEMENTS) / 100

ellFullness = math.ceil(10000*elements_in_ell / (ELL_THRESHOLD * num_rows)) / 100

print(f"Average: {avg}, Threshold: {ELL_THRESHOLD} (Multiplier = {MULTIPLIER})")
print(f"All elements: {NUM_ELEMENTS}")
print(f"Ell elements: {elements_in_ell} <=> {12 * (ELL_THRESHOLD * num_rows) / (1024**3)} GB")
print(f"Coo elements: {NUM_ELEMENTS - elements_in_ell}")
print(f"% in ELL: {ellPerc} %")
print(f"Ell is {ellFullness} % full")



x = np.array([i+1 for i in range(len(row_data))])
y = np.array(row_data)


fig, ax = plt.subplots()


b = ax.plot(x, y)
if LOG_SCALE:
    ax.set_yscale('log')

ax.fill_between(x, y)

if DRAW_LINES:
    for n in [1, 2, 4, 8]:
        y_line = np.array([ELL_THRESHOLD*n for i in range(len(row_data))])
        ax.plot(x, y_line, label=str(ELL_THRESHOLD*n))

ax.set_xlabel('Indeks v urejenem seznamu')
ax.set_ylabel('Dolžina vrstice')

plt.title('Zbirka web-google, dolžine vrstic urejene po velikosti z mejami ELL (logaritmična skala)')
plt.legend(loc='upper right')
plt.show()
