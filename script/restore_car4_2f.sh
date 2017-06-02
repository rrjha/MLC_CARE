#!/bin/sh
#SBATCH --job-name=res_mlc    # Job name
#SBATCH --mail-type=ALL               # Mail events (NONE, BEGIN, END, FAIL, ALL
#SBATCH --nodes=1
#SBATCH --mail-user=<email_address>   # Where to send mail	
#SBATCH --ntasks=1                    # Run on a single CPU
#SBATCH --mem=4080mb                   # Memory limit
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
/home/rakeshjha/MLC_CARE/gem5/run_restore4_2f.sh $1 $2 $3
date
