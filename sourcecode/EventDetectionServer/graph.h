#pragma once
#include <map>
#include <set>
#include <vector>
#include <string>
#include "HashtagSubQualitiesManager.h"
#include "VerticesSubQualitiesManager.h"
#include <unordered_map>

typedef struct Statistics
{
    int nbFoundPatterns;
    int nbExploredPatterns;
    double graphLoadingTime;
    double initMeasureCalculationTime;
    double patternMiningTime;
    double clusteringTime;
    double preparClustTime;
    double jarTime;
    double clusWritingTime;
    double removeRepForSamp;
    
} Statistics;



typedef struct TimeInterval
{
    std::map<int, int> hashtagsOccurrences;
	std::map<int, double> hashScores;
    //std::map<int,double> hashEntropies;
	std::set<int> positiveHashs;
    std::vector<std::string> tweetIds;
    
} TimeInterval;

typedef struct Vertex
{
	std::string identifier;
	std::set<int> neighbors;
	std::vector<TimeInterval> periodicHashOcc;
	std::vector<std::map<int, double> > avgOccurrences;
    std::map<int,int> hashtagsTotalOccurrences;
    std::map<int,double> hashtagsStandardDev;
} Vertex;

typedef struct Tweet{
    std::string identifier;
    double latitude;
    double longitude;
    long timeStamp;
    std::string text;
    std::set<int> terms;
    std::vector<std::string> mediaUrls;
    std::string userId;
} Tweet;

class Graph {
public:
	Graph(int cycle, int numberOfUsedTweets,int totalHashtagOccurrences):cycle(cycle), numberOfUsedTweets(numberOfUsedTweets), totalHashtagOccurrences(totalHashtagOccurrences){};
    ~ Graph();
    std::vector<std::string> & getHashtags() { return hashtags; }
    void setHashtags(std::vector<std::string>  & hashtags1){hashtags=hashtags1;}
	std::vector<Vertex> & getVertices() { return vertices; }
	std::vector<int> & getHashtagsRepetition() {return hashtagsRepetition;}
	int getCycle() { return cycle;}
    int getNumberOfUsedTweets(){return numberOfUsedTweets;}
    void setNumberOfUsedTweets(int numberOfTweets){numberOfUsedTweets=numberOfTweets;}
    
    int getTotalHashtagOccurrences() {return totalHashtagOccurrences;}
    //void setHashtagTotalOccurrences(std::vector<int> & pHhashtagTotalOccurrences){hashtagTotalOccurrences=pHhashtagTotalOccurrences;}
    //void setTimeTotalOccurrences(int * pTimeTotalOccurrences){timeTotalOccurrences=pTimeTotalOccurrences;}
    //std::vector<int> & getHashtagTotalOccurrences(){return hashtagTotalOccurrences;}
    //int * getTimeTotalOccurrences(){return timeTotalOccurrences;}
    
    void setHashtagCooccurrencesGraph(HashSubjQualitiesManager * hashtagCooccurrencesGraph1){hashtagCooccurrencesGraph=hashtagCooccurrencesGraph1;}
    HashSubjQualitiesManager * getHashtagCooccurrences(){return hashtagCooccurrencesGraph;}
    
    void setVerticesSubQualitiesManager(VerticesSubQualitiesManager * verticesSubQualitiesManager1){verticesSubQualitiesManager=verticesSubQualitiesManager1;}
    VerticesSubQualitiesManager * getVerticesSubQualitiesManager(){return verticesSubQualitiesManager;}
    
    
    void setHashtagsMap(std::unordered_map<std::string,int> & hashtagsMap1){hashtagsMap=hashtagsMap1;}
    std::unordered_map<std::string,int> & getHashtagsMap() {return hashtagsMap;}
    void setVerticesIds(std::unordered_map<std::string,int> & verticesIds1){verticesIds=verticesIds1;}
    std::unordered_map<std::string,int> & getVerticesIds() {return verticesIds;}
    void inactivatePreferences();
    void setTweets(std::map<std::string,Tweet> &tweets1){tweets=tweets1;}
    std::map<std::string,Tweet> & getTweets(){return tweets;}
    Graph * cloneGraph();
    
    bool areTweetsLoaded;
    std::unordered_map<std::string,int> verticesIds;
    std::unordered_map<std::string,int> hashtagsMap;
    std::vector<std::string> hashtags;
    HashSubjQualitiesManager * hashtagCooccurrencesGraph;
private:
	
	std::vector<Vertex> vertices;
    std::vector<int> hashtagsRepetition;
	int cycle;
    int numberOfUsedTweets;
    int totalHashtagOccurrences;
    //std::vector<int> hashtagTotalOccurrences;
    //int * timeTotalOccurrences;
    
    
    VerticesSubQualitiesManager * verticesSubQualitiesManager;
    
    
    std::map<std::string,Tweet> tweets;
    
};
