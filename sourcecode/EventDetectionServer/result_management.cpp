//#include "stdafx.h"
#include "result_management.h"
#include <fstream>
#include <string>
#include <algorithm>
#include "json.hpp"
#include "PrefBasedSorter.h"
#include "utils.hpp"
using namespace std;
string fileSeparator;
using json = nlohmann::json;
int nbMinTweets=5;
int nbMinUsers=2;

void loadPreviousResult(string inputFilePath,vector<Pattern> & previousPatterns,unordered_map<string, int> hashtagIds,unordered_map<string, int> verticesIds){
    std::ifstream previousPatternsFile(inputFilePath);
    if (!previousPatternsFile.is_open()){
        std::cout<<"cannot open file"<<inputFilePath<<std::endl;
        throw "cannot open file";
    }
    std::string line;
    std::string fileAsString;
    while (std::getline(previousPatternsFile, line))
    {
        fileAsString += line;
    }
    previousPatternsFile.close();
    
    json fileAsJson = json::parse(fileAsString.c_str());
    if (showCouts){
        std::cout<<"file loaded and parsed"<<std::endl;
    }
    
    int numberOfFoundPatterns = fileAsJson["numberOfPatterns"].get<int>();
    previousPatterns.clear();
    for (int i=0;i<numberOfFoundPatterns;i++){
        //if (showCouts){
        //    std::cout<<"pattern:"<<std::to_string(i)<<std::endl;
        //}
        json patternAsJson=fileAsJson["patterns"][i];
        Pattern p;
        p.beginTime=patternAsJson["beginTime"].get<int>();
        p.endTime=patternAsJson["endTime"].get<int>();
        json verticesAsJson=patternAsJson["vertexIds"];
        for (int cpt = 0; cpt < verticesAsJson.size(); cpt++) {
            string vertexId=verticesAsJson[cpt].get<string>();
            p.vertexIndices.insert(verticesIds[vertexId]);
        }
        json hashtagsAsJson=patternAsJson["hashtags"];
        for (int cpt = 0; cpt < hashtagsAsJson.size(); cpt++) {
            if (hashtagIds.find(hashtagsAsJson[cpt].begin().key())!=hashtagIds.end()){
                p.hashtags[hashtagIds[hashtagsAsJson[cpt].begin().key()]]=hashtagsAsJson[cpt].begin().value().get<double>();
            }
        }
        previousPatterns.push_back(p);
    }
    
}

void writePostProcessedResult(Graph *graph,std::vector<Pattern> & finalPatterns,std::string outputFilePath,bool orderResult,Statistics & statistics,DesignPoint &designPoint){
#ifdef _WIN32
    //define something for Windows
    fileSeparator = "\\";
    
#else
    //define it for a Unix machine
    fileSeparator = "/";
#endif
    if (outputFilePath.find_last_of(fileSeparator)<outputFilePath.size()){
        outputFilePath=outputFilePath.substr(0,outputFilePath.find_last_of(fileSeparator)+1)+"postProcessedPatterns.json";
    }
    else {
        outputFilePath="postProcessedPatterns.json";
    }
    writeResult(graph, finalPatterns, outputFilePath, orderResult, statistics, designPoint);
}


void writeResult(Graph * graph, vector<Pattern> & retrievedPatterns, string outputFilePath,bool orderResult,Statistics & statistics,DesignPoint &designPoint)
{
#ifdef _WIN32
    //define something for Windows
    fileSeparator = "\\";
    
#else
    //define it for a Unix machine
    fileSeparator = "/";
#endif
    if (showCouts){
    std::cout<<"writing results"<<std::endl;
    std::cout<<"writing statistics"<<std::endl;
    }
    string statisticsFilePath;
    if (outputFilePath.find_last_of(fileSeparator)<outputFilePath.size()){
        statisticsFilePath=outputFilePath.substr(0,outputFilePath.find_last_of(fileSeparator)+1)+"Statistics.txt";
    }
    else {
        statisticsFilePath="Statistics.txt";
    }
    std::ofstream statFile;
    statFile.open(statisticsFilePath);
    statFile<<"nbFoundPatterns:"<<to_string(statistics.nbFoundPatterns)<<endl;
    statFile<<"nbExploredPatterns:"<<to_string(statistics.nbExploredPatterns)<<endl;
    statFile<<"graphLoadingTime:"<<to_string(statistics.graphLoadingTime)<<endl;
    statFile<<"initMeasureCalculationTime:"<<to_string(statistics.initMeasureCalculationTime)<<endl;
    statFile<<"patternMiningTime:"<<to_string(statistics.patternMiningTime)<<endl;
    statFile<<"clusteringTime:"<<to_string(statistics.clusteringTime)<<endl;
    statFile<<"preparClustTime:"<<to_string(statistics.preparClustTime)<<endl;
    statFile<<"jarTime:"<<to_string(statistics.jarTime)<<endl;
    statFile<<"clusWritingTime:"<<to_string(statistics.clusWritingTime)<<endl;
    statFile<<"removeRepForSamp:"<<to_string(statistics.removeRepForSamp)<<endl;
    statFile.close();
    if (showCouts){
    std::cout<<"writing statistics finished"<<std::endl;
    }
    for (Pattern & pattern :retrievedPatterns){
        pattern.totalScore=0;
        for (map<int,double>::iterator it=pattern.hashtags.begin();it!=pattern.hashtags.end();it++){
            pattern.totalScore+=it->second;
        }
    }
    if (orderResult){
        
        // write result not ordered by prefBased
        sort(retrievedPatterns.begin(),retrievedPatterns.end(),comparePatterns);
        string resultNotOrderedPath=outputFilePath.substr(0,outputFilePath.find_last_of("."))+"QualitySorted"+outputFilePath.substr(outputFilePath.find_last_of("."),outputFilePath.size());
        writeResult(graph, retrievedPatterns, resultNotOrderedPath, false,statistics,designPoint);
        // sort result
        if (designPoint.prefBased==true){
            PrefBasedSorter sorter(graph);
            sorter.sortResults(retrievedPatterns);
        }
        else {
            return;
        }
    }
    
    vector<vector<string>> tweetsIdsOfPatterns;
    
    for (Pattern & pattern : retrievedPatterns){
        vector<string> tweetIds;
        for (int vertexId : pattern.vertexIndices){
            for (int timeId=pattern.beginTime;timeId<=pattern.endTime;timeId++){
                if (!graph->areTweetsLoaded){
                    tweetIds.insert(tweetIds.end(), graph->getVertices()[vertexId].periodicHashOcc[timeId].tweetIds.begin(),
                                graph->getVertices()[vertexId].periodicHashOcc[timeId].tweetIds.end());
                }
                else {
                    std::vector<std::string> &cellTweetsIds=graph->getVertices()[vertexId].periodicHashOcc[timeId].tweetIds;
                    for (std::string tweetId : cellTweetsIds){
                        std::set<int> &tweetTerms=graph->getTweets()[tweetId].terms;
                        for (int term : tweetTerms){
                            if (pattern.hashtags.find(term)!=pattern.hashtags.end()){
                                tweetIds.push_back(tweetId);
                                break;
                            }
                        }
                    }
                }
            }
        }
        tweetsIdsOfPatterns.push_back(tweetIds);
    }
    if (designPoint.filterBySizes){
        int cpt=0;
        while (cpt<retrievedPatterns.size()){
            if (tweetsIdsOfPatterns[cpt].size()<nbMinTweets){
                // remove
                tweetsIdsOfPatterns.erase(tweetsIdsOfPatterns.begin()+cpt);
                retrievedPatterns.erase(retrievedPatterns.begin()+cpt);
            }
            else {
                // counting nb users
                std::set<std::string> users;
                for (std::string tweetId : tweetsIdsOfPatterns[cpt]){
                    Tweet & tweet=graph->getTweets()[tweetId];
                    users.insert(tweet.userId);
                }
                if (users.size()<nbMinUsers){
                    tweetsIdsOfPatterns.erase(tweetsIdsOfPatterns.begin()+cpt);
                    retrievedPatterns.erase(retrievedPatterns.begin()+cpt);
                }
                else {
                    cpt++;
                }
            }
        }
    }

	std::ofstream resultFile;
	resultFile.open(outputFilePath);
	resultFile << "{" << endl;
	resultFile << "\t\"numberOfPatterns\":" << to_string(retrievedPatterns.size()) << "," << endl;
    //resultFile << "\t\"numberOfPatterns\":" << to_string(30) << "," << endl;
	resultFile << "\t\"patterns\":[" << endl;
	bool first = true;
    int cpt=0;
    //int cc=0;
	for (Pattern & pattern : retrievedPatterns) {
        //if (cc>=30){
        //    break;
        //}
        //cc++;
        
		if (first) {
			first = false;
		}
		else {
			resultFile << "," << endl;
		}
		resultFile << "\t\t{" << endl;
		resultFile << "\t\t\t\"beginTime\":" << to_string(pattern.beginTime) << "," << endl;
		resultFile << "\t\t\t\"endTime\":" << to_string(pattern.endTime) << "," << endl;
        resultFile << "\t\t\t\"score\":" << to_string(pattern.totalScore) << "," << endl;
		resultFile << "\t\t\t\"vertexIds\":[";
		bool first2 = true;
		for (int vertexId : pattern.vertexIndices) {
			if (first2) {
				first2 = false;
			}
			else {
				resultFile << ",";
			}
			resultFile << "\""<<graph->getVertices()[vertexId].identifier<<"\"";
		}
		resultFile << "]," << endl;
		resultFile << "\t\t\t\"hashtags\":[";
		bool first3 = true;
		vector<pair<int, double>> hashVector;
		for (map<int, double>::iterator it = pattern.hashtags.begin(); it != pattern.hashtags.end(); it++) {
			pair<int, double> currentP;
			currentP.first = it->first;
			currentP.second = it->second;
			hashVector.push_back(currentP);
		}
		sort(hashVector.begin(), hashVector.end(), sortHashtags);

		for (vector<pair<int, double>>::iterator it = hashVector.begin(); it != hashVector.end(); it++) {
			if (first3) {
				first3 = false;
			}
			else {
				resultFile << ",";
			}
			resultFile << "{\"" << graph->getHashtags()[it->first] << "\":" << to_string(it->second) << "}";
		}
		resultFile << "]," << endl;
        
        resultFile << "\t\t\t\"tweetIdsInPattern\":[";
        first3=true;
        for (string tweetId : tweetsIdsOfPatterns[cpt]){
            if (first3) {
                first3 = false;
            }
            else {
                resultFile << ",";
            }
            if (designPoint.writeTweetsInDetails){
                Tweet & tweet=graph->getTweets()[tweetId];
                resultFile<<"{\"id\":\""<<tweet.identifier<<"\",\"latitude\":"<<to_string(tweet.latitude)<<",\"longitude\":"<<to_string(tweet.longitude)<<",\"timeStamp\":"<<to_string(tweet.timeStamp)<<",\"text\":\""<<tweet.text<<"\",\"instaUrls\":[";
                bool first=true;
                for (std::string &mediaUrl : tweet.mediaUrls){
                    if (first){
                        first=false;
                    }
                    else {
                            resultFile<<",";
                    }
                    resultFile<<"\""<<mediaUrl<<"\"";
                }
                resultFile<<"]}";
            }
            else {
                resultFile<<"\""<<tweetId<<"\"";
            }
            
        }
        resultFile << "]" << endl;
		resultFile << "\t\t}";
        cpt++;
	}
	resultFile << "\t]" << endl;
	resultFile << "}" << endl;
	resultFile.close();
    if (showCouts){
    std::cout<<"writing result finished"<<std::endl;
    }

}

bool sortHashtags(pair<int, double> h1, pair<int, double> h2)
{
	return (h1.second > h2.second);
}
