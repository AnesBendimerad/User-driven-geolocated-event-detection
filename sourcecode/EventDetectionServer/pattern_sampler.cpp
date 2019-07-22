//
//  pattern_sampler.cpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 03/01/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//
#include <random>
#include "pattern_sampler.h"
#include "utils.hpp"
#include <algorithm>
#include <iostream>
#include "covering_management.h"
using namespace std;

std::random_device randomDevice;
std::mt19937 generator(randomDevice());
std::uniform_real_distribution<> distribution(0, 1);

PatternSampler::~ PatternSampler(){
    
    delete initProbaTable;
}

void PatternSampler::initializeInitProbaTable(){
    double totalValue=0;
    maxProbaTable=(int)(graph->getVertices().size()*(designPoint.endMineTime-designPoint.beginMineTime-designPoint.minTime+1));
    initProbaTable=new double[maxProbaTable];
    int currentIndex=0;
    for (int i=0;i<graph->getVertices().size();i++){
        for (int timeIndex=designPoint.beginMineTime;timeIndex<designPoint.endMineTime-designPoint.minTime+1;timeIndex++){
            int beginTime=timeIndex;
            int endTime=timeIndex+designPoint.minTime-1;
            set<int> validHashtags;
            validHashtags.insert(graph->getVertices()[i].periodicHashOcc[beginTime].positiveHashs.begin(),
                                 graph->getVertices()[i].periodicHashOcc[beginTime].positiveHashs.end());
            
            for (int time2=beginTime+1;time2<=endTime;time2++){
                TimeInterval & interval=graph->getVertices()[i].periodicHashOcc[time2];
                set<int> setIntersection;
                set_intersection(validHashtags.begin(), validHashtags.end(), interval.positiveHashs.begin(), interval.positiveHashs.end(),
                                 std::inserter(setIntersection, setIntersection.begin()));
                validHashtags = setIntersection;
            }
            
            if (currentIndex==0){
                initProbaTable[currentIndex]=0;
            }
            else {
                initProbaTable[currentIndex]=initProbaTable[currentIndex-1];
            }
            for (int hashId : validHashtags){
                for (int time2=beginTime;time2<=endTime;time2++){
                    initProbaTable[currentIndex]+=graph->getVertices()[i].periodicHashOcc[time2].hashScores[hashId];
                    totalValue+=graph->getVertices()[i].periodicHashOcc[time2].hashScores[hashId];
                }
                
            }
            validHashtagsPerProbaTableEl.push_back(validHashtags);
            currentIndex++;
        }
    }
    for (int i=0;i<maxProbaTable;i++){
        initProbaTable[i]/=totalValue;
    }
}

std::vector<Pattern> & PatternSampler::samplePatterns(DesignPoint designPoint){
    PatternSampler::designPoint=designPoint;
    struct timeb start, end;
    if (showCouts){
    std::cout<<"samplingTime in MS : "<<designPoint.samplingTimeInMS<<std::endl;
    std::cout<<"minquality: "<<designPoint.minQuality<<std::endl;
    }
    sampledPatterns.clear();
    initializeInitProbaTable();
    ftime(&start);
    bool continu=true;
    while (continu){
        
        sampleOnePattern();
        ftime(&end);
        double totalTime = getTimeComputationInMS(start, end);
        //std::cout<<"one sampling finished, total time :"<<totalTime<<std::endl;
        
        if (totalTime>=designPoint.samplingTimeInMS){
            continu=false;
        }
    }
    return sampledPatterns;
}

void PatternSampler::sampleOnePattern(){
    // 1. Draw a pattern P=(v,t)
    
    //cout<<"created begin pattern"<<endl;
    double value=distribution(generator);
    int chosen=0;
    while (chosen<maxProbaTable-1 && value>initProbaTable[chosen]){
        chosen++;
    }
    int vertexIndex=chosen/(designPoint.endMineTime-designPoint.beginMineTime-designPoint.minTime+1);
    int timeIndex=chosen % ((int)(designPoint.endMineTime-designPoint.beginMineTime-designPoint.minTime+1));
    
    timeIndex+=designPoint.beginMineTime;
    
    
    int numV=graph->getVertices().size();
    int numT=graph->getVertices()[0].periodicHashOcc.size();
    Pattern pattern;
    pattern.beginTime=timeIndex;
    pattern.endTime=timeIndex+designPoint.minTime-1;
    pattern.vertexIndices.insert(vertexIndex);
    pattern.totalScore=0;
    for (int hashId : validHashtagsPerProbaTableEl[chosen]){
        for (int time2=pattern.beginTime;time2<=pattern.endTime;time2++){
            if (time2==pattern.beginTime){
                pattern.hashtags[hashId]=graph->getVertices()[vertexIndex].periodicHashOcc[time2].hashScores[hashId];
            }
            else {
                pattern.hashtags[hashId]+=graph->getVertices()[vertexIndex].periodicHashOcc[time2].hashScores[hashId];
            }
        }
        pattern.totalScore+=pattern.hashtags[hashId];
    }
    
    
   // TimeInterval & interval=graph->getVertices()[vertexIndex].periodicHashOcc[timeIndex];
   // for (std::map<int, double>::iterator it=interval.hashScores.begin();it!=interval.hashScores.end();it++){
   //     pattern.hashtags[it->first]=it->second;
    //}
    
    // 2. launch sampleOnePatternByExtention
    //cout<<"begin loop expansion"<<endl;
    bool continu=true;
    while (continu){
        continu=samplePatternByExtention(pattern);
    }
}

bool PatternSampler::samplePatternByExtention(Pattern & currentPattern){
    vector<Pattern> candidatesPatterns;
    vector<double> scoresOfCandidates;
    candidatesPatterns.push_back(currentPattern);
    double totalScore=0;
    //for (map<int,double>::iterator it=currentPattern.hashtags.begin();it!=currentPattern.hashtags.end();it++){
    //    totalScore+=it->second;
    //}
    totalScore=currentPattern.totalScore;
    scoresOfCandidates.push_back(totalScore);
    double minScore=totalScore;
    
    set<int> allNeighbors;
    for (int vertexIndex : currentPattern.vertexIndices){
        allNeighbors.insert(graph->getVertices()[vertexIndex].neighbors.begin(),graph->getVertices()[vertexIndex].neighbors.end());
    }
    
    set<int> setDif;
    set_difference(allNeighbors.begin(), allNeighbors.end(), currentPattern.vertexIndices.begin(), currentPattern.vertexIndices.end(),std::inserter(setDif, setDif.begin()));
    allNeighbors = setDif;
    
    
    
    
    
    
    
    for (int neighborIndex : allNeighbors){
        Vertex & candV=graph->getVertices()[neighborIndex];
        Pattern currentCand;
        currentCand.beginTime=currentPattern.beginTime;
        currentCand.endTime=currentPattern.endTime;
        currentCand.vertexIndices.insert(currentPattern.vertexIndices.begin(),currentPattern.vertexIndices.end());
        currentCand.vertexIndices.insert(neighborIndex);
        set<int> validHashtags;
        
        
        for (std::map<int, double>::iterator it=currentPattern.hashtags.begin();it!=currentPattern.hashtags.end();it++){
            validHashtags.insert(it->first);
        }
        for (int time=currentCand.beginTime;time<=currentCand.endTime;time++){
            set<int> setIntersection;
            set_intersection(validHashtags.begin(), validHashtags.end(), candV.periodicHashOcc[time].positiveHashs.begin(), candV.periodicHashOcc[time].positiveHashs.end(),
                             std::inserter(setIntersection, setIntersection.begin()));
            validHashtags = setIntersection;
        }
        double candQuality=0;
        for (int hashId: validHashtags){
            currentCand.hashtags[hashId]=currentPattern.hashtags[hashId];
            for (int time=currentCand.beginTime;time<=currentCand.endTime;time++){
                currentCand.hashtags[hashId]+=candV.periodicHashOcc[time].hashScores[hashId];
            }
            candQuality+=currentCand.hashtags[hashId];
        }
        currentCand.totalScore=candQuality;
        candidatesPatterns.push_back(currentCand);
        totalScore+=candQuality;
        scoresOfCandidates.push_back(candQuality);
        if (minScore>candQuality){
            minScore=candQuality;
        }
        
    }
    if (currentPattern.beginTime>designPoint.beginMineTime){
        Pattern currentCand;
        currentCand.beginTime=currentPattern.beginTime-1;
        currentCand.endTime=currentPattern.endTime;
        currentCand.vertexIndices.insert(currentPattern.vertexIndices.begin(),currentPattern.vertexIndices.end());
        set<int> validHashtags;
        for (std::map<int, double>::iterator it=currentPattern.hashtags.begin();it!=currentPattern.hashtags.end();it++){
            validHashtags.insert(it->first);
        }
        for (int vertexId : currentCand.vertexIndices){
            set<int> setIntersection;
            TimeInterval &interval=graph->getVertices()[vertexId].periodicHashOcc[currentCand.beginTime];
            set_intersection(validHashtags.begin(), validHashtags.end(),interval.positiveHashs.begin(), interval.positiveHashs.end(), std::inserter(setIntersection, setIntersection.begin()));
            validHashtags = setIntersection;
        }
        double candQuality=0;
        for (int hashId: validHashtags){
            currentCand.hashtags[hashId]=currentPattern.hashtags[hashId];
            for (int vertexId : currentCand.vertexIndices){
                TimeInterval &interval=graph->getVertices()[vertexId].periodicHashOcc[currentCand.beginTime];
                currentCand.hashtags[hashId]+=interval.hashScores[hashId];
            }
            candQuality+=currentCand.hashtags[hashId];
        }
        currentCand.totalScore=candQuality;
        candidatesPatterns.push_back(currentCand);
        totalScore+=candQuality;
        scoresOfCandidates.push_back(candQuality);
        if (minScore>candQuality){
            minScore=candQuality;
        }
    }
    if (currentPattern.endTime<designPoint.endMineTime-1){
        Pattern currentCand;
        currentCand.beginTime=currentPattern.beginTime;
        currentCand.endTime=currentPattern.endTime+1;
        currentCand.vertexIndices.insert(currentPattern.vertexIndices.begin(),currentPattern.vertexIndices.end());
        set<int> validHashtags;
        for (std::map<int, double>::iterator it=currentPattern.hashtags.begin();it!=currentPattern.hashtags.end();it++){
            validHashtags.insert(it->first);
        }
        for (int vertexId : currentCand.vertexIndices){
            set<int> setIntersection;
            TimeInterval &interval=graph->getVertices()[vertexId].periodicHashOcc[currentCand.endTime];
            set_intersection(validHashtags.begin(), validHashtags.end(),interval.positiveHashs.begin(), interval.positiveHashs.end(),std::inserter(setIntersection, setIntersection.begin()));
            validHashtags = setIntersection;
        }
        double candQuality=0;
        for (int hashId: validHashtags){
            currentCand.hashtags[hashId]=currentPattern.hashtags[hashId];
            for (int vertexId : currentCand.vertexIndices){
                TimeInterval &interval=graph->getVertices()[vertexId].periodicHashOcc[currentCand.endTime];
                currentCand.hashtags[hashId]+=interval.hashScores[hashId];
            }
            candQuality+=currentCand.hashtags[hashId];
        }
        currentCand.totalScore=candQuality;
        candidatesPatterns.push_back(currentCand);
        totalScore+=candQuality;
        scoresOfCandidates.push_back(candQuality);
        if (minScore>candQuality){
            minScore=candQuality;
        }
    }
    totalScore=0;
    
    // instruction to remove minscore value :
    minScore=0;
    //
    
    for (int i=0;i<scoresOfCandidates.size();i++){
        scoresOfCandidates[i]-=minScore;
        totalScore+=scoresOfCandidates[i];
        scoresOfCandidates[i]=totalScore;
    }
    for (int i=0;i<scoresOfCandidates.size();i++){
        scoresOfCandidates[i]/=totalScore;
    }
    
    double value=distribution(generator);
    int chosen=0;
    while (chosen<scoresOfCandidates.size()-1 && value>scoresOfCandidates[chosen]){
        chosen++;
    }
    currentPattern=candidatesPatterns[chosen];
    
    if (chosen>0){
        return true;
    }
    else {
        if (!useMinQuality){
            sampledPatterns.push_back(candidatesPatterns[chosen]);
        }
        else if (candidatesPatterns[chosen].totalScore>=minQuality){
            sampledPatterns.push_back(candidatesPatterns[chosen]);
        }
        
        return false;
    }
}


void PatternSampler::removeRepetition(){
    if (useMinQuality){
        vector<Pattern> filteredPatterns;
        for (Pattern & pattern :sampledPatterns){
            pattern.totalScore=0;
            for (map<int,double>::iterator it=pattern.hashtags.begin();it!=pattern.hashtags.end();it++){
                pattern.totalScore+=it->second;
            }
            if (pattern.totalScore>=minQuality){
                filteredPatterns.push_back(pattern);
            }
        }
        sampledPatterns=filteredPatterns;
    }
    
    vector<Pattern> cleanedPatterns;
    for (Pattern & pattern : sampledPatterns){
        bool repeated=false;
        for (Pattern & p2:cleanedPatterns){
            if (equals(pattern, p2)){
                repeated=true;
                break;
            }
        }
        if (!repeated){
            cleanedPatterns.push_back(pattern);
        }
    }
    sampledPatterns=cleanedPatterns;
}

void PatternSampler::removeRedundancy(){
    // this function remove redundancy with mincov, and patterns with quality<minQuality
    
    if (useMinQuality){
        vector<Pattern> filteredPatterns;
        for (Pattern & pattern :sampledPatterns){
            pattern.totalScore=0;
            for (map<int,double>::iterator it=pattern.hashtags.begin();it!=pattern.hashtags.end();it++){
                pattern.totalScore+=it->second;
            }
            if (pattern.totalScore>=minQuality){
                filteredPatterns.push_back(pattern);
            }
        }
        sampledPatterns=filteredPatterns;
    }
    
    vector<Pattern> cleanedPatterns;
    for (Pattern & pattern : sampledPatterns){
        if (!isPatternCovered(graph,pattern, cleanedPatterns, designPoint.minCov)){
            cleanedPatterns.push_back(pattern);
        }
    }
    sampledPatterns=cleanedPatterns;
}











