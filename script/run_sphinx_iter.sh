#! /bin/sh

iter=1
option=$1
length=$2

while [ $iter -le $3 ]
do
	let length=$2*$iter
	sbatch restore_options.sh sphinx3 2200000 $length $option
	#echo $length $option
	let option=$option+1
	let iter=$iter+1
done
