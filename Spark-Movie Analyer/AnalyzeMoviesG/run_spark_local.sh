#!/bin/bash

# Run locally

# Set name of HBase table (in user's namespace)
#INPUT="moviesSample20M"
INPUT="moviesSample100K"
OUTPUT_DIR="out_local"
echo "Using table $INPUT"

# Get the additional class path elements for HBase
#  (we redirect stderr to /dev/null to avoid annoying messages).
HBASE_CLASSPATH="$(hbase mapredcp 2>/dev/null)"

rm -rf "$OUTPUT_DIR"
# Run the job

spark-submit \
    --master 'local[*]' \
    --deploy-mode client \
    --name AnalyzeMoviesG \
    --class AnalyzeMoviesG \
    --driver-class-path "$HBASE_CLASSPATH" \
    --conf spark.executor.extraClassPath="$HBASE_CLASSPATH" \
    ./jar/AnalyzeMoviesG.jar 'local[*]' "$INPUT" "file://$PWD/$OUTPUT_DIR"

exit $?
