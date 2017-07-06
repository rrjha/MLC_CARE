LIST="astar bzip2 cactusADM GemsFDTD gobmk lbm leslie3d mcf omnetpp perlbench sjeng soplex sphinx3 zeusmp"
DESIGN="options_500 options_505 options_501 options_502 options_506 options_503 options_504"
for a in $LIST
do
	printf "%-15s\n" $a
	ds=0
	for ds in $DESIGN
	do
        printf "%s, " $ds
		grep  "system.switch_cpus_1.ipc_total" $a/$ds/stats.txt	| 	awk '{printf "%1.6f\n", $2}'
	done
	echo ""
done
