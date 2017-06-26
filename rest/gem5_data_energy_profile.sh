LIST="astar bzip2 cactusADM GemsFDTD gobmk lbm leslie3d mcf omnetpp perlbench sjeng soplex sphinx3 zeusmp"
DESIGN="options_430"
for a in $LIST
do
	printf "%-15s\t\n," $a
	ds=0
	for ds in $DESIGN
	do
		grep  "system.l3.tags.dd_profile_data" $a/$ds/stats.txt	| grep -v "system.l3.tags.dd_profile_data::total" |	awk '{printf "%5.0f,\t", $2} END {if(!NR) printf "%9s\t", " "}'
		printf "\n,"
		grep  "system.l3.tags.du_profile_data" $a/$ds/stats.txt	| grep -v "system.l3.tags.du_profile_data::total" |	awk '{printf "%5.0f,\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		printf "\n,"
		grep  "system.l3.tags.ud_profile_data" $a/$ds/stats.txt	| grep -v "system.l3.tags.ud_profile_data::total" |	awk '{printf "%5.0f,\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		printf "\n,"
		grep  "system.l3.tags.uu_profile_data" $a/$ds/stats.txt	| grep -v "system.l3.tags.uu_profile_data::total" |	awk '{printf "%5.0f,\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		printf "\n,"
	done

	echo ""
done
