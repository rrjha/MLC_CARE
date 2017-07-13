LIST="astar bzip2 cactusADM GemsFDTD gobmk lbm leslie3d omnetpp perlbench sjeng soplex sphinx3 zeusmp mcf"
#DESIGN="options_550 options_507 options_521 options_560"
DESIGN="options_522 options_524"
for a in $LIST
do
	printf "%-15s\n" "$a,"
	ds=0
	for ds in $DESIGN
	do
        printf "%s, " $ds

		ls -l $a/$ds/stats.txt | awk '{printf "%s %s %s\n", $6, $7, $8}'
		echo ""
	done

	echo ""
done
