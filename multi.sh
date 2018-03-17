# Run multiple instances of the solver in parallel

# Parameters needed:
# num_games = total number of games to play
# cores = number of cores to use
# run_name = name of screens for this batch of games

NUM_PARAMS=3

# Check that correct parameters are given
if [ "$#" -ne $NUM_PARAMS ]; then
    echo "Incorrect number of parameters"
    exit 1
fi

# Parameter order:
# num_games, cores, run_name

num_games=$1
cores=$2
run_name=$3

if [ $(( num_games % cores )) -ne 0 ]; then
    echo "Number of games not evenly divisible by core count"
    exit 1
fi

games_per_core=$(( num_games / cores ))

echo "Launching games..."

if [ -d tmp/ ]; then
    rm -r tmp/
fi

mkdir tmp/

for ((i=0; i<cores; i++)); do
    fname="tmp/res$i.txt"
    screen_name="$run_name$i"

    screen -S $screen_name -dm
    screen -S $screen_name -p 0 -X stuff "time ./a.out $games_per_core $fname^M"
done

echo "Launched $cores processes with $games_per_core games each."
