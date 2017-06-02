#!/bin/sh
#SBATCH --job-name=rest_options    # Job name
#SBATCH --mail-type=ALL               # Mail events (NONE, BEGIN, END, FAIL, ALL
#SBATCH --nodes=1
#SBATCH --mail-user=<email_address>   # Where to send mail	
#SBATCH --ntasks=1                    # Run on a single CPU
#SBATCH --mem=4080mb                   # Memory limit
#SBATCH --time=72:05:00               # Time limit hrs:min:sec
#SBATCH --output=res_op_%j.out   # Standard output and error log
#SBATCH  --qos=peir-b
pwd; hostname; date
 
module load gcc python libz
export LIBRARY_PATH=/apps/gcc/5.2.0/python/2.7.10/lib:$LIBRARY_PATH
var1=$1
var2=$2
var3=$3 
var4=$4
default=1000000
sum=$(($var2 * $default))
num2=$3
sum2=$(($var3 * $default))
#echo "Running plot script on a single CPU core"
# need change frm 3792 to 3782 due to 1B warmup
/home/rakeshjha/MLC_CARE/gem5/run_options_cp.sh $var1 /home/rakeshjha/MLC_CARE/gem5/rest $sum $sum2 $var4
date
