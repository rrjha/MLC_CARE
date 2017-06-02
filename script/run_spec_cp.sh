#!/bin/sh
#SBATCH --job-name=res_mlc    # Job name
#SBATCH --mail-type=ALL               # Mail events (NONE, BEGIN, END, FAIL, ALL
#SBATCH --nodes=4
#SBATCH --mail-user=<email_address>   # Where to send mail	
#SBATCH --ntasks=4                    # Run on a single CPU
#SBATCH --mem=8192mb                   # Memory limit
#SBATCH --time=72:05:00               # Time limit hrs:min:sec
#SBATCH --output=res_mlc_%j.out   # Standard output and error log
 
pwd; hostname; date
 
module load gcc python libz
export LIBRARY_PATH=/apps/gcc/5.2.0/python/2.7.10/lib:$LIBRARY_PATH
var1=$1
var2=$2
var3=$3 
#echo "Running plot script on a single CPU core"
# need change frm 3792 to 3782 due to 1B warmup
# benchnark #warmup million # run million
/home/rakeshjha/MLC_CARE/gem5/build/ALPHA/gem5.opt ~/gem5/configs/example/se.py --cpu-type=atomic --num-cpus=4 --cpu-clock=3.2GHz --caches --l2cache --l1d_size=32kB --l1i_size=32kB --l2_size=4MB --l1d_assoc=4 --l1i_assoc=4 --l2_assoc=8 --l3cache --l3_size=4MB --l3_assoc=32 --l3_tags=2 --cacheline_size=64 --mem-type=DDR3_1600_8x8 --mem-channels=2 --mem-size=8GB --at-instruction --checkpoint-dir=/home/rakeshjha/MLC_CARE/gem5/cpts --checkpoint-at-end --maxinsts=40400000000 --cmd="/home/rakeshjha/MLC_CARE/cpu2006/benchspec/CPU2006/429.mcf/run/run_base_ref_my-alpha.0000/mcf_base.my-alpha" --options="/home/rakeshjha/MLC_CARE/cpu2006/benchspec/CPU2006/429.mcf/run/run_base_ref_my-alpha.0000/inp.in"
date
