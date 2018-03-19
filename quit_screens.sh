# Stop running named screens

# Parameters needed:
# screen name base
# number of screens

NUM_PARAMS=2

# Check all parameters are provided
if [ "$#" -ne $NUM_PARAMS ]; then
    echo "Incorrect number of parameters"
    exit 1
fi

# Parameter order:
# screen_name, num_screens

screen_name=$1
num_screens=$2

for ((i=0; i<num_screens; i++)); do
    echo "Stopping screen $screen_name$i"
    screen -S $screen_name$i -X quit
done
