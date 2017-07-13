LIST="perlbench sjeng zeusmp"
DESIGN="options_500 options_521 options_522 options_523 options_524 options_528"
for a in $LIST
do
	printf "%-15s\n" $a
	ds=0
	for ds in $DESIGN
	do
        printf "%s, " $ds

		grep  "system.l3.tags.total_ZT::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}' 
		grep  "system.l3.tags.total_ST::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}'
		grep  "system.l3.tags.total_HT::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}' 
		grep  "system.l3.tags.total_TT::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}'
		grep  "system.l3.tags.total_Reps::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}'
		grep  "system.l3.overall_miss_rate::total" $a/$ds/stats.txt	| 	awk '{printf "%1.6f, ", $2}'
		grep  "system.switch_cpus_1.ipc_total" $a/$ds/stats.txt	| 	awk '{printf "%1.6f", $2}'
		echo ""
	done

	echo ""
done
