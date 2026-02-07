#!/bin/bash

ROLL="MT25090"
PORT=9090
DURATION=5

MSG_SIZES=(1024 4096 16384 65536)
THREADS=(1 2 4 8)
IMPLEMENTATIONS=("A1" "A2" "A3")

CSV_FILE="${ROLL}_Part_C_Expresults.csv"

echo "Impl,MessageSize,Threads,Throughput_Gbps,Latency_us,Cycles,L1_DCache_Misses,LLC_Misses,ContextSwitches" > $CSV_FILE

make clean
make

for impl in "${IMPLEMENTATIONS[@]}"
do
    SERVER="./${ROLL}_Part_${impl}_Server"

    for size in "${MSG_SIZES[@]}"
    do
        for th in "${THREADS[@]}"
        do
            echo "Running Impl=$impl Size=$size Threads=$th"

            # Start server normally
            $SERVER $PORT $size &
            SERVER_PID=$!
            sleep 2

            # Run client under perf
            PERF_OUTPUT=$(perf stat -e cycles,L1-dcache-load-misses,LLC-load-misses,context-switches \
            ./${ROLL}_Part_A_Client 127.0.0.1 $PORT $size $th $DURATION 2>&1)

            # Kill server safely
            kill $SERVER_PID 2>/dev/null
            wait $SERVER_PID 2>/dev/null
            sleep 1

            # Save perf output for debugging
            echo "$PERF_OUTPUT" > perf_output.txt

            # ----------------- ROBUST PERF PARSING -----------------

            CYCLES=$(echo "$PERF_OUTPUT" | grep " cycles" | grep -Eo '[0-9,]+' | head -1 | tr -d ',')

            L1MISS=$(echo "$PERF_OUTPUT" | grep "L1-dcache-load-misses" | grep -Eo '[0-9,]+' | head -1 | tr -d ',')

            LLCMISS=$(echo "$PERF_OUTPUT" | grep "LLC-load-misses" | grep -Eo '[0-9,]+' | head -1 | tr -d ',')

            CSW=$(echo "$PERF_OUTPUT" | grep "context-switches" | grep -Eo '[0-9,]+' | head -1 | tr -d ',')

            # ----------------- CLIENT PARSING -----------------

            TOTAL_BYTES=$(echo "$PERF_OUTPUT" | grep TOTAL_BYTES | cut -d '=' -f2)
            TOTAL_REQ=$(echo "$PERF_OUTPUT" | grep TOTAL_REQUESTS | cut -d '=' -f2)
            TOTAL_TIME=$(echo "$PERF_OUTPUT" | grep TOTAL_TIME | cut -d '=' -f2)

            # Safety guards
            if [ -z "$TOTAL_BYTES" ]; then TOTAL_BYTES=0; fi
            if [ -z "$TOTAL_TIME" ] || [ "$TOTAL_TIME" = "0" ]; then TOTAL_TIME=1; fi
            if [ -z "$TOTAL_REQ" ] || [ "$TOTAL_REQ" = "0" ]; then TOTAL_REQ=1; fi

            # Compute throughput (Gbps)
            THROUGHPUT=$(echo "scale=6; ($TOTAL_BYTES * 8) / ($TOTAL_TIME * 1000000000)" | bc)

            # Compute latency (microseconds)
            LATENCY=$(echo "scale=6; ($TOTAL_TIME * 1000000) / $TOTAL_REQ" | bc)

            # Write to CSV
            echo "$impl,$size,$th,$THROUGHPUT,$LATENCY,$CYCLES,$L1MISS,$LLCMISS,$CSW" >> $CSV_FILE

        done
    done
done

echo "All experiments completed."
