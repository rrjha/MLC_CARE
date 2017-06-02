#!/bin/sh
#SBATCH --job-name=cp_job_test    # Job name
#SBATCH --mail-type=ALL               # Mail events (NONE, BEGIN, END, FAIL, ALL
#SBATCH --nodes=1
#SBATCH --mail-user=<email_address>   # Where to send mail	
#SBATCH --ntasks=2                    # Run on a single CPU
#SBATCH --mem=4080mb                   # Memory limit
#SBATCH --time=72:05:00               # Time limit hrs:min:sec
#SBATCH --output=cp_test_%j.out   # Standard output and error log
 
pwd; hostname; date
 
module load gcc python libz
export LIBRARY_PATH=/apps/gcc/5.2.0/python/2.7.10/lib:$LIBRARY_PATH
 
#echo "Running plot script on a single CPU core"

/home/rakeshjha/MLC_CARE/gem5/cp_gem5.sh mcf 3060
date
