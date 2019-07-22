//
//  utils.cpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 02/01/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//

#include "utils.hpp"
#include <stdlib.h>   
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include "string_management.h"
using namespace std;


double getTimeComputationInS(struct timeb &start, struct timeb &end)
{
    return end.time - start.time + (double)(end.millitm - start.millitm) / 1000;
}
double getTimeComputationInMS(struct timeb &start, struct timeb &end)
{
    return (end.time - start.time)*1000 + (double)(end.millitm - start.millitm);
}
void readParametersContinuePreferenceBased(std::string paramFilePath,DesignPoint & designPoint){
    
    bool success=false;
    int tentative=0;
    // set to default values
    setToDefaultValues(designPoint);
    designPoint.prefBased=true;
    
    
    while (tentative<10 && !success){
    if (showCouts){
    std::cout<<"read parameters from file:"<<paramFilePath<<std::endl;
    }
    set<string> notAlreadyInited;
    notAlreadyInited.insert("outputFile"); //
        
    notAlreadyInited.insert("approach");//
    notAlreadyInited.insert("postProcessing");//
    notAlreadyInited.insert("samplingTimeInMS");//
    notAlreadyInited.insert("userId");
    //userId
    std::ifstream paramFile(paramFilePath);
    
    if (!paramFile.is_open()){
        std::cout<<"problem : file not opened"<<std::endl;
        std::chrono::milliseconds timespan(1000);
        std::this_thread::sleep_for(timespan);
        tentative++;
        continue;
    }
    
    success=true;
    std::string line;
    while (std::getline(paramFile, line))
    {
        if (line.at(line.length() - 1) == '\r' || line.at(line.length() - 1) == '\n') {
            line = line.substr(0, line.length() - 1);
        }
        if (showCouts){
        std::cout<<line<<std::endl;
        }
        vector<string> elements=split(line,':');
         if(elements[0].compare("outputFile")==0){
            notAlreadyInited.erase("outputFile");
            designPoint.outputFilePath=elements[1];
        }
        else if(elements[0].compare("fileContainingLikedPatterns")==0){
            designPoint.fileContainingLikedPatterns=elements[1];
        }
        else if(elements[0].compare("chosenPatterns")==0){
            vector<string> elements2=split(elements[1],',');
            for (string el : elements2){
                designPoint.chosenPatterns.push_back(atoi(el.c_str()));
            }
        }
        else if(elements[0].compare("stDevWeightFactor")==0){
            designPoint.stDevWeightFactor=stod(elements[1].c_str());
        }
        else if(elements[0].compare("minCov")==0){
            designPoint.minCov=stod(elements[1].c_str());
        }
        else if(elements[0].compare("minQuality")==0){
            designPoint.minQuality=stod(elements[1].c_str());
        }
        else if(elements[0].compare("minSize")==0){
            
            designPoint.minSize=stod(elements[1].c_str());
        }
        else if(elements[0].compare("minTime")==0){
            designPoint.minTime=stod(elements[1].c_str());
        }
        else if(elements[0].compare("alpha")==0){
            designPoint.alpha=stod(elements[1].c_str());
        }
        else if(elements[0].compare("subjectivePower")==0){
            designPoint.subjectivePower=stod(elements[1].c_str());
        }
        else if(elements[0].compare("beginMineTime")==0){
            designPoint.beginMineTime=atoi(elements[1].c_str());
        }
        else if(elements[0].compare("endMineTime")==0){
            designPoint.endMineTime=atoi(elements[1].c_str());
        }
        else if(elements[0].compare("approach")==0){
            notAlreadyInited.erase("approach");
            designPoint.approach=elements[1];
        }
        else if(elements[0].compare("postProcessing")==0){
            notAlreadyInited.erase("postProcessing");
            designPoint.postProcessing=elements[1];
        }
        else if(elements[0].compare("samplingTimeInMS")==0){
            notAlreadyInited.erase("samplingTimeInMS");
            designPoint.samplingTimeInMS=atoi(elements[1].c_str());
        }
        else if(elements[0].compare("modelIsAll")==0){
            string data=elements[1];
            std::transform(data.begin(), data.end(), data.begin(), ::tolower);
            if (data.compare("true")==0){
                designPoint.modelIsAll=true;
            }
            else {
                designPoint.modelIsAll=false;
            }
            
        }
        else if(elements[0].compare("writeTweetsInDetails")==0){
            string data=elements[1];
            std::transform(data.begin(), data.end(), data.begin(), ::tolower);
            if (data.compare("true")==0){
                designPoint.writeTweetsInDetails=true;
            }
            else {
                designPoint.writeTweetsInDetails=false;
            }
        }
        else if(elements[0].compare("userId")==0){
            designPoint.userId=atoi(elements[1].c_str());
            notAlreadyInited.erase("userId");
        }
        else if(elements[0].compare("filterBySizes")==0){
            string data=elements[1];
            std::transform(data.begin(), data.end(), data.begin(), ::tolower);
            if (data.compare("true")==0){
                designPoint.filterBySizes=true;
            }
            else {
                designPoint.filterBySizes=false;
            }
        }
        else if (elements[0].compare("clusterize")==0){//clusterize
            string data=elements[1];
            std::transform(data.begin(), data.end(), data.begin(), ::tolower);
            if (data.compare("true")==0){
                designPoint.clusterize=true;
            }
            else {
                designPoint.clusterize=false;
            }
        }
        else {
            std::cerr<<"unrecognized parameter"<<std::endl;
            throw "unrecognized parameter";
        }
    }
    paramFile.close();
    if (showCouts){
    std::cout<<"parameters file closed"<<std::endl;
    }
    if (notAlreadyInited.find("approach")!=notAlreadyInited.end()){
        notAlreadyInited.erase("approach");
        designPoint.approach="complete";
        notAlreadyInited.erase("postProcessing");
        notAlreadyInited.erase("samplingTimeInMS");
    }
    else if (designPoint.approach.compare("complete")==0){
        notAlreadyInited.erase("postProcessing");
        notAlreadyInited.erase("samplingTimeInMS");
    }
    
    
    if (notAlreadyInited.size()>0){
        std::cout<<"some parameters are missing:";
        for (std::string param :notAlreadyInited){
            std::cout<<param<<",";
        }
        std::cout<<std::endl;
        std::cerr<<"some parameters are missing"<<std::endl;
        throw "some parameters are missing";
    }
    }
    if (!success){
        std::cerr<<"openning file failed"<<std::endl;
        throw "openning file failed";
    }
    if (showCouts){
    std::cout<<"parameters already read"<<std::endl;
    }
}
void readParametersLaunchPreferenceBased(std::string paramFilePath,DesignPoint & designPoint){
    bool success=false;
    int tentative=0;
    setToDefaultValues(designPoint);
    designPoint.prefBased=true;
    
    while (tentative<10 && !success){

        
    set<string> notAlreadyInited;
    notAlreadyInited.insert("graphFile");
    notAlreadyInited.insert("hashtagCooccurrencesFile");
    notAlreadyInited.insert("userId");
    std::string line;
    std::ifstream paramFile(paramFilePath);
    
        if (!paramFile.is_open()){
            std::cout<<"problem : file not opened"<<std::endl;
            std::chrono::milliseconds timespan(1000);
            std::this_thread::sleep_for(timespan);
            tentative++;
            continue;
        }
        
        success=true;
        
    while (std::getline(paramFile, line))
    {
        if (line.at(line.length() - 1) == '\r' || line.at(line.length() - 1) == '\n') {
            line = line.substr(0, line.length() - 1);
        }
        vector<string> elements=split(line,':');
        if (elements[0].compare("graphFile")==0){
            notAlreadyInited.erase("graphFile");
            designPoint.graphFilePath=elements[1];
        }
        else if(elements[0].compare("hashtagCooccurrencesFile")==0){
            notAlreadyInited.erase("hashtagCooccurrencesFile");
            designPoint.hashtagCooccurrencesFilePath=elements[1];
        }
        else if(elements[0].compare("tweetsFilePath")==0){
            designPoint.tweetsFilePath=elements[1];
        }
        else if(elements[0].compare("writeTweetsInDetails")==0){
            string data=elements[1];
            std::transform(data.begin(), data.end(), data.begin(), ::tolower);
            if (data.compare("true")==0){
                designPoint.writeTweetsInDetails=true;
            }
            else {
                designPoint.writeTweetsInDetails=false;
            }
        }
        else if(elements[0].compare("userId")==0){
            designPoint.userId=atoi(elements[1].c_str());
            notAlreadyInited.erase("userId");
        }
        else {
            std::cerr<< "unrecognized parameter" <<std::endl;
            throw "unrecognized parameter";
        }
    }
    paramFile.close();
    if (notAlreadyInited.size()>0){
        std::cerr<< "some parameters are missing" <<std::endl;
        throw "some parameters are missing";
    }
    
    designPoint.beginMineTime=0;
    designPoint.endMineTime=-1;
    }
    if (!success){
        std::cerr<<"openning file failed"<<std::endl;
        throw "openning file failed";
    }
}


void readParametersSimpleDetection(string paramFilePath,DesignPoint & designPoint){
    setToDefaultValues(designPoint);
    bool success=false;
    int tentative=0;
    designPoint.prefBased=false;
    while (tentative<10 && !success){
    set<string> notAlreadyInited;
    notAlreadyInited.insert("graphFile");
    notAlreadyInited.insert("outputFile");
    notAlreadyInited.insert("approach");
    notAlreadyInited.insert("postProcessing");
    notAlreadyInited.insert("samplingTimeInMS");
    notAlreadyInited.insert("hashtagCooccurrencesFile");
    notAlreadyInited.insert("userId");
        
    std::ifstream paramFile(paramFilePath);
        if (!paramFile.is_open()){
            std::cout<<"problem : file not opened"<<std::endl;
            std::chrono::milliseconds timespan(1000);
            std::this_thread::sleep_for(timespan);
            tentative++;
            continue;
        }
        success=true;

    std::string line;
    std::string fileAsString;
    while (std::getline(paramFile, line))
    {
        if (showCouts){
        cout<<line<<endl;
        }
        if (line.at(line.length() - 1) == '\r' || line.at(line.length() - 1) == '\n') {
            line = line.substr(0, line.length() - 1);
        }
        vector<string> elements=split(line,':');
        if (elements[0].compare("graphFile")==0){
            notAlreadyInited.erase("graphFile");
            designPoint.graphFilePath=elements[1];
        }
        else if(elements[0].compare("outputFile")==0){
            notAlreadyInited.erase("outputFile");
            designPoint.outputFilePath=elements[1];
        }
        else if(elements[0].compare("stDevWeightFactor")==0){
            designPoint.stDevWeightFactor=stod(elements[1].c_str());
        }
        else if(elements[0].compare("minCov")==0){
            designPoint.minCov=stod(elements[1].c_str());
        }
        else if(elements[0].compare("minQuality")==0){
            designPoint.minQuality=stod(elements[1].c_str());
        }
        else if(elements[0].compare("minSize")==0){
            designPoint.minSize=stod(elements[1].c_str());
        }
        else if(elements[0].compare("minTime")==0){
            designPoint.minTime=stod(elements[1].c_str());
            
        }
        else if(elements[0].compare("beginMineTime")==0){
            designPoint.beginMineTime=atoi(elements[1].c_str());
        }
        else if(elements[0].compare("endMineTime")==0){
            designPoint.endMineTime=atoi(elements[1].c_str());
        }
        else if(elements[0].compare("approach")==0){
            notAlreadyInited.erase("approach");
            designPoint.approach=elements[1];
        }
        else if(elements[0].compare("postProcessing")==0){
            notAlreadyInited.erase("postProcessing");
            designPoint.postProcessing=elements[1];
        }
        else if(elements[0].compare("samplingTimeInMS")==0){
            notAlreadyInited.erase("samplingTimeInMS");
            designPoint.samplingTimeInMS=atoi(elements[1].c_str());
        }
        else if(elements[0].compare("hashtagCooccurrencesFile")==0){
            notAlreadyInited.erase("hashtagCooccurrencesFile");
            designPoint.hashtagCooccurrencesFilePath=elements[1];
        }
        else if(elements[0].compare("tweetsFilePath")==0){
            designPoint.tweetsFilePath=elements[1];
        }
        else if(elements[0].compare("modelIsAll")==0){
            string data=elements[1];
            std::transform(data.begin(), data.end(), data.begin(), ::tolower);
            if (data.compare("true")==0){
                designPoint.modelIsAll=true;
            }
            else {
                designPoint.modelIsAll=false;
            }
            
        }
        else if(elements[0].compare("writeTweetsInDetails")==0){
            string data=elements[1];
            std::transform(data.begin(), data.end(), data.begin(), ::tolower);
            if (data.compare("true")==0){
                designPoint.writeTweetsInDetails=true;
            }
            else {
                designPoint.writeTweetsInDetails=false;
            }
        }
        else if(elements[0].compare("userId")==0){
            designPoint.userId=atoi(elements[1].c_str());
            notAlreadyInited.erase("userId");
        }
        else if(elements[0].compare("filterBySizes")==0){
            string data=elements[1];
            std::transform(data.begin(), data.end(), data.begin(), ::tolower);
            if (data.compare("true")==0){
                designPoint.filterBySizes=true;
            }
            else {
                designPoint.filterBySizes=false;
            }
        }
        else if (elements[0].compare("clusterize")==0){//clusterize
            string data=elements[1];
            std::transform(data.begin(), data.end(), data.begin(), ::tolower);
            if (data.compare("true")==0){
                designPoint.clusterize=true;
            }
            else {
                designPoint.clusterize=false;
            }
        }
        else {
            std::cerr<<"unrecognized parameter"<<std::endl;
            throw "unrecognized parameter";
        }
    }
    paramFile.close();
    if (notAlreadyInited.find("approach")!=notAlreadyInited.end()){
        notAlreadyInited.erase("approach");
        designPoint.approach="complete";
        notAlreadyInited.erase("postProcessing");
        notAlreadyInited.erase("samplingTimeInMS");
    }
    else if (designPoint.approach.compare("complete")==0){
        notAlreadyInited.erase("postProcessing");
        notAlreadyInited.erase("samplingTimeInMS");
    }

    if (notAlreadyInited.size()>0){
        std::cerr<<"some parameters are missing"<<std::endl;
        throw "some parameters are missing";
    }
    }
    if (!success){
        std::cerr<<"openning file failed"<<std::endl;
        throw "openning file failed";
    }
    
}
void readParametersAddNewLikes(std::string paramFilePath,DesignPoint & designPoint){
    setToDefaultValues(designPoint);
    bool success=false;
    int tentative=0;
    while (tentative<10 && !success){
        set<string> notAlreadyInited;
        notAlreadyInited.insert("fileContainingLikedPatterns");
        notAlreadyInited.insert("chosenPatterns");
        notAlreadyInited.insert("userId");
        std::ifstream paramFile(paramFilePath);
        if (!paramFile.is_open()){
            std::cout<<"problem : file not opened"<<std::endl;
            std::chrono::milliseconds timespan(1000);
            std::this_thread::sleep_for(timespan);
            tentative++;
            continue;
        }
        success=true;
        
        std::string line;
        std::string fileAsString;
        while (std::getline(paramFile, line))
        {
            if (showCouts){
            cout<<line<<endl;
            }
            if (line.at(line.length() - 1) == '\r' || line.at(line.length() - 1) == '\n') {
                line = line.substr(0, line.length() - 1);
            }
            vector<string> elements=split(line,':');
            if (elements[0].compare("fileContainingLikedPatterns")==0){
                notAlreadyInited.erase("fileContainingLikedPatterns");
                designPoint.fileContainingLikedPatterns=elements[1];
            }
            else if(elements[0].compare("chosenPatterns")==0){
                vector<string> elements2=split(elements[1],',');
                for (string el : elements2){
                    designPoint.chosenPatterns.push_back(atoi(el.c_str()));
                }
            }
            else if(elements[0].compare("userId")==0){
                designPoint.userId=atoi(elements[1].c_str());
                notAlreadyInited.erase("userId");
            }
            else {
                std::cerr<<"unrecognized parameter"<<std::endl;
                throw "unrecognized parameter";
            }
        }
        paramFile.close();
        if (notAlreadyInited.size()>0){
            std::cerr<<"some parameters are missing"<<std::endl;
            throw "some parameters are missing";
        }
    }
    if (!success){
        std::cerr<<"openning file failed"<<std::endl;
        throw "openning file failed";
    }
}

void setToDefaultValues(DesignPoint & designPoint){
    designPoint.alpha=0.7;
    designPoint.beginMineTime=0;
    designPoint.endMineTime=-1;
    designPoint.minCov=0.8;
    designPoint.minQuality=40;
    designPoint.minSize=1;
    designPoint.minTime=1;
    designPoint.modelIsAll=true;
    designPoint.stDevWeightFactor=1;
    designPoint.subjectivePower=2;
    designPoint.tweetsFilePath="";
    designPoint.writeTweetsInDetails=false;
    designPoint.userId=-1;
    designPoint.filterBySizes=false;
    designPoint.clusterize=true;
}








