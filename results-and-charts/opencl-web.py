from platform import python_version_tuple
import sys
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import Akima1DInterpolator

# --- PARAMETRI ---
data_folder = "opencl-web-google"
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


#def make_pretty_error_chart(x_vals, y_vals, label):
    # Interpolacija
    #akima_interp = Akima1DInterpolator(x_vals, y_vals)
    #x_interp = np.linspace(x_vals.min(), x_vals.max(), 100)
    #y_interp = akima_interp(x_interp)
    #plt.plot(x_interp, y_interp, color, ls="dotted")
    #plt.bar(x_vals, y_vals, label=label)


work_group = [64, 128, 192, 256, 512]
x = np.array(work_group)
r = np.arange(5)
povprecja = []
povprecja2 = []
povprecja3 = []
povprecja4 = []
povprecja5 = []
povprecja6 = []
povprecja7 = []
povprecja8 = []

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


for n in work_group:
    file_name = f"{data_folder}/hybrid-2x_8-{n}.txt"
    data = np.loadtxt(file_name, delimiter=",")
    povprecja5.append(np.average(data[:, COLUMN]))

for n in work_group:
    file_name = f"{data_folder}/hybrid-2x_15-{n}.txt"
    data = np.loadtxt(file_name, delimiter=",")
    povprecja6.append(np.average(data[:, COLUMN]))

for n in work_group:
    file_name = f"{data_folder}/hybrid-4x_8-{n}.txt"
    data = np.loadtxt(file_name, delimiter=",")
    povprecja7.append(np.average(data[:, COLUMN]))

for n in work_group:
    file_name = f"{data_folder}/hybrid-4x_15-{n}.txt"
    data = np.loadtxt(file_name, delimiter=",")
    povprecja8.append(np.average(data[:, COLUMN]))

w=0.9

#plt.xticks(np.arange(8), [)
bar1 = plt.bar(r, povprecja, w)
bar2 =plt.bar(r+5+w, povprecja2, w)
bar3 =plt.bar(r+10+2*w, povprecja3, w)
bar4 =plt.bar(r+15+3*w, povprecja4, w)
bar5 =plt.bar(r+20+4*w, povprecja5, w)
bar6 =plt.bar(r+25+5*w, povprecja6, w)
bar7 =plt.bar(r+30+6*w, povprecja7, w)
bar8 =plt.bar(r+35+7*w, povprecja8, w)

naslovi = ['CSR (1e-8)', 'CSR (1e-15)', 'Hybrid (1e-8)','Hybrid (1e-15)','CSR (1e-15) float','Hybrid (1e-8) float']

naslovi = ['CSR (1e-8)', 'CSR (1e-15)', 'Hybrid (1e-8)','Hybrid (1e-15)','Hybrid2x (1e-8)','Hybrid2x (1e-15)','Hybrid4x (1e-8)','Hybrid4x (1e-15)']
iks = [2,8,14,20,26,32,38,44]
plt.xticks(iks, naslovi)
plt.title('WEB-GOOGLE')

#plt.xticks(r+w,['CSR_8', 'CSR_15', 'Hybrid_8','Hybrid_15','Hybrid2x_8','Hybrid2x_15','Hybrid4x_8','Hybrid4x_15',])
#plt.legend( (bar1, bar2, bar3), ('Player1', 'Player2', 'Player3') )




plt.show()