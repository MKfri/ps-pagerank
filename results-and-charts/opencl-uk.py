from platform import python_version_tuple
import sys
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import Akima1DInterpolator

# --- PARAMETRI ---
data_folder = "opencl-uk-2002"
# 7 => Parallel sections time
COLUMN = 5
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


available_formats = ["csr", "hybrid", "all"]
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


def make_pretty_error_chart(x_vals, y_vals, label):
    # Interpolacija
    #akima_interp = Akima1DInterpolator(x_vals, y_vals)
    #x_interp = np.linspace(x_vals.min(), x_vals.max(), 100)
    #y_interp = akima_interp(x_interp)
    #plt.plot(x_interp, y_interp, color, ls="dotted")
    plt.bar(x_vals, y_vals, label=label)


work_group = [64, 128, 192, 256, 512]
x = np.array(work_group)
r = np.arange(5)
povprecja = []
povprecja2 = []
povprecja3 = []
povprecja4 = []

for n in work_group:
    file_name = f"{data_folder}/csr_8-{n}.txt"
    data = np.loadtxt(file_name, delimiter=",")
    povprecja.append(np.average(data[:, COLUMN]))
    
for n in work_group:
    file_name = f"{data_folder}/csr_15-{n}.txt"
    data = np.loadtxt(file_name, delimiter=",")
    povprecja2.append(np.average(data[:, COLUMN]))

for n in work_group:
    file_name = f"{data_folder}/hybrid_8-{n}.txt"
    data = np.loadtxt(file_name, delimiter=",")
    povprecja3.append(np.average(data[:, COLUMN]))

for n in work_group:
    file_name = f"{data_folder}/hybrid_15-{n}.txt"
    data = np.loadtxt(file_name, delimiter=",")
    povprecja4.append(np.average(data[:, COLUMN]))
    

w=0.8
plt.bar(r, povprecja, w)
plt.bar(r+5, povprecja2, w)
plt.bar(r+10, povprecja3, w)
plt.bar(r+15, povprecja4, w)


plt.legend(loc='upper right')

plt.show()