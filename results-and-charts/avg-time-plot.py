import sys
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import Akima1DInterpolator


fig, ax = plt.subplots()

# --- PARAMETRI ---
data_folder = "openmp-single-prec"
#data_folder = "openmp-double-prec"
# 7 => Parallel sections time
COLUMN = 7
labels_list = ["Število iteracij", "Sortiranje", "Branje",
                "Priprava", "Pretvorba formata",
                "Računanje", "Zaporedni del",
                "Čas paralelnih sekcij", 
                "Skupni čas"]


def print_help():
    print("Skripta za izpis povprecnih casov")
    print("Vzame dva obvezna parametra:\n")
    print("1) specificira format, mozne vrednosti:")
    print("csr, coo, hybrid, all\n")
    print("2) pove ali naj je izpisan tudi std. odklon, mozni vrednosti:")
    print("err, noerr\n")
    print("3) opcijski parameter, stolpec (stevilska vrednost)")
    exit(1)


available_formats = ["csr", "coo", "hybrid", "all"]
available_errors = ["err", "noerr"]
# Parse CLI params
cli_params = sys.argv

if len(cli_params) not in [3,4]:
    print("Napačno število parametrov\n")
    print_help()

chosen_format = cli_params[1].lower()
chosen_error = cli_params[2].lower()

if (chosen_error not in available_errors or chosen_format not in available_formats):
    print("Napačna vrednost obveznih parametrov\n")
    print_help()

display_error = chosen_error == "err"

if len(cli_params) == 4:
    try:
        COLUMN = int(cli_params[3])
        if COLUMN > 8 or COLUMN < 0:
            raise ValueError
    except ValueError:
        print("Stolpec mora biti celo stevilo med 0 in 8")
        print_help()

# end of CLI param parsing


def make_pretty_error_chart(x_vals, y_vals, err, color, label):
    fmt = f"o{color}"
    if display_error:
        ax.errorbar(x_vals, y_vals, yerr=err, label=label, barsabove=True, capsize=4, fmt=fmt)
    else:
        ax.errorbar(x_vals, y_vals, label=label, barsabove=True, capsize=4, fmt=fmt)

    # Interpolacija
    akima_interp = Akima1DInterpolator(x_vals, y_vals)
    x_interp = np.linspace(x_vals.min(), x_vals.max(), 100)
    y_interp = akima_interp(x_interp)
    ax.plot(x_interp, y_interp, color, ls="dotted")




thread_num = [1, 2, 4, 8, 16, 32]
x = np.array(thread_num)

# COO
if (chosen_format in ["coo", "all"]):
    coo_avg_time = []
    coo_std_dev = []
    for n in thread_num:
        file_name = f"{data_folder}/coo_64-{n}.txt"
        data = np.loadtxt(file_name, delimiter=",")
        coo_avg_time.append(np.average(data[:, COLUMN]))
        coo_std_dev.append(np.std(data[:, COLUMN]))

    y_coo = np.array(coo_avg_time)
    coo_err = np.array(coo_std_dev)
    make_pretty_error_chart(x, y_coo, coo_err, 'g', "COO")

# CSR
if (chosen_format in ["csr", "all"]):
    csr_avg_time = []
    csr_std_dev = []
    for n in thread_num:
        file_name = f"{data_folder}/csr_64-{n}.txt"
        data = np.loadtxt(file_name, delimiter=",")
        csr_avg_time.append(np.average(data[:, COLUMN]))
        csr_std_dev.append(np.std(data[:, COLUMN]))

    y_csr = np.array(csr_avg_time)
    csr_err = np.array(csr_std_dev)
    make_pretty_error_chart(x, y_csr, csr_err, 'b', "CRS")


# Hybrid
if (chosen_format in ["hybrid", "all"]):
    hybrid_avg_time = []
    hybrid_std_dev = []
    for n in thread_num:
        file_name = f"{data_folder}/hybrid_64-{n}.txt"
        data = np.loadtxt(file_name, delimiter=",")
        hybrid_avg_time.append(np.average(data[:, COLUMN]))
        hybrid_std_dev.append(np.std(data[:, COLUMN]))

    y_hybrid = np.array(hybrid_avg_time)
    hybrid_err = np.array(hybrid_std_dev)
    make_pretty_error_chart(x, y_hybrid, hybrid_err, 'r', "Hybrid: COO+ELL")



ax.set_xlabel('Število jeder')
ax.set_ylabel('Čas izvajanja')

plt.title('OpenMP: Čas izvajanja (natančnost 1e-8)')

plt.legend(loc='upper right')

plt.show()


