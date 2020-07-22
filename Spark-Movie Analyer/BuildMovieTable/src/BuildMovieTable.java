/*
@FileName: BuildMovieTable.java
@author: Andrick Adhikari and Nidhi Madabushi
*/

import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.ArrayList;
import java.io.IOException;
import java.lang.Iterable;
import java.util.Iterator;
import java.util.List;
import java.util.function.Function;

import org.apache.spark.SparkConf;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.api.java.JavaRDD;
import org.apache.spark.api.java.JavaPairRDD;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.hbase.HBaseConfiguration;
import org.apache.hadoop.hbase.client.Put;
import org.apache.hadoop.hbase.client.Result;
import org.apache.hadoop.hbase.io.ImmutableBytesWritable;
import org.apache.hadoop.hbase.mapreduce.TableInputFormat;
import org.apache.hadoop.hbase.mapreduce.TableOutputFormat;
import org.apache.hadoop.mapreduce.Job;

import org.apache.spark.api.java.function.FlatMapFunction;
import org.apache.spark.api.java.function.Function2;
import org.apache.spark.api.java.function.PairFlatMapFunction;
import org.apache.spark.api.java.function.PairFunction;

import scala.Tuple2;


public class BuildMovieTable {
  public static void main(String [] args) throws Exception
  {
    if(args.length != 3)
    {
      System.err.println("Usage: BuildMovieTable master inputDir TableName");
      System.exit(1);
    }
    
    //Setting up the spark context
    SparkConf config = new SparkConf().setMaster(args[0]).setAppName("BuildMovieTable");
    JavaSparkContext sc = new JavaSparkContext(config);
    
    //Create hadoop API to write output to table
    Job hOutputJob = null;    
    try {
      hOutputJob = Job.getInstance(HBaseConfiguration.create());
    }
    catch(IOException e)
    {
      System.err.println("ERROR: Job.getInstance exception: " + e.getMessage());
        System.exit(2);
    }
    hOutputJob.getConfiguration().set(TableOutputFormat.OUTPUT_TABLE, args[2]);
    hOutputJob.setOutputFormatClass(TableOutputFormat.class);
    
    
    //Read and parse the input files
    JavaPairRDD<String, Tuple2<Tuple2<String,String>,String>> movieData 
        = sc.textFile(args[1] + "/movie.csv").flatMapToPair(new ParseMovieData());
    
    JavaPairRDD<String, Tuple2<Tuple2<String,String>,String>> ratingData 
        = sc.textFile(args[1] + "/rating/*").mapToPair(new ParseTagRatingData("rating"));
    
    JavaPairRDD<String, Tuple2<Tuple2<String,String>,String>> tagData
        = sc.textFile(args[1] + "/tag.csv").mapToPair(new ParseTagRatingData("tag"));
    
    JavaPairRDD<String, Tuple2<Tuple2<String,String>,String>> combinedData
      = movieData.union(ratingData).union(tagData);
    
    //Combine all the data for a particular Row(movie Id) tuples of (column: values)
    JavaPairRDD<String, ArrayList<Tuple2<Tuple2<String,String>,String>>> mergedData
      = combinedData.combineByKey(
          entry->{
            ArrayList<Tuple2<Tuple2<String,String>,String>> list 
              = new ArrayList<Tuple2<Tuple2<String,String>,String>>();
            list.add(entry);
            return list;
          }, 
          (list, entry) ->{
            list.add(entry);
            return list;
          }, 
          (list1,list2) ->{
            list1.addAll(list2);
            return list1;
          });
    
    //Parse mergedData and add to put object for each movieID
     JavaPairRDD<ImmutableBytesWritable,Put> outputRDD = mergedData.mapToPair(
         row ->
         {
           ImmutableBytesWritable rowKey = new ImmutableBytesWritable(row._1().getBytes());
           Put put = new Put(row._1().getBytes());
           for (Tuple2<Tuple2<String,String>,String> x : row._2())
           {
             Tuple2<String, String> columnInfo = x._1();
             put.addColumn(columnInfo._1().getBytes(),
                 columnInfo._2().getBytes(),
                 x._2().getBytes());
           }
           
           return new Tuple2<ImmutableBytesWritable,Put>(rowKey, put);
         });
     
     outputRDD.saveAsNewAPIHadoopDataset(hOutputJob.getConfiguration());
    
  }
  /*
  * @class: ParseMovieData
  	@Description: for flatMapToPair , for each String in movies.csv
  	creates (movieId:(info:title, movie-title)), (movieId:(info:genre, genreList))
  */
  static class ParseMovieData implements PairFlatMapFunction<String, String,Tuple2<Tuple2<String,String>,String>>{

    @Override
    public Iterator<Tuple2<String, Tuple2<Tuple2<String, String>, String>>> call(String x) throws Exception {
      ArrayList<Tuple2<String, Tuple2<Tuple2<String,String>,String>>> movieDataList =
           new ArrayList<Tuple2<String, Tuple2<Tuple2<String,String>,String>>>();
       
       String data[] = ParseCSV.parseLine(x);

       //Exit if not sufficient fields
       if(data.length != 3)
       {
         System.err.println("not enough movie.csv fields");
         System.exit(2);
       }
       String movieId = data[0];
       String title = data[1];
       String genre = data[2];
       
       //add tuples for title and genre to the key movieId
       movieDataList.add(new Tuple2(movieId, new Tuple2(new Tuple2("info","title"), title)));
       movieDataList.add(new Tuple2(movieId, new Tuple2(new Tuple2("info","genres"), genre)));
       return movieDataList.iterator();
    }   
  }
  /*
  @class: ParseTagRatingData
  @Description:  parse Strings in rating.csv or tag.csv
  Returns either (movieId, (rating:userID, ratingValue|timeStamp)) or (movieId, (tag:userID, tag|timeStamps)) tuples to be used
  */
  static class ParseTagRatingData implements PairFunction<String, String,Tuple2<Tuple2<String,String>,String>>{
    String columnName;
    public ParseTagRatingData(String columnName_) {
      columnName = columnName_;
    }
    @Override
    public Tuple2<String, Tuple2<Tuple2<String, String>, String>> call(String x) throws Exception {
      // TODO Auto-generated method stub
       String data[] = ParseCSV.parseLine(x);
       if(data.length != 4)
       {
         System.err.println("not enough movie.csv fields");
         System.exit(2);
       }
      return new Tuple2(data[1], new Tuple2(new Tuple2(columnName,data[0]), new String(data[2] + "|" + data[3])));
    }
  }
  
}
