
import numpy as np

COLUMN = 6

folder_names = ["openmp-single-prec", "openmp-double-prec"]
available_formats = ["csr", "coo", "hybrid"]
thread_num = [1, 2, 4, 8, 16, 32]


measurements = []

for folder in folder_names:
    for fmt in available_formats:
        for n in thread_num:
            file_name = f"{folder}/{fmt}_64-{n}.txt"
            data = np.loadtxt(file_name, delimiter=",")
            relevant_data = data[:, COLUMN]
            measurements.extend(relevant_data.tolist())


np_measurements = np.array(measurements)

print(f"Avg = {np.average(np_measurements)}; Std. dev = {np.std(np_measurements)}")