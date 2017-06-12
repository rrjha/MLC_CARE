#! /bin/sh

iter=1
option=$1
length=$2
incr=$4

while [ $iter -le $3 ]
do
	sbatch restore_options.sh omnetpp 377200 $length $option
	#echo $length $option
        let length=$length+$incr
	let option=$option+1
	let iter=$iter+1
done
