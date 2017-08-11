LIST="astar bzip2 cactusADM GemsFDTD gobmk lbm leslie3d mcf omnetpp perlbench sjeng soplex sphinx3 zeusmp"
DESIGN="options_620"
DESIGN1="options_566"
MY_DIR=`pwd`
for a in $LIST
do
	printf "%-15s\n" "$a,"
	ds=0
	for ds in $DESIGN
	do
		printf "%-15s\t" "$ds,"
		grep  "system.l3.tags.total_ranks" $a/$ds/stats.txt	| 	grep -v "system.l3.tags.total_ranks::0" | awk '{printf "%08d,", $2} END {if(!NR) printf "%9s\t", " "}' 
		echo ""
	done

	cd /ufrc/peir/qizeng/gem5/rest/
        ds1=0
        for ds1 in $DESIGN1
        do
		printf "%-15s\t" "$ds1,"
                grep  "system.l3.tags.total_ranks" $a/$ds1/stats.txt     |       grep -v "system.l3.tags.total_ranks::0" | awk '{printf "%08d,", $2} END {if(!NR) printf "%9s\t", " "}'
		echo ""
        done
	echo ""

	cd $MY_DIR
done
