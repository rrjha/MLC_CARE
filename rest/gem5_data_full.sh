LIST="gobmk astar mcf bzip2 h264 hmmer libquantum"
LIST="gobmk astar mcf bzip2 h264 hmmer libquantum perlbench gcc xalancbmk lbm milc sphinx3 soplex leslie3d bwaves"
LIST="gobmk astar mcf bzip2 h264 hmmer libquantum perlbench gcc xalancbmk lbm milc sphinx3 soplex leslie3d bwaves"
LIST="bzip2 mcf soplex perlbench milc lbm astar bwaves hmmer leslie3d gcc cactus xalancbmk gobmk h264"
LIST="zeusmp GemsFDTD gobmk bzip2 astar h264ref lbm leslie3d mcf omnetpp perlbench"
LIST="lbm leslie3d"
#astar/         bzip2/         gem5_data.sh   GemsFDTD/      gobmk/         h264ref/       lbm/           leslie3d/      mcf/           omnetpp/       perlbench/
#LIST="bzip2 mcf soplex perlbench milc lbm astar bwaves hmmer leslie3d gcc cactus"
#for a in $LIST
#	do
#	echo "$a"
#	grep -A 9 -w "0 times" '/media/fd538cb5-97ec-46b1-829f-6aaae01875e3/qi/trace_cache/count_mix/noduel4_mix_LW32_r4_trace-'$a'_16M_32way_f8e8s128d100000t47.txt'
#	done
DESIGN="car2b lru2b car4_2f lru4_2f car8_f1 lru8_f1 car8_2f lru8_2f"
#echo "                                       mlc                                        lru"
for a in $LIST
do
	printf "%-15s\n" $a
	ds=0
#	sed '2q;d' 'mix_encodingNLRU_LW'$ds'_r4_trace-'$a'_16M_32way_f0_t12.txt'	| 	awk '{printf "%5.2f\t", $4} END {if(!NR) printf "%9s\t", " "}' 
#	sed '3q;d' 'mix_encodingNLRU_LW'$ds'_r4_trace-'$a'_16M_32way_f0_t12.txt'	| 	awk '{printf "%5.2f\t", $4} END {if(!NR) printf "%9s\t", " "}' 
#	sed '4q;d' 'mix_encodingNLRU_LW'$ds'_r4_trace-'$a'_16M_32way_f0_t12.txt'	| 	awk '{printf "%5.2f\t", $4} END {if(!NR) printf "%9s\t", " "}' 
#	sed '5q;d' 'mix_encodingNLRU_LW'$ds'_r4_trace-'$a'_16M_32way_f0_t12.txt'	| 	awk '{printf "%5.2f\t", $4} END {if(!NR) printf "%9s\t", " "}' 
	#sed '17q;d' 'mix_encodingNLRU_LW'$ds'_r4_trace-'$a'_16M_32way_f0_t25.txt'	| 	awk '{printf "%9.4f\t", $4} END {if(!NR) printf "%9s\t", " "}' 
	for ds in $DESIGN
	do
		printf "%-8s\t" $ds 		
		grep  "system.l3.tags.avg_ZT::257" $a/$ds/stats.txt	| 	awk '{printf "%5.2f\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		grep  "system.l3.tags.avg_ST::257" $a/$ds/stats.txt	| 	awk '{printf "%5.2f\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		grep  "system.l3.tags.avg_HT::257" $a/$ds/stats.txt	| 	awk '{printf "%5.2f\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		grep  "system.l3.tags.avg_TT::257" $a/$ds/stats.txt	| 	awk '{printf "%5.2f\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		grep  "system.l3.tags.avg_ZT::65" $a/$ds/stats.txt	| 	awk '{printf "%5.2f\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		grep  "system.l3.tags.avg_ST::65" $a/$ds/stats.txt	| 	awk '{printf "%5.2f\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		grep  "system.l3.tags.avg_HT::65" $a/$ds/stats.txt	| 	awk '{printf "%5.2f\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		grep  "system.l3.tags.avg_TT::65" $a/$ds/stats.txt	| 	awk '{printf "%5.2f\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		grep  "system.l3.tags.total_ZT::total" $a/$ds/stats.txt	| 	awk '{printf "%5.0f\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		grep  "system.l3.tags.total_ST::total" $a/$ds/stats.txt	| 	awk '{printf "%5.0f\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		grep  "system.l3.tags.total_HT::total" $a/$ds/stats.txt	| 	awk '{printf "%5.0f\t", $2} END {if(!NR) printf "%9s\t", " "}' 
		grep  "system.l3.tags.total_TT::total" $a/$ds/stats.txt	| 	awk '{printf "%5.0f\t", $2} END {if(!NR) printf "%9s\t", " "}'
		grep  "system.l3.overall_miss_rate::total" $a/$ds/stats.txt	| 	awk '{printf "%1.6f\t", $2} END {if(!NR) printf "%9s\t", " "}'
		grep  "system.switch_cpus_1.ipc_total" $a/$ds/stats.txt	| 	awk '{printf "%1.6f\t", $2} END {if(!NR) printf "%9s\t", " "}'

		
		#grep  "system.l3.tags.avg_ZT::257" $a/$ds/stats.txt	| 	awk '{printf "%5.2f\t", $2} END {if(!NR) printf "%9s\t", " "}' 
#		sed '2q;d' 'mix_encodingNLRU_LW'$ds'_r4_trace-'$a'_16M_32way_f4_t12.txt'	| 	awk '{printf "%5.2f\t", $4} END {if(!NR) printf "%9s\t", " "}' 
#		sed '3q;d' 'mix_encodingNLRU_LW'$ds'_r4_trace-'$a'_16M_32way_f4_t12.txt'	| 	awk '{printf "%5.2f\t", $4} END {if(!NR) printf "%9s\t", " "}' 
#		sed '4q;d' 'mix_encodingNLRU_LW'$ds'_r4_trace-'$a'_16M_32way_f4_t12.txt'	| 	awk '{printf "%5.2f\t", $4} END {if(!NR) printf "%9s\t", " "}' 
#		sed '5q;d' 'mix_encodingNLRU_LW'$ds'_r4_trace-'$a'_16M_32way_f4_t12.txt'	| 	awk '{printf "%5.2f\t", $4} END {if(!NR) printf "%9s\t", " "}' 
#		sed '13q;d' 'mix_encodingNLRU_LW'$ds'_r4_trace-'$a'_16M_32way_f4_t12.txt'	| 	awk '{printf "%4.4f\t", $4} END {if(!NR) printf "%9s\t", " "}' 
#		sed '19q;d' 'mix_encodingNLRU_LW'$ds'_r4_trace-'$a'_16M_32way_f4_t12.txt'	| 	awk '{printf "%4.4f\t", $4} END {if(!NR) printf "%9s\t", " "}' 
#		sed '19q;d' 'mix_encodingNLRU_LW'$ds'_r4_trace-'$a'_16M_32way_f4_t12.txt'	| 	awk '{printf "%4.4f\t", $9} END {if(!NR) printf "%9s\t", " "}' 
		echo ""
	done

	echo ""
done
