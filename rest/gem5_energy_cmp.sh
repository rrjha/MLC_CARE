LIST="astar bzip2 cactusADM GemsFDTD gobmk lbm leslie3d mcf omnetpp perlbench sjeng soplex sphinx3 zeusmp"
DESIGN="options_620"
DESIGN1="options_512"
DESIGN2="options_3"
DESIGN3="options_566"
MY_DIR=`pwd`
for a in $LIST
do
	printf "%-15s\n" "$a,"
	ds=0
	for ds in $DESIGN
	do
        printf "%-15s" "$ds,"

		grep  "system.l3.tags.total_ZT::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}' 
		grep  "system.l3.tags.total_ST::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}'
		grep  "system.l3.tags.total_HT::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}' 
		grep  "system.l3.tags.total_TT::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}'
		#grep  "system.l3.tags.total_Reps::total" $a/$ds/stats.txt	| 	awk '{printf "%010d, ", $2}'
		grep  "system.l3.overall_miss_rate::total" $a/$ds/stats.txt	| 	awk '{printf "%1.6f, ", $2}'
		grep  "system.switch_cpus_1.ipc_total" $a/$ds/stats.txt	| 	awk '{printf "%1.6f", $2}'
		echo ""
	done

	ds1=0
	cd /ufrc/peir/qizeng/gem5/rest
        for ds1 in $DESIGN1
        do
        printf "%-15s" "$ds1,"

                grep  "system.l3.tags.total_ZT::total" $a/$ds1/stats.txt |       awk '{printf "%010d, ", $2}'
                grep  "system.l3.tags.total_ST::total" $a/$ds1/stats.txt |       awk '{printf "%010d, ", $2}'
                grep  "system.l3.tags.total_HT::total" $a/$ds1/stats.txt |       awk '{printf "%010d, ", $2}'
                grep  "system.l3.tags.total_TT::total" $a/$ds1/stats.txt |       awk '{printf "%010d, ", $2}'
                #grep  "system.l3.tags.total_Reps::total" $a/$ds1/stats.txt       |       awk '{printf "%010d, ", $2}'
                grep  "system.l3.overall_miss_rate::total" $a/$ds1/stats.txt     |       awk '{printf "%1.6f, ", $2}'
                grep  "system.switch_cpus_1.ipc_total" $a/$ds1/stats.txt |       awk '{printf "%1.6f", $2}'
                echo ""
        done

	ds2=0
	cd /home/qizeng/gem5_twostep/rest
        for ds2 in $DESIGN2
        do
        printf "%-15s" "$ds2,"

                grep  "system.l3.tags.total_ZT::total" $a/$ds2/stats.txt |       awk '{printf "%010d, ", $2}'
                grep  "system.l3.tags.total_ST::total" $a/$ds2/stats.txt |       awk '{printf "%010d, ", $2}'
                grep  "system.l3.tags.total_HT::total" $a/$ds2/stats.txt |       awk '{printf "%010d, ", $2}'
                grep  "system.l3.tags.total_TT::total" $a/$ds2/stats.txt |       awk '{printf "%010d, ", $2}'
                #grep  "system.l3.tags.total_Reps::total" $a/$ds2/stats.txt       |       awk '{printf "%010d, ", $2}'
                grep  "system.l3.overall_miss_rate::total" $a/$ds2/stats.txt     |       awk '{printf "%1.6f, ", $2}'
                grep  "system.switch_cpus_1.ipc_total" $a/$ds2/stats.txt |       awk '{printf "%1.6f", $2}'
                echo ""
        done

	ds3=0
	cd /ufrc/peir/qizeng/gem5/rest/
        for ds3 in $DESIGN3
        do
        printf "%-15s" "$ds3,"

                grep  "system.l3.tags.total_ZT::total" $a/$ds3/stats.txt |       awk '{printf "%010d, ", $2}'
                grep  "system.l3.tags.total_ST::total" $a/$ds3/stats.txt |       awk '{printf "%010d, ", $2}'
                grep  "system.l3.tags.total_HT::total" $a/$ds3/stats.txt |       awk '{printf "%010d, ", $2}'
                grep  "system.l3.tags.total_TT::total" $a/$ds3/stats.txt |       awk '{printf "%010d, ", $2}'
                #grep  "system.l3.tags.total_Reps::total" $a/$ds3/stats.txt       |       awk '{printf "%010d, ", $2}'
                grep  "system.l3.overall_miss_rate::total" $a/$ds3/stats.txt     |       awk '{printf "%1.6f, ", $2}'
                grep  "system.switch_cpus_1.ipc_total" $a/$ds3/stats.txt |       awk '{printf "%1.6f", $2}'
                echo ""
        done


        echo ""

	cd $MY_DIR
done
