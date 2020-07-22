/*
@fileName : AnalyzeMoviesG.java
@author: Andrick Adhikari and Nidhi Madabhushi
*/

import java.io.IOException;
import java.io.Serializable;
import java.util.NavigableMap;
import java.util.ArrayList;
import java.util.Iterator;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.hbase.HBaseConfiguration;
import org.apache.hadoop.hbase.client.Put;
import org.apache.hadoop.hbase.client.Result;
import org.apache.hadoop.hbase.io.ImmutableBytesWritable;
import org.apache.hadoop.hbase.mapreduce.TableInputFormat;
import org.apache.hadoop.hbase.mapreduce.TableOutputFormat;
import org.apache.hadoop.mapreduce.Job;

import org.apache.spark.SparkConf;
import org.apache.spark.api.java.JavaPairRDD;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.api.java.function.PairFlatMapFunction;
import org.apache.spark.storage.StorageLevel;

import scala.Tuple2;


public class AnalyzeMoviesG {
	public static void main(String [] args) {
		//check arguments
		if(args.length != 3)
		{
			System.err.println("usage: AnaylizeMoviesG master inputTable outputDir");
			System.exit(1);    
		}
		
	    // Create a Java Spark Context
	    SparkConf conf = new SparkConf().setMaster(args[0]).setAppName("AnalyzeMoviesG");
	    JavaSparkContext sc = new JavaSparkContext(conf);

	    // Create the HBase configuration
	    Configuration hConf = HBaseConfiguration.create();
	    hConf.set(TableInputFormat.INPUT_TABLE, args[1]);
	    
	    // Get an RDD from the HBase table
	    JavaPairRDD<ImmutableBytesWritable,Result> tableRDD =
	      sc.newAPIHadoopRDD(hConf, 
	                         TableInputFormat.class,
	                         ImmutableBytesWritable.class,
	                         Result.class);
	    
	   
	    //parse Data for <genre|year> : <double: rating, Int: tagCount>
	    JavaPairRDD<String, Tuple2<Double,Integer>> parsedRDD = tableRDD.flatMapToPair(new ParseTable());
		
	    //Calculate the average rating and sum up the tags
	    JavaPairRDD<String, CombineValues> finalRDD = parsedRDD.combineByKey(
	    		entry ->{
	    			CombineValues merger = new CombineValues();
	    			merger.addEntry(entry);
	    			return merger;
	    		}, 
	    		(merger, entry) ->{
	    			merger.addEntry(entry);
	    			return merger;
	    		}, 
	    		(mergerA, mergerB) ->{
	    			mergerA.mergeCombiners(mergerB);
	    			return mergerA;
	    		});
	    
	    //Saved as text File
	    finalRDD.saveAsTextFile(args[2]);

	}
	/*
		@class: CombinedValues
		@description: for a particular genre|year key sums up the ratings
					Also, sums the tag counts.
	*/
	static class CombineValues implements Serializable{
		
		double ratingTotal; //sum of ratings
		long count; // number of ratings
		long tagCount; // count of tags

		//Constructor : Initializes all the fields to 0
		public CombineValues()
		{
			ratingTotal = 0.0;
			count = 0;
			tagCount = 0;			
		}
		public void addEntry(Tuple2<Double,Integer> entry)
		{
			// Add ratings , increases count (for rating > 0)
			if(entry._1() > 0.0)
			{
				ratingTotal += entry._1();
				count++;
			}

			//Increments the tagCount for valid tags
			if(entry._2() > 0)
			{
				tagCount += entry._2();
			}
		}
		// merge fields of another object of class CombineValues
		public void mergeCombiners(CombineValues obj)
		{
			ratingTotal += obj.ratingTotal;
			count += obj.count;
			tagCount += obj.tagCount;
		}

		//Returns average rating and total tag count as String
		public String toString()
		{
			String returnString = "";
		    returnString += Double.toString(ratingTotal/count) + " ";
		    returnString += Long.toString(tagCount);
		    return returnString;
		}
	}
	/*
		@class ParseTable
		@Description: Implements the pairFlatMapFunction.
						parse each row of the Table into a tuple of (key:'genre"year', value: tuple2(rating, tagCount))

	*/
	static class ParseTable implements PairFlatMapFunction<Tuple2<ImmutableBytesWritable,Result>, String, Tuple2<Double, Integer>>
	{
		 String infoFamily = "info";
		 String genreKey = "genres";
		 byte[] ratingFamily = "rating".getBytes();
		 byte[] tagFamily = "tag".getBytes();

		@Override
		public Iterator<Tuple2<String, Tuple2<Double, Integer>>> call(Tuple2<ImmutableBytesWritable, Result> x)
				throws Exception {
		 ArrayList<Tuple2<String, Tuple2<Double,Integer>>> genreList 
	 		= new ArrayList<Tuple2<String, Tuple2<Double, Integer>>>();
		 //For each row
	 	 Result r = x._2();
	 	 NavigableMap<byte[],byte[]> infoMap = r.getFamilyMap(infoFamily.getBytes());
	 	 if(infoMap.containsKey(genreKey.getBytes())){

	 	 	 //Get the list of genres in a row
		     String [] genres = new String(infoMap.get(genreKey.getBytes())).split("\\|");

	 		 NavigableMap<byte[], byte[]> ratingMap = r.getFamilyMap(ratingFamily);
	 		 //For each rating|timestamp in rating:userId column
	 		 //Parse the year and rating and add for each genre in genre list
	 		 for(byte[] ratingValues: ratingMap.values())
	 		 {
	 			 String [] ratingValue =new String(ratingValues).split("\\|");
				 if(ratingValue.length != 2)
				     continue;
	 			 String year = ratingValue[1].split(" ")[0].split("-")[0];
	 			 double rating = Double.parseDouble(ratingValue[0]);

				 for(String genre: genres)
	 			 genreList.add(new Tuple2(genre+"|"+year,new Tuple2(rating,-1)));
	 			 
	 		 }
	 		 //For each tag|timestamp value in tag:userId column add 1 to each genre
	 		 NavigableMap<byte[], byte[]> tagMap = r.getFamilyMap(tagFamily);
	 		 for (byte[] tagValues: tagMap.values())
	 		 {
	 			 String [] tagValue =new String(tagValues).split("\\|");
	 			 String year = tagValue[tagValue.length - 1].split(" ")[0].split("-")[0];	 			 
				 for(String genre: genres)
	 			 genreList.add(new Tuple2(genre+"|"+year, new Tuple2(-1.1, 1)));
	 		 }
	 		 
	 	 }
		 return genreList.iterator();
		}
		
	}
}
