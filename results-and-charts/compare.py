from platform import python_version_tuple
import sys
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import Akima1DInterpolator

data_folder = "opencl-uk-2002"

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



work_group = [64, 128, 192, 256, 512]
x = np.array(work_group)
r = np.arange(1)
povprecja = []
povprecja2 = []



file_name = f"{data_folder}/csr_15-{64}.txt"
data = np.loadtxt(file_name, delimiter=",")
povprecja.append(np.average(data[:, COLUMN]))
    
COLUMN = 7    
data_folder = "openmp-double-prec"

file_name = f"{data_folder}/coo_64-{32}.txt"
data = np.loadtxt(file_name, delimiter=",")
povprecja2.append(np.average(data[:, COLUMN]))


naslovi = ['opencl', 'openmp']

iks = [0,1.8]
plt.xticks(iks, naslovi)
plt.title('Primerjava opencl in openmp, double, 1e-15')

#plt.xticks(r+w,['CSR_8', 'CSR_15', 'Hybrid_8','Hybrid_15','Hybrid2x_8','Hybrid2x_15','Hybrid4x_8','Hybrid4x_15',])
#plt.legend( (bar1, bar2, bar3), ('Player1', 'Player2', 'Player3') )
w=0.9
bar1 = plt.bar(r, povprecja, w)
bar2 =plt.bar(r+w+w, povprecja2, w)



plt.show()