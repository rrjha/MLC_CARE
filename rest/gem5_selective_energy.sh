#LIST="astar bzip2 cactusADM GemsFDTD gobmk lbm leslie3d omnetpp perlbench sjeng soplex sphinx3 zeusmp"
#LIST="perlbench zeusmp sjeng"
LIST="zeusmp"
DESIGN="options_507 options_576"
#DESIGN="options_500"
for a in $LIST
do
	printf "%-15s\n" "$a,"
	ds=0
	for ds in $DESIGN
	do
        printf "%s, " $ds

		grep  "system.l3.tags.total_ZT::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}' 
		grep  "system.l3.tags.total_ST::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}'
		grep  "system.l3.tags.total_HT::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}' 
		grep  "system.l3.tags.total_TT::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}'
		#grep  "system.l3.tags.total_Reps::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}'
		grep  "system.l3.overall_miss_rate::total" $a/$ds/stats.txt	| 	awk '{printf "%1.6f, ", $2}'
		#grep  "system.l3.overall_miss_rate::total" $a/$ds/stats.txt	| 	awk '{printf "%1.6f", $2}'
		grep  "system.switch_cpus_1.ipc_total" $a/$ds/stats.txt	| 	awk '{printf "%1.6f", $2}'
		echo ""
	done

	echo ""
done
