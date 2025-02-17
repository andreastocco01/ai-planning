#!/bin/bash
#SBATCH --job-name=planning
#SBATCH --partition=arrow
#SBATCH --array=1-1000 # 1-1000 for full testbed
#SBATCH --time=02:10:00 # time limit per job!
#SBATCH --mem=14GB
#SBATCH --error=logs/%A_%a.err
#SBATCH --output=logs/%A_%a.out

# warm up processors
sudo cpupower frequency-set -g performance

sleep 0.1

stress-ng -c 4 --cpu-ops=100

# set limits
ulimit -v 16777216

# COMMANDS TO EXECUTE

# Number of tasks per job
TASKS_PER_JOB=125 # 125 for full testbed

# Compute the range of lines for this job
START_LINE=$(( (SLURM_ARRAY_TASK_ID - 1) * TASKS_PER_JOB + 1 ))
END_LINE=$(( START_LINE + TASKS_PER_JOB - 1 ))

# Read and execute the assigned tasks
sed -n "${START_LINE},${END_LINE}p" tasks.txt | while read INSTANCE ALGORITHM SEED; do
    BASENAME="${INSTANCE::-4}"

    case "$ALGORITHM" in
        0) MAPPED_ALGO="random" ;;
        1) MAPPED_ALGO="greedy" ;;
        2) MAPPED_ALGO="hadd" ;;
        3) MAPPED_ALGO="hmax" ;;
        *) echo "Unknown algorithm: $ALGORITHM"; exit 1 ;;
    esac

    OUTPUT_DIR="../out/${MAPPED_ALGO}"
    mkdir -p "$OUTPUT_DIR"
    OUTPUT_FILE="${OUTPUT_DIR}/${BASENAME}_${MAPPED_ALGO}_${SEED}.out"
    timeout 60 ../build/main --from-file "../DeletefreeSAS/$INSTANCE" --alg "$ALGORITHM" --seed "$SEED" --debug 0 > "$OUTPUT_FILE"

    if [[ $? -eq 124 ]]; then
        echo "Task $INSTANCE $ALGORITHM $SEED timed out after 60s" >> timeout_log.txt
    fi
done

# back to power saving mode
sudo cpupower frequency-set -g powersave
