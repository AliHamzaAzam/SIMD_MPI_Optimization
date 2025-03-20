#!/bin/bash
# This script times the serial executable "test" for each .tfa file
# and saves the filename and runtime (in seconds) to serial_times.csv

OUTPUT_FILE="serial_times.csv"
echo "filename,time" > "$OUTPUT_FILE"

for file in *.tfa; do
    echo "Running ./test on $file"
    # Use /usr/bin/time to measure the real time (elapsed seconds)
    runtime=$(/usr/bin/time -f "%e" ./test "$file" 2>&1 1>/dev/null)
    echo "$file,$runtime" >> "$OUTPUT_FILE"
done

echo "Results saved in $OUTPUT_FILE"
