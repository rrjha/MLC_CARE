#sbatch restore_car4_2f.sh $1 $2 200
#sbatch restore_lru4_2f.sh $1 $2 200
#sbatch restore_car8_2f.sh $1 $2 200
#sbatch restore_lru8_2f.sh $1 $2 200
#sbatch restore_car8.sh $1 $2 200
#sbatch restore_lru8.sh $1 $2 200

sbatch restore_options.sh $1 $2 200 8
sbatch restore_options.sh $1 $2 200 9
sbatch restore_options.sh $1 $2 200 10 
sbatch restore_options.sh $1 $2 200 12

sbatch restore_options.sh $1 $2 200 50
sbatch restore_options.sh $1 $2 200 51
sbatch restore_options.sh $1 $2 200 53
sbatch restore_options.sh $1 $2 200 54
sbatch restore_options.sh $1 $2 200 42
sbatch restore_options.sh $1 $2 200 41
sbatch restore_options.sh $1 $2 200 40

sbatch restore_options.sh $1 $2 500 30
sbatch restore_options.sh $1 $2 500 31

sbatch restore_options.sh $1 $2 200 24
sbatch restore_options.sh $1 $2 200 25
#sbatch restore_options.sh $1 $2 200 40
#sbatch restore_options.sh $1 $2 200 40




#sbatch restore_lru.sh $1 $2 200
#sbatch restore_lru_nf.sh $1 $2 200
#sbatch restore_car4_2f.sh $1 $2 200
