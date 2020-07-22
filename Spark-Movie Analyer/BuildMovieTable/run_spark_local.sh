#!/bin/bash

# Run locally

# Set name of HBase table (in user's namespace)
#INPUT_DIR="/u/home/mikegoss/PDCPublic/data/MovieLens100K"
INPUT_DIR="/u/home/mikegoss/PDCPublic/data/MovieLens20M"
OUTPUT="${USER}:movies"
echo "Using table $INPUT_DIR"

# Get the additional class path elements for HBase
#  (we redirect stderr to /dev/null to avoid annoying messages).
HBASE_CLASSPATH="$(hbase mapredcp 2>/dev/null)"

# Run the job

spark-submit \
    --master 'local[*]' \
    --deploy-mode client \
    --name BuildMovieTable \
    --class BuildMovieTable \
    --driver-class-path "$HBASE_CLASSPATH" \
    --conf spark.executor.extraClassPath="$HBASE_CLASSPATH" \
    ./jar/BuildMovieTable.jar 'local[*]' "file://$INPUT_DIR" "$OUTPUT"

exit $?
