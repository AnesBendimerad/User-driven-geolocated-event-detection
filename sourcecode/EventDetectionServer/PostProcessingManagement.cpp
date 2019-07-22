//
//  PostProcessingManagement.cpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 30/08/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//

#include "PostProcessingManagement.h"
#include <fstream>
#include <iostream>
#include <math.h>
#include <algorithm>
#include "utils.hpp"
std::string fileSeparator2;
void postProcess(Graph *graph,std::vector<Pattern> & retrievedPatterns,Statistics & statistics,DesignPoint designPoint,std::vector<Pattern> &finalPatterns){
#ifdef _WIN32
    //define something for Windows
    fileSeparator2 = "\\";
    
#else
    //define it for a Unix machine
    fileSeparator2 = "/";
#endif
    struct timeb start, end;
    struct timeb start2, end2;
    ftime(&start);
    std::string outputFolderPath;
    if (designPoint.outputFilePath.find_last_of(fileSeparator2)<designPoint.outputFilePath.size()){
        outputFolderPath=designPoint.outputFilePath.substr(0,designPoint.outputFilePath.find_last_of(fileSeparator2)+1);
    }
    else {
        outputFolderPath=".";
    }
    statistics.clusWritingTime=0;
    std::string inputForLouvainPath=outputFolderPath+fileSeparator2+"network.txt";
    std::map<int, int> louvIndexToPatternIdMap;
    std::map<int, int> louvIndexToTermIdMap;
    int curLouvId=0;
    std::ofstream inputLouvain;
    inputLouvain.open(inputForLouvainPath);
    
    // iterating, pattern by pattern
    int pIndex=0;
    
    if (showCouts){
        std::cout<<"preparing patterns for louvain"<<std::endl;
    }
    
    for (Pattern &pattern : retrievedPatterns){
        // we first give to each term a louvainId
        std::map<int, int> termIdToLouvIdForClust;
        for (std::map<int, double>::iterator it = pattern.hashtags.begin(); it != pattern.hashtags.end(); it++) {
            termIdToLouvIdForClust[it->first]=curLouvId;
            louvIndexToPatternIdMap[curLouvId]=pIndex;
            louvIndexToTermIdMap[curLouvId]=it->first;
            curLouvId++;
        }
        // we add to file the connections
        for (std::map<int, double>::iterator it = pattern.hashtags.begin(); it != pattern.hashtags.end(); it++) {
            std::map<int, double>::iterator it2=it;
            it2++;
            for (;it2!=pattern.hashtags.end();it2++){
                // if it->first and it2->first co-occur in the timeIntervals of the pattern, then add connection to file
                if (graph->hashtagCooccurrencesGraph->isNbCoOcHigherThanThres(it->first, it2->first, designPoint.beginMineTime, designPoint.endMineTime,MIN_WEIGHT_COOCC)){
                    ftime(&start2);
                    inputLouvain<<std::to_string(termIdToLouvIdForClust[it->first])<<"\t"<<std::to_string(termIdToLouvIdForClust[it2->first])<<std::endl;
                    ftime(&end2);
                    statistics.clusWritingTime+=getTimeComputationInS(start2, end2);
                    
                }
            }
        }
        pIndex++;
    }
    inputLouvain.close();
    
    ftime(&end);
    statistics.preparClustTime = getTimeComputationInS(start, end);
    ftime(&start);
    // apply louvain
    std::string cmd="java -Xmx4096m -jar ModularityOptimizer.jar "+outputFolderPath+fileSeparator2+"network.txt "+outputFolderPath+fileSeparator2+"communities.txt 2 0.1 1 10 10 0 0";
    system(cmd.c_str());
    ftime(&end);
    statistics.jarTime = getTimeComputationInS(start, end);
    
    
    ftime(&start);
    // open result file
    std::ifstream outLouvainFile(outputFolderPath+fileSeparator2+"communities.txt");
    
    std::map<int,std::vector<int>> clustersMap;
    std::string line;
    int cpt=0;
    while (std::getline(outLouvainFile, line))
    {
        int curNum=std::stoi(line);
        if (clustersMap.find(curNum)==clustersMap.end()){
            std::vector<int> newVec;
            clustersMap[curNum]=newVec;
        }
        clustersMap[curNum].push_back(cpt);
        cpt++;
    }
    
    outLouvainFile.close();
    
    std::vector<TempoPattern> dividedPatterns;
    
    for (std::map<int,std::vector<int>>::iterator it = clustersMap.begin(); it != clustersMap.end(); it++) {
        std::vector<int> louvIndicesOfTerms=it->second;
        
        TempoPattern newP;
        // init begin and end time for newP
        Pattern &fatherPatt=retrievedPatterns[louvIndexToPatternIdMap[louvIndicesOfTerms[0]]];
        newP.fatherIndex=louvIndexToPatternIdMap[louvIndicesOfTerms[0]];
        
                // init hashtags and totalScore
        newP.totalScore=0;
        for (int louvInd : louvIndicesOfTerms){
            int termId=louvIndexToTermIdMap[louvInd];
            //newP.hashtagsIds.insert(termId);
            newP.hashtags[termId]=fatherPatt.hashtags[termId];
            newP.totalScore+=fatherPatt.hashtags[termId];
        }
        // add newP if minScore is good
        if (newP.totalScore>=designPoint.minQuality){
            dividedPatterns.push_back(newP);
        }
    }
    
    // clustering events
    inputLouvain.open(outputFolderPath+fileSeparator2+"network2.txt");
    int cpt1=0;
    for (std::vector<TempoPattern>::iterator it = dividedPatterns.begin(); it != dividedPatterns.end(); it++) {
        std::vector<TempoPattern>::iterator it2=it;
        it2++;
        int cpt2=cpt1+1;
        for (;it2 != dividedPatterns.end(); it2++){
            double patSim=getSimilarityOfTwoPatterns2(graph,*it, *it2,retrievedPatterns);
            if (patSim>MIN_SIM_PAT){
                // write sim in inputLouvain
                ftime(&start2);
                inputLouvain<<std::to_string(cpt1)<<"\t"<<std::to_string(cpt2)<<"\t"<<std::to_string(patSim)<<std::endl;
                ftime(&end2);
                statistics.clusWritingTime+=getTimeComputationInS(start2, end2);
            }
            cpt2++;
        }
        cpt1++;
    }
    
    inputLouvain.close();
    
    ftime(&end);
    statistics.preparClustTime += getTimeComputationInS(start, end);
    
    ftime(&start); //-Xmx4096m
    cmd="java -Xmx4096m -jar ModularityOptimizer.jar "+outputFolderPath+fileSeparator2+"network2.txt "+outputFolderPath+fileSeparator2+"communities2.txt 2 0.1 1 10 10 0 0";
    system(cmd.c_str());
    ftime(&end);
    statistics.jarTime += getTimeComputationInS(start, end);

    
    // reading results
    outLouvainFile.open(outputFolderPath+fileSeparator2+"communities2.txt");
    
    std::map<int,std::vector<int>> clustersMap2;
    
    cpt=0;
    while (std::getline(outLouvainFile, line))
    {
        int curNum=std::stoi(line);
        if (clustersMap2.find(curNum)==clustersMap2.end()){
            std::vector<int> newVec;
            clustersMap2[curNum]=newVec;
        }
        clustersMap2[curNum].push_back(cpt);
        cpt++;
    }
    
    outLouvainFile.close();
    
    // creating final patterns and recalculating score
    
    finalPatterns.clear();
    for (std::map<int,std::vector<int>>::iterator it = clustersMap2.begin(); it != clustersMap2.end(); it++) {
        Pattern newP;
        std::set<int> termsSet;
        std::vector<int> patIndices=it->second;
        
        newP.beginTime=retrievedPatterns[dividedPatterns[patIndices[0]].fatherIndex].beginTime;
        newP.endTime=retrievedPatterns[dividedPatterns[patIndices[0]].fatherIndex].endTime;
        for (int patId : patIndices){
            TempoPattern & tempoP=dividedPatterns[patId];
            Pattern &curP=retrievedPatterns[tempoP.fatherIndex];
            if (curP.beginTime<newP.beginTime){
                newP.beginTime=curP.beginTime;
            }
            if (curP.endTime>newP.endTime){
                newP.endTime=curP.endTime;
            }
            for (int i : curP.vertexIndices){
                newP.vertexIndices.insert(i);
            }
            for (std::unordered_map<int, double>::iterator it=tempoP.hashtags.begin();it!=tempoP.hashtags.end();it++){
                termsSet.insert(it->first);
            }
        }
        // now we must just calculate scores of terms
        newP.totalScore=0;
        for (int termId : termsSet){
            double score=0;
            for (int vertexId : newP.vertexIndices){
                for (int timeId = newP.beginTime;timeId<=newP.endTime;timeId++){
                    if (graph->getVertices()[vertexId].periodicHashOcc[timeId].positiveHashs.find(termId)!=graph->getVertices()[vertexId].periodicHashOcc[timeId].positiveHashs.end()){
                        score+=graph->getVertices()[vertexId].periodicHashOcc[timeId].hashScores[termId];
                    }
                }
            }
            newP.hashtags[termId]=score;
            newP.totalScore+=score;
        }
        finalPatterns.push_back(newP);
    }
    
    //finalPatterns.clear();
    //finalPatterns.insert(finalPatterns.begin(), dividedPatterns.begin(),dividedPatterns.end());
}


double getSimilarityOfTwoPatterns(Pattern & p1,Pattern & p2){
    // calculating terms similarity
    double nbIntersection=0;
    double nbTermsP1=(double)p1.hashtags.size();
    double nbTermsP2=(double)p2.hashtags.size();
    for (std::map<int, double>::iterator it = p1.hashtags.begin(); it != p1.hashtags.end(); it++) {
        if (p2.hashtags.find(it->first)!=p2.hashtags.end()){
            nbIntersection++;
        }
    }
    double simTerms;
    if (nbTermsP1>nbTermsP2){
        simTerms=nbIntersection/nbTermsP2;
    }
    else {
        simTerms=nbIntersection/nbTermsP1;
    }
    
    // calculating time similarity
    double timeSizeP1=(double)(p1.endTime-p1.beginTime+1);
    double timeSizeP2=(double)(p2.endTime-p2.beginTime+1);
    
    double timeIntersection=((double)(std::max(std::min(p1.endTime, p2.endTime) - std::max(p1.beginTime, p2.beginTime) + 1, 0)));
    double timeSim;
    if (timeSizeP1>timeSizeP2){
        timeSim=timeIntersection/timeSizeP2;
    }
    else {
        timeSim=timeIntersection/timeSizeP1;
    }
    return timeSim*simTerms;
}

double getSimilarityOfTwoPatterns2(Graph *graph,TempoPattern & p1Temp,TempoPattern & p2Temp,std::vector<Pattern> & retrievedPatterns){
    Pattern & trueP1=retrievedPatterns[p1Temp.fatherIndex];
    Pattern & trueP2=retrievedPatterns[p2Temp.fatherIndex];
    
    // calculating time similarity
    double timeSizeP1=(double)(trueP1.endTime-trueP1.beginTime+1);
    double timeSizeP2=(double)(trueP2.endTime-trueP2.beginTime+1);
    
    double timeIntersection=((double)(std::max(std::min(trueP1.endTime, trueP2.endTime) - std::max(trueP1.beginTime, trueP2.beginTime) + 1, 0)));
    double enumerator=trueP1.endTime-trueP1.beginTime+trueP2.endTime-trueP2.beginTime+2;
    if (timeIntersection==0){
        if (trueP1.beginTime>trueP2.endTime){
            enumerator=enumerator-(trueP1.beginTime-trueP2.endTime)+1;
        }
        else {
            enumerator=enumerator-(trueP2.beginTime-trueP1.endTime)+1;
        }
    }
    
    double timeSim=enumerator/(timeSizeP1+timeSizeP2);
    
    if (timeSim<MIN_SIM_PAT){
        return 0;
    }
    
    // calculating vertex similarities
    std::set<int> v1AndNeighbors;
    for (std::set<int>::iterator it=trueP1.vertexIndices.begin();it!=trueP1.vertexIndices.end();it++){
        v1AndNeighbors.insert(*it);
        v1AndNeighbors.insert(graph->getVertices()[*it].neighbors.begin(),graph->getVertices()[*it].neighbors.end());
    }
    double spaceSim=0;
    std::set<int> setIntersection;
    set_intersection(v1AndNeighbors.begin(), v1AndNeighbors.end(), trueP2.vertexIndices.begin(), trueP2.vertexIndices.end(),
                     std::inserter(setIntersection, setIntersection.begin()));
    if (setIntersection.size()>0){
        spaceSim=1;
    }
    //double interSpace=(double)setIntersection.size();
    //double unionSpace=((double)(p1.vertexIndices.size()+p2.vertexIndices.size()))-interSpace;
    //spaceSim=interSpace/unionSpace;
    // calculating terms similarity
    //double nbIntersection=0;
    //double nbTermsP1=(double)p1.hashtags.size();
    //double nbTermsP2=(double)p2.hashtags.size();
    //for (std::map<int, double>::iterator it = p1.hashtags.begin(); it != p1.hashtags.end(); it++) {
    //        if (p2.hashtags.find(it->first)!=p2.hashtags.end()){
    //            nbIntersection++;
    //       }
    //   }
    //   double simTerms=nbIntersection/(nbTermsP1+nbTermsP2-nbIntersection);
    
    double curScore=timeSim*spaceSim;
    if (curScore<MIN_SIM_PAT){
        return 0;
    }
    
    double totalW=p1Temp.totalScore+p2Temp.totalScore;
    double interW=0;
    for (std::unordered_map<int, double>::iterator it = p1Temp.hashtags.begin(); it != p1Temp.hashtags.end(); it++) {
        if (p2Temp.hashtags.find(it->first)!=p2Temp.hashtags.end()){
            interW+=it->second;
            interW+=p2Temp.hashtags[it->first];
        }
    }
    double simTerms=interW/totalW;
    
    
    
    return timeSim*simTerms*spaceSim;
}
