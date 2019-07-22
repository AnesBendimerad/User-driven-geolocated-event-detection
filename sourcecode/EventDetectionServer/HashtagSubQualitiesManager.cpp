//
//  HashtagSubQualitiesManager2.cpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 02/01/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//

#include "HashtagSubQualitiesManager.h"
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <math.h>
#include <algorithm>
#include "string_management.h"
#include "jacobi.hpp"

bool compareCoOcc (CoOccPerTime i,CoOccPerTime j) { return (i.timeIndex<j.timeIndex); }

void HashSubjQualitiesManager::loadHashtagsLastEventsIndices(string importanceOfHashtagsFilePath,unordered_map<string,int> & hashtagsMap){
    std::ifstream importanceOfHashtagsFile(importanceOfHashtagsFilePath);
    if (!importanceOfHashtagsFile.is_open()){
        std::cout<<"cannot open file"<<importanceOfHashtagsFilePath<<std::endl;
        throw "cannot open file";
    }
    hashtagsLastEventIndices.clear();
    string line;
    int cpt=0;
    while (std::getline(importanceOfHashtagsFile, line))
    {
        if (line.at(line.length() - 1) == '\r' || line.at(line.length() - 1) == '\n') {
            line = line.substr(0, line.length() - 1);
        }
        vector<string> elements=split(line,':');
        if (cpt==0){
            totalEventNumber=stod(elements[1].c_str());
        }
        else {
            if (hashtagsMap.find(elements[0])!=hashtagsMap.end()){
                hashtagsLastEventIndices[hashtagsMap[elements[0]]]=stod(elements[1].c_str());
            }
        }
        cpt++;
    }
    importanceOfHashtagsFile.close();
}

void HashSubjQualitiesManager::saveQualitiesOfHashtags(string importanceOfHashtagsFilePath,vector<string> & hashtagsAsString)
{
    ofstream importanceOfHashtagsFile(importanceOfHashtagsFilePath);
    for (int i=0;i<subjectiveQualities.size();i++){
        importanceOfHashtagsFile<<hashtagsAsString[i]<<":"<<to_string(subjectiveQualities[i])<<endl;
    }
    importanceOfHashtagsFile.close();
}

void HashSubjQualitiesManager::saveHashtagsLastEventsIndices(string importanceOfHashtagsFilePath,vector<string> & hashtagsAsString)
{
    ofstream importanceOfHashtagsFile(importanceOfHashtagsFilePath);
    importanceOfHashtagsFile<<"totalEventNumber:"<<to_string(totalEventNumber)<<endl;
    for (map<int,double>::iterator it=hashtagsLastEventIndices.begin();it!=hashtagsLastEventIndices.end();it++){
        importanceOfHashtagsFile<<hashtagsAsString[it->first]<<":"<<to_string(it->second)<<endl;
    }
    importanceOfHashtagsFile.close();
}
void HashSubjQualitiesManager::initHashtagsLastEventIndices(){
    hashtagsLastEventIndices.clear();
    subjectiveQualities.clear();
    totalEventNumber=0;
}

void HashSubjQualitiesManager::updateHashtagsLastEventsIndices(map<int, double> & hashtags){
    
    totalEventNumber++;
    for (map<int,double>::iterator it=hashtags.begin();it!=hashtags.end();it++){
        hashtagsLastEventIndices[it->first]=totalEventNumber;
    }
}

void HashSubjQualitiesManager::setValuesForBounds(int beginTime,int endTime){
    hashtagCooccurrences.clear();
    hashtagsTotalDegree.clear();
    if (endTime==-1){
        endTime=(int)hashtagCooccurrencesPerTime.size();
    }
    for (int i=beginTime;i<endTime;i++){
        std::unordered_map<int,std::unordered_map<int,int>> & currentHashtagCooccurrences=hashtagCooccurrencesPerTime[i];
        for (std::unordered_map<int,std::unordered_map<int,int>>::iterator it=currentHashtagCooccurrences.begin();it!=currentHashtagCooccurrences.end();it++){
            if (hashtagCooccurrences.find(it->first)==hashtagCooccurrences.end()){
                std::unordered_map<int,int> newMap;
                hashtagCooccurrences[it->first]=newMap;
                hashtagsTotalDegree[it->first]=0;
            }
            for (std::unordered_map<int,int>::iterator it2=it->second.begin();it2!=it->second.end();it2++){
                if (hashtagCooccurrences[it->first].find(it2->first)==hashtagCooccurrences[it->first].end()){
                    hashtagCooccurrences[it->first][it2->first]=it2->second;
                }
                else {
                    hashtagCooccurrences[it->first][it2->first]+=it2->second;
                }
                hashtagsTotalDegree[it->first]+=it2->second;
            }
        }
    }
}



void HashSubjQualitiesManager::calculateSubjectiveQualities(double alpha,double subjectivePower){
    HashSubjQualitiesManager::alpha=alpha;
    HashSubjQualitiesManager::subjectivePower=subjectivePower;
    vector<map<int,double>> aMatrix;
    subjectiveQualities.clear();
    map<int,double> bMatrix;
    // int hashtagNumber=(int)hashtagCooccurrences.size();
    for (int i=0;i<numberOfAllHashtags;i++){
        map<int,double> aMatrixI;
        if (hashtagCooccurrences.find(i)!=hashtagCooccurrences.end()){
            for (unordered_map<int,int>::iterator it=hashtagCooccurrences[i].begin();it!=hashtagCooccurrences[i].end();it++){
                double value=hashtagsTotalDegree[i];
                if (value==0){
                    cout<<"value equal to 0"<<endl;
                }
                aMatrixI[it->first]=alpha*(((double)it->second)/((double)hashtagsTotalDegree[i]));
            }
            aMatrixI[i]=-1;
        }
        else {
            aMatrixI[i]=(alpha-1);
        }
        
        aMatrix.push_back(aMatrixI);
        
        if (hashtagsLastEventIndices.find(i)!=hashtagsLastEventIndices.end()){
            if (totalEventNumber>0){
                bMatrix[i]=(alpha-1)*(1+subjectivePower/(1+log(totalEventNumber-hashtagsLastEventIndices[i]+1)));
            }
            else {
                bMatrix[i]=(alpha-1);
            }
        }
        else {
            bMatrix[i]=alpha-1;
        }
    }
    solveEquation(aMatrix, bMatrix, subjectiveQualities);
    
}

int HashSubjQualitiesManager::getNumberOfCoOccurrences(int termId1,int termId2, int beginTime,int endTime){
    int nb=0;
    for (int i=beginTime;i<endTime;i++){
        //std::cout<<"i:"<<to_string(i)<<std::endl;
        if (hashtagCooccurrencesPerTime[i].find(termId1)!=hashtagCooccurrencesPerTime[i].end()){
            if (hashtagCooccurrencesPerTime[i][termId1].find(termId2)!=hashtagCooccurrencesPerTime[i][termId1].end()){
                nb+=hashtagCooccurrencesPerTime[i][termId1][termId2];
            }
        }
    }
    return nb;
}
bool HashSubjQualitiesManager::isNbCoOcHigherThanThres(int termId1,int termId2, int beginTime,int endTime,int thres){
    int nb=0;
    // we first check in the overall dataset:
    // if they don't cooccure in the overall dataset, don't check in the space of the pattern :D
    if (hashtagCooccurrences.find(termId1)==hashtagCooccurrences.end()){
        return false;
        
    }
    std::unordered_map<int,int> & currentMap= hashtagCooccurrences[termId1];
    if (currentMap.find(termId2)==currentMap.end()){
        return false;
    }
    if (thres>1 && currentMap[termId2]<thres){
        return false;
    }
    std::vector<CoOccPerTime> & curVec=hashCoOccInverted[termId1][termId2];
    std::vector<CoOccPerTime>::iterator it;
    CoOccPerTime beginCC;
    beginCC.timeIndex=beginTime;
    beginCC.coOcc=0;
    it=std::lower_bound(curVec.begin(),curVec.end(),beginCC,compareCoOcc);
    
    while (it!=curVec.end() && it->timeIndex<endTime){
        nb+=it->coOcc;
        if (nb>=thres){
            return true;
        }
    }
    return false;
}















