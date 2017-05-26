 # arg1 sec arg2 length
# sbatch restore_sec.sh lbm 90000 $2 $1
 #sbatch restore_sec.sh bwaves 800000 $2 $1
 sbatch restore_sec.sh leslie3d 220000 $2 $1
 sbatch restore_sec.sh bzip2 379200 $2 $1
 sbatch restore_sec.sh mcf 306000 $2 $1
 sbatch restore_sec.sh GemsFDTD 430000 $2 $1
 sbatch restore_sec.sh omnetpp 377200 $2 $1
 #sbatch restore_sec.sh hmmer 331300 $2 $1
 sbatch restore_sec.sh zeusmp 350000 $2 $1
 sbatch restore_sec.sh gobmk 53000 $2 $1
 sbatch restore_sec.sh astar 900000 $2 $1
 sbatch restore_sec.sh lbm 60000 $2 $1
 sbatch restore_sec.sh perlbench 380000 $2 $1
 sbatch restore_sec.sh cactusADM 100000 $2 $1
 #sbatch restore_sec.sh h264ref 527100 $2 $1
 sbatch restore_sec.sh soplex 210000 $2 $1
 sbatch restore_sec.sh sphinx3 2200000 $2 $1

 sbatch restore_sec.sh sjeng 1200000 $2 $1
 #sbatch restore_sec.sh tonto 930000 $2 $1
