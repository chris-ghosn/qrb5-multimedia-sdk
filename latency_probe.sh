#!/bin/bash
# Measures end-to-end pipeline latency via GST_DEBUG timestamps
# Run alongside the pipeline to validate <100ms target

export GST_DEBUG="GST_TRACER:7"
export GST_TRACERS="latency(flags=pipeline+element)"
export GST_DEBUG_FILE="/tmp/latency_trace.log"

python3 test_low_latency_pipeline.py &
PIPE_PID=$!
sleep 32  # capture one full 30s run + buffer

kill $PIPE_PID 2>/dev/null

echo "=== Glass-to-Glass Latency Summary ==="
grep "latency" /tmp/latency_trace.log \
  | awk -F'[=,]' '{print $2}' \
  | awk '{sum+=$1; n++} END {
      printf "Samples : %d
", n;
      printf "Avg     : %.2f ms
", (sum/n)/1e6;
    }'
