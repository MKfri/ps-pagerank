import sys
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import Akima1DInterpolator


# --- PARAMETRI ---
#data_folder = "openmp-single-prec"
data_folder = "openmp-double-prec"
# 7 => Parallel sections time
COLUMN = 7
labels_list = ["Število iteracij", "Sortiranje", "Branje",
                "Priprava", "Pretvorba formata",
                "Računanje", "Zaporedni del",
                "Čas paralelnih sekcij", 
                "Skupni čas"]


def print_help():
    print("Skripta za izpis povprecnih casov")
    print("Vzame en obvezen in en opcijski parameter:\n")
    print("1) specificira format, mozne vrednosti:")
    print("csr, coo, hybrid, all\n")
    print("2) opcijski parameter, stolpec (stevilska vrednost)")
    exit(1)


available_formats = ["csr", "coo", "hybrid", "all"]
available_errors = ["err", "noerr"]
# Parse CLI params
cli_params = sys.argv

if len(cli_params) not in [2,3]:
    print("Napačno število parametrov\n")
    print_help()

chosen_format = cli_params[1].lower()

if (chosen_format not in available_formats):
    print("Napačno naveden format\n")
    print_help()


if len(cli_params) == 3:
    try:
        COLUMN = int(cli_params[2])
        if COLUMN > 8 or COLUMN < 0:
            raise ValueError
    except ValueError:
        print("Stolpec mora biti celo stevilo med 0 in 8")
        print_help()

# end of CLI param parsing


def make_pretty_error_chart(x_vals, y_vals, color, label):
    fmt = f"o{color}"

    plt.errorbar(x_vals, y_vals, label=label, barsabove=True, capsize=4, fmt=fmt)

    # Interpolacija
    akima_interp = Akima1DInterpolator(x_vals, y_vals)
    x_interp = np.linspace(x_vals.min(), x_vals.max(), 100)
    y_interp = akima_interp(x_interp)
    plt.plot(x_interp, y_interp, color, ls="dotted")




thread_num = [1, 2, 4, 8, 16, 32]
x = np.array(thread_num)

# COO
if (chosen_format in ["coo", "all"]):
    coo_avg_time = []

    for n in thread_num:
        file_name = f"{data_folder}/coo_64-{n}.txt"
        data = np.loadtxt(file_name, delimiter=",")
        coo_avg_time.append(np.average(data[:, COLUMN]))
    one_core_time = coo_avg_time[0]
    
    y_coo = np.array([one_core_time/(coo_avg_time[i]*thread_num[i]) for i in range(len(coo_avg_time))])
    
    make_pretty_error_chart(x, y_coo, 'g', "COO")

# CSR
if (chosen_format in ["csr", "all"]):
    csr_avg_time = []

    for n in thread_num:
        file_name = f"{data_folder}/csr_64-{n}.txt"
        data = np.loadtxt(file_name, delimiter=",")
        csr_avg_time.append(np.average(data[:, COLUMN]))
    one_core_time = csr_avg_time[0]

    y_csr = np.array([one_core_time/(csr_avg_time[i]*thread_num[i]) for i in range(len(csr_avg_time))])

    make_pretty_error_chart(x, y_csr, 'b', "CRS")


# Hybrid
if (chosen_format in ["hybrid", "all"]):
    hybrid_avg_time = []

    for n in thread_num:
        file_name = f"{data_folder}/hybrid_64-{n}.txt"
        data = np.loadtxt(file_name, delimiter=",")
        hybrid_avg_time.append(np.average(data[:, COLUMN]))
    one_core_time = hybrid_avg_time[0]

    y_hybrid = np.array([one_core_time/(hybrid_avg_time[i]*thread_num[i]) for i in range(len(hybrid_avg_time))])

    make_pretty_error_chart(x, y_hybrid, 'r', "Hybrid: COO+ELL")



plt.legend(loc='upper right')

plt.show()


