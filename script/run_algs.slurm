#!/bin/bash
#SBATCH --job-name=planning
#SBATCH --partition=arrow
#SBATCH --array=1-776
#SBATCH --time=01:20:00 # time limit per job!
#SBATCH --mem=14GB
#SBATCH --error=logs_set_var_78/%A_%a.err
#SBATCH --output=logs_set_var_78/%A_%a.out

# warm up processors
sudo cpupower frequency-set -g performance

sleep 0.1

stress-ng -c 4 --cpu-ops=100

# set limits
ulimit -v 16777216

# COMMANDS TO EXECUTE

# Number of tasks per job
TASKS_PER_JOB=64

# Compute the range of lines for this job
START_LINE=$(( (SLURM_ARRAY_TASK_ID - 1) * TASKS_PER_JOB + 1 ))
END_LINE=$(( START_LINE + TASKS_PER_JOB - 1 ))

# Read and execute the assigned tasks
sed -n "${START_LINE},${END_LINE}p" tasks.txt | while read INSTANCE ALGORITHM SEED START END; do
    BASENAME="${INSTANCE::-4}"

    case "$ALGORITHM" in
        0) MAPPED_ALGO="random" ;;
        1) MAPPED_ALGO="greedy" ;;
        2) MAPPED_ALGO="pruning" ;;
        3) MAPPED_ALGO="lookahead" ;;
        4) MAPPED_ALGO="backprop_min" ;;
        5) MAPPED_ALGO="backprop_max" ;;
        6) MAPPED_ALGO="backprop_sum" ;;
        7) MAPPED_ALGO="re-apply_backprop_min" ;;
        8) MAPPED_ALGO="backprop_min_ucs" ;;
        *) echo "Unknown algorithm: $ALGORITHM"; exit 1 ;;
    esac

    OUTPUT_DIR="../out_set_var/${MAPPED_ALGO}"
    mkdir -p "$OUTPUT_DIR"
    OUTPUT_FILE="${OUTPUT_DIR}/${BASENAME}_${MAPPED_ALGO}_${SEED}_${START}_${END}.out"
    echo "$OUTPUT_FILE"
    timeout 60 ../build/main --from-file "../DeletefreeSAS/$INSTANCE" --alg "$ALGORITHM" --seed "$SEED" --debug 0 --start "$START" --end "$END" > "$OUTPUT_FILE"

    if [[ $? -eq 124 ]]; then
        echo "Task $INSTANCE $ALGORITHM $SEED $START $END timed out after 60s" >> timeout_log_set_var_78.txt
    fi
done

# back to power saving mode
sudo cpupower frequency-set -g powersave
