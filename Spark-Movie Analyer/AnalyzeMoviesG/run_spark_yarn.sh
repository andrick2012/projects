#!/bin/bash

# Run using Yarn

# Set name of HBase table
INPUT="moviesSample100K"
#INPUT="moviesSample20M"
OUTPUT_DIR="out_yarn"
echo "Using table $INPUT"

# Get the additional class path elements for HBase
#  (we redirect stderr to /dev/null to avoid annoying messages)
HBASE_CLASSPATH="$(hbase mapredcp 2>/dev/null)"

hadoop fs -mkdir -p "hdfs://$PWD"

hadoop fs -rm -f -r "hdfs://$PWD/$OUTPUT_DIR"
rm -rf "./$OUTPUT_DIR"
# Run Spark job

spark-submit \
    --master yarn \
    --deploy-mode cluster \
    --name AnalyzeMoviesG \
    --num-executors 4 \
    --class AnalyzeMoviesG \
    --driver-class-path "$HBASE_CLASSPATH" \
    --conf spark.executor.extraClassPath="$HBASE_CLASSPATH" \
    jar/AnalyzeMoviesG.jar yarn "$INPUT" "hdfs://$PWD/$OUTPUT_DIR" 

# Copy result from HDFS to Linux FS
spark_exit=$?
if [[ $spark_exit -eq 0 ]] ; then
    echo "Copying output from $OUTPUT_DIR"
    hadoop fs -get $PWD/$OUTPUT_DIR .
    exit $?
else
    echo "Spark job failed with status $spark_exit"
    exit $spark_exit
fi
