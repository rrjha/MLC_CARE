LIST="astar bzip2 cactusADM GemsFDTD gobmk lbm leslie3d mcf omnetpp perlbench sjeng soplex sphinx3 zeusmp"
DESIGN="options_450 options_451"
for a in $LIST
do
	printf "%-15s\t" "$a,"
	ds=0
	for ds in $DESIGN
	do
		grep  "system.l3.tags.total_ranks" $a/$ds/stats.txt	| 	grep -v "system.l3.tags.total_ranks::0" | awk '{printf "%08d,", $2} END {if(!NR) printf "%9s\t", " "}' 
	done

	echo ""
done
