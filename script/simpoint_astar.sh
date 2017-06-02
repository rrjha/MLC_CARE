#!/bin/sh
#SBATCH --job-name=sp_job_test    # Job name
#SBATCH --mail-type=ALL               # Mail events (NONE, BEGIN, END, FAIL, ALL
#SBATCH --nodes=1
#SBATCH --mail-user=<email_address>   # Where to send mail	
#SBATCH --ntasks=1                    # Run on a single CPU
#SBATCH --mem=4080mb                   # Memory limit
#SBATCH --time=472:05:00               # Time limit hrs:min:sec
#SBATCH --output=sp_test_%j.out   # Standard output and error log
 
pwd; hostname; date
 
module load gcc python libz
export LIBRARY_PATH=/apps/gcc/5.2.0/python/2.7.10/lib:$LIBRARY_PATH
 
#echo "Running plot script on a single CPU core

var1=$1
var2=$2
cd /home/rakeshjha/MLC_CARE
#/home/rakeshjha/MLC_CARE/gem5/cp_gem5.sh $1
OUTPUT_DIR=/home/rakeshjha/MLC_CARE/gem5/simpoints
if [ ! -d "$OUTPUT_DIR/$var1" ]; then
mkdir $OUTPUT_DIR/$var1
fi
cd /home/rakeshjha/MLC_CARE/cpu2006/benchspec/CPU2006/473.astar/run/run_base_ref_my-alpha.0000/
/home/rakeshjha/MLC_CARE/gem5/build/ALPHA/gem5.opt --outdir=$OUTPUT_DIR/$var1 /home/rakeshjha/MLC_CARE/gem5/configs/example/se.py --simpoint-profile --simpoint-interval=200000000 --fastmem --cmd="/home/rakeshjha/MLC_CARE/cpu2006/benchspec/CPU2006/473.astar/run/run_base_ref_my-alpha.0000/astar_base.my-alpha" --options="rivers.cfg"
date
