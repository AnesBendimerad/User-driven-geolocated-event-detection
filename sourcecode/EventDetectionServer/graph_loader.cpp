//#include "stdafx.h"
#include "graph_loader.h"
#include "json.hpp"
#include <fstream>
#include "string_management.h"
using namespace std;
using json = nlohmann::json;

Graph * GraphLoader::load()
{
    std::ifstream inputFile(designPoint->graphFilePath);
    if (!inputFile.is_open()){
        std::cout<<"cannot open file"<<designPoint->graphFilePath<<std::endl;
        throw "cannot open file";
    }
    std::string line;
    std::string fileAsString;
    while (std::getline(inputFile, line))
    {
        fileAsString += line;
    }
    inputFile.close();
    json fileAsJson = json::parse(fileAsString.c_str());
    int numberOfVertices = fileAsJson["numberOfVertices"].get<int>();
    int numberOfTimes = fileAsJson["numberOfTimes"].get<int>();
    int cycle = fileAsJson["cycle"].get<int>();
    int numberOfUsedTweets=0;
    int totalHashtagOccurrences=fileAsJson["totalOccurrenceOfHashtags"].get<int>();
    Graph * graph = new Graph(cycle,numberOfUsedTweets,totalHashtagOccurrences);
    unordered_map<string, int> hashtagIds;
    map<string, int> vertexIndicesFromIds;
    int currentNumberOfHashtags = 0;
    
    if (designPoint->endMineTime>=0 && designPoint->endMineTime<= numberOfTimes){
        numberOfTimes=designPoint->endMineTime;
    }
    else {
        designPoint->endMineTime=numberOfTimes;
    }
    
    // vertices construction
    for (int vertexIndex = 0; vertexIndex < numberOfVertices; vertexIndex++) {
        Vertex vertex;
        vertex.identifier =  fileAsJson["vertices"][vertexIndex]["vertexId"].get<string>();
        vertexIndicesFromIds[vertex.identifier] = vertexIndex;
        for (int timeIndex= 0; timeIndex < numberOfTimes; timeIndex++) {
            TimeInterval interval;
            json intervalAsJson = fileAsJson["vertices"][vertexIndex]["hashtags"][timeIndex];
            for (int cpt = 0; cpt < intervalAsJson.size(); cpt++) {
                json values=intervalAsJson[cpt].begin().value();
                //int val1=values[0].get<int>();
                int val1=values.get<int>();
                //double val2=values[1].get<double>();
                int hashtagId = getHashtagId(intervalAsJson[cpt].begin().key(), hashtagIds, currentNumberOfHashtags, graph->getHashtags());
                
                interval.hashtagsOccurrences[hashtagId] =val1;
                vertex.hashtagsTotalOccurrences[hashtagId]+=val1;
            }
            vertex.periodicHashOcc.push_back(interval);
        }
        for (int timeIndex= 0; timeIndex < numberOfTimes; timeIndex++) {
            json intervalAsJson = fileAsJson["vertices"][vertexIndex]["tweetsId"][timeIndex];
            for (int cpt = 0; cpt < intervalAsJson.size(); cpt++) {
                vertex.periodicHashOcc[timeIndex].tweetIds.push_back(intervalAsJson[cpt].get<string>());
                numberOfUsedTweets++;
            }
        }
        
        graph->getVertices().push_back(vertex);
        
        
        
    }
    
    // read edges
    for (int edgeIndex = 0; edgeIndex <fileAsJson["edges"].size(); edgeIndex++) {
        json edges = fileAsJson["edges"][edgeIndex];
        int vertexIndex = vertexIndicesFromIds[edges["vertexId"].get<string>()];
        for (int j = 0; j < edges["connectedVertices"].size(); j++) {
            graph->getVertices()[vertexIndex].neighbors.insert(vertexIndicesFromIds[edges["connectedVertices"][j].get<string>()]);
        }
    }
    graph->setHashtagsMap(hashtagIds);
    
    unordered_map<string, int> verticesIds;
    for (int i=0;i<graph->getVertices().size();i++){
        verticesIds[graph->getVertices()[i].identifier]=i;
    }
    graph->setVerticesIds(verticesIds);
    graph->setNumberOfUsedTweets(numberOfUsedTweets);
    
    {
    //if (preferenceBased){
        std::ifstream hashtagCooccurrencesFile(designPoint->hashtagCooccurrencesFilePath);
        if (!hashtagCooccurrencesFile.is_open()){
            std::cout<<"cannot open file"<<designPoint->hashtagCooccurrencesFilePath<<std::endl;
            throw "cannot open file";
        }
        std::string line;
        std::vector<std::unordered_map<int,std::unordered_map<int,int>>> hashtagCooccurrencesPerTime;
        std::vector<std::unordered_map<int,std::vector<CoOccPerTime> > > hashCoOccInverted;
        for (int i=0;i<currentNumberOfHashtags;i++){
            std::unordered_map<int,std::vector<CoOccPerTime> > curMap;
            hashCoOccInverted.push_back(curMap);
        }
        int timeIndex=-1;
        while (std::getline(hashtagCooccurrencesFile, line))
        {
            if (line.at(line.length() - 1) == '\r' || line.at(line.length() - 1) == '\n') {
                line = line.substr(0, line.length() - 1);
            }
            
            vector<string> elements1=split(line, ',');
            if (elements1[0].compare("NewTimeInterval-nbTweets")==0){
                timeIndex++;
                unordered_map<int,std::unordered_map<int,int>> currentHashtagCooc;
                hashtagCooccurrencesPerTime.push_back(currentHashtagCooc);
            }
            else {
                int hash1=hashtagIds[elements1[0]];
                int hash2=hashtagIds[elements1[1]];
                int occNumber=atoi(elements1[2].c_str());
                if (hashtagCooccurrencesPerTime[timeIndex].find(hash1)==hashtagCooccurrencesPerTime[timeIndex].end()){
                    unordered_map<int,int> occ;
                    hashtagCooccurrencesPerTime[timeIndex][hash1]=occ;
                }
                if (hashtagCooccurrencesPerTime[timeIndex].find(hash2)==hashtagCooccurrencesPerTime[timeIndex].end()){
                    unordered_map<int,int> occ;
                    hashtagCooccurrencesPerTime[timeIndex][hash2]=occ;
                }
                unordered_map<int,int> & occ1=hashtagCooccurrencesPerTime[timeIndex][hash1];
                unordered_map<int,int> & occ2=hashtagCooccurrencesPerTime[timeIndex][hash2];
                if (occ1.find(hash2)==occ1.end()){
                    occ1[hash2]=occNumber;
                }
                else {
                    occ1[hash2]=occ1[hash2]+occNumber;
                }
                if (occ2.find(hash1)==occ2.end()){
                    occ2[hash1]=occNumber;
                }
                else {
                    occ2[hash1]=occ2[hash1]+occNumber;
                }
                CoOccPerTime coOcc;
                coOcc.timeIndex=timeIndex;
                coOcc.coOcc=occNumber;
                
                if (hashCoOccInverted[hash1].find(hash2)==hashCoOccInverted[hash1].end()){
                    std::vector<CoOccPerTime> myVec;
                    hashCoOccInverted[hash1][hash2]=myVec;
                }
                //if (! std::binary_search (hashCoOccInverted[hash1][hash2].begin(), hashCoOccInverted[hash1][hash2].end(), timeIndex)){
                hashCoOccInverted[hash1][hash2].push_back(coOcc);
                //}
                if (hashCoOccInverted[hash2].find(hash1)==hashCoOccInverted[hash1].end()){
                    std::vector<CoOccPerTime> myVec;
                    hashCoOccInverted[hash2][hash1]=myVec;
                }
                //if (! std::binary_search (hashCoOccInverted[hash2][hash1].begin(), hashCoOccInverted[hash2][hash1].end(), timeIndex)){
                    hashCoOccInverted[hash2][hash1].push_back(coOcc);
                //}
            }
        }
        //hna
        hashtagCooccurrencesFile.close();
        HashSubjQualitiesManager * hashcoocc=new HashSubjQualitiesManager(hashtagCooccurrencesPerTime,currentNumberOfHashtags,hashCoOccInverted);
        graph->setHashtagCooccurrencesGraph(hashcoocc);
        //graph->setNumberOfUsedTweets(numberOfUsedTweets);
        graph->setVerticesSubQualitiesManager(new VerticesSubQualitiesManager(graph));
        
    }
    //else {
    //    graph->inactivatePreferences();
    //}
    if (designPoint->tweetsFilePath.compare("")!=0){
        // opening file
        graph->areTweetsLoaded=true;
        std::ifstream tweetsFile(designPoint->tweetsFilePath);
        std::string line;
        std::map<std::string,Tweet> tweets;
        std::getline(tweetsFile, line);
        while (std::getline(tweetsFile, line))
        {
            if (line.at(line.length() - 1) == '\r' || line.at(line.length() - 1) == '\n') {
                line = line.substr(0, line.length() - 1);
            }
            Tweet tweet;
            std::vector<string> elements = split(line, '|');
            // 0 : id, 8 : hashtags, 9: usermentions
            std::string idTweet=elements[0];
            std::vector<string> hashtags=split(elements[8],',');
            std::vector<string> usermentions=split(elements[9],',');
            
            for (std::string & hashtag : hashtags){
                if (graph->getHashtagsMap().find("#"+hashtag)!=graph->getHashtagsMap().end()){
                    tweet.terms.insert(graph->getHashtagsMap()["#"+hashtag]);
                }
            }
            for (std::string & usermention : usermentions){
                if (graph->getHashtagsMap().find("@"+usermention)!=graph->getHashtagsMap().end()){
                    tweet.terms.insert(graph->getHashtagsMap()["@"+usermention]);
                }
            }
            tweet.identifier=idTweet;
            tweet.userId=elements[4];
            if (designPoint->writeTweetsInDetails){
                tweet.timeStamp=stol(elements[1]);
                tweet.latitude=stod(elements[2]);
                tweet.longitude=stod(elements[3]);
                tweet.text=elements[11];
                std::vector<string> mediaUrls=split(elements[10],',');
                for (std::string & mediaUrl : mediaUrls){
                    if (mediaUrl.compare("")!=0){
                        tweet.mediaUrls.push_back(mediaUrl);
                    }
                }
                std::replace(tweet.text.begin(),tweet.text.end(),'"','\'');
            }
            
            tweets[idTweet]=tweet;
        }
        graph->setTweets(tweets);
    }
    else {
        graph->areTweetsLoaded=false;
    }
    
    
    return graph;
}


int GraphLoader::getHashtagId(string hashtag, unordered_map<string, int>& hashtagIds, int & currentNumberOfHashtags, vector<string> & hashtagIdsVector)
{
    if (hashtagIds.find(hashtag) == hashtagIds.end()) {
		hashtagIds[hashtag] = currentNumberOfHashtags;
		hashtagIdsVector.push_back(hashtag);
		currentNumberOfHashtags++;
		return (currentNumberOfHashtags - 1);
	}
	else {
		int hashtagId = hashtagIds[hashtag];
		return hashtagId;
	}
}
