
import math
import sys
import numpy as np
import matplotlib.pyplot as plt

# Pridobljeno preko "avg-sequential-time.py"
AVG_SEQUENTIAL_TIME = 118.0



def print_help():
    print("Skripta za izpis delezev sekvencnih in paralelnih operacij")
    print("Vzame dva obvezna paramatra:\n")
    print("1) specificira format, mozne vrednosti:")
    print("csr, coo, hybrid\n")
    print("2) Natancnost, mozni vrednosti:")
    print("single, double\n")
    exit(1)

available_formats = ["csr", "coo", "hybrid"]
precisions = {
    "single": "openmp-single-prec",
    "double": "openmp-double-prec"
}

# Parse CLI params
cli_params = sys.argv

if len(cli_params) != 3:
    print("Napačno število parametrov\n")
    print_help()

chosen_format = cli_params[1].lower()
precision = cli_params[2].lower()

if (chosen_format not in available_formats):
    print("Napačno naveden format\n")
    print_help()

if (precision not in precisions):
    print("Napačna natančnost\n")
    print_help()




thread_num = [1, 2, 4, 8, 16, 32]
parallel_percentages = []
sequential_percentages = []

for n in thread_num:
    file_name = f"{precisions[precision]}/{chosen_format}_64-{n}.txt"
    data = np.loadtxt(file_name, delimiter=",")
    avg = np.average(data[:, 7])
    paral = avg / (avg+AVG_SEQUENTIAL_TIME) * 100

    parallel_percentages.append(paral)
    sequential_percentages.append(100 - paral)

print(parallel_percentages)
print(sequential_percentages)




w = 0.8

threads_num_str = [str(n) for n in thread_num]

plt.bar(threads_num_str, sequential_percentages, w)
plt.bar(threads_num_str, parallel_percentages, w, bottom=sequential_percentages)



plt.show()