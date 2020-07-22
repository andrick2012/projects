#!/bin/bash

# Run using Yarn

# Set name of HBase table
#INPUT_DIR="hdfs:///Public/data/MovieLens100K"
INPUT_DIR="hdfs:///Public/data/MovieLens20M"
OUTPUT="${USER}:movies"
echo "Using table $INPUT_DIR"

# Get the additional class path elements for HBase
#  (we redirect stderr to /dev/null to avoid annoying messages)
HBASE_CLASSPATH="$(hbase mapredcp 2>/dev/null)"

# Run Spark job

spark-submit \
    --master yarn \
    --deploy-mode cluster \
    --name BuildMovieTable \
    --num-executors 2 \
    --class BuildMovieTable \
    --driver-class-path "$HBASE_CLASSPATH" \
    --conf spark.executor.extraClassPath="$HBASE_CLASSPATH" \
    jar/BuildMovieTable.jar yarn "$INPUT_DIR" "$OUTPUT"

exit $?
