//
//  RelativeBasedMeasure2.cpp
//  PatternMiningBasedEventDetector
//
//  Created by Anes Bendimerad on 23/11/2016.
//  Copyright © 2016 Anes Bendimerad. All rights reserved.
//

#include "RelativeBasedMeasure.h"
#include <math.h>
#include <map>
#include <iostream>
using namespace std;
void RelativeBasedMeasure::calculateHashtagScores(){
    //endTime=(int)graph->getVertices()[0].periodicHashOcc.size();
    int numberOfTweets=0;
    vector<int> repetitions;
    graph->getHashtagsRepetition().clear();
    for (int i=0;i<graph->getHashtags().size();i++){
        graph->getHashtagsRepetition().push_back(0);
    }
    for (int i = 0; i < graph->getVertices().size(); i++) {
        graph->getVertices()[i].hashtagsTotalOccurrences.clear();
        graph->getVertices()[i].hashtagsStandardDev.clear();
        map<int,double> squaredXs;
        map<int,double> sumXs;
        for (int timeIndex = 0; timeIndex < endTime; timeIndex++) {
            numberOfTweets+=graph->getVertices()[i].periodicHashOcc[timeIndex].tweetIds.size();
            for (map<int, int>::iterator it = graph->getVertices()[i].periodicHashOcc[timeIndex].hashtagsOccurrences.begin(); it != graph->getVertices()[i].periodicHashOcc[timeIndex].hashtagsOccurrences.end(); it++) {
                 graph->getHashtagsRepetition()[it->first]+=it->second;
                if (graph->getVertices()[i].hashtagsTotalOccurrences.find(it->first)==graph->getVertices()[i].hashtagsTotalOccurrences.end()){
                    graph->getVertices()[i].hashtagsTotalOccurrences[it->first]=it->second;
                }
                else {
                    graph->getVertices()[i].hashtagsTotalOccurrences[it->first]=graph->getVertices()[i].hashtagsTotalOccurrences[it->first]+it->second;
                }
                double curOcc=it->second;
                if (squaredXs.find(it->first)==squaredXs.end()){
                    squaredXs[it->first]=(curOcc*curOcc);
                    sumXs[it->first]=curOcc;
                }
                else {
                    squaredXs[it->first]=squaredXs[it->first]+(curOcc*curOcc);
                    sumXs[it->first]=sumXs[it->first]+curOcc;
                }
                
            }
        }
        for(map<int,double>::iterator it=squaredXs.begin();
            it!=squaredXs.end();it++){
            double squaredX=it->second;
            double sumX=sumXs[it->first];
            double value=squaredX-sumX*sumX/((double)endTime);
            value/=((double)(endTime-1));
            graph->getVertices()[i].hashtagsStandardDev[it->first]=sqrt(value);
            
        }
    }
    double logNumberOfTweets=log((double)numberOfTweets);
    for (int i = 0; i < graph->getVertices().size(); i++) {
        for (int timeIndex = beginTime; timeIndex < endTime; timeIndex++) {
            graph->getVertices()[i].periodicHashOcc[timeIndex].hashScores.clear();
            graph->getVertices()[i].periodicHashOcc[timeIndex].positiveHashs.clear();
            for (map<int, int>::iterator it = graph->getVertices()[i].periodicHashOcc[timeIndex].hashtagsOccurrences.begin(); it != graph->getVertices()[i].periodicHashOcc[timeIndex].hashtagsOccurrences.end(); it++) {
                
                
                double score=(double(it->second))-((double)graph->getVertices()[i].hashtagsTotalOccurrences[it->first])/((double)endTime)-((double)graph->getVertices()[i].hashtagsStandardDev[it->first])*stDevWeightFactor;
                
                
                score=score *(log(((double)numberOfTweets)/((double)graph->getHashtagsRepetition()[it->first]))/logNumberOfTweets);
                
                //double totalNbOfTerm=graph->getHashtagsRepetition()[it->first];
                
                if (preferenceBased){
                    score=score*(graph->getHashtagCooccurrences()->getSubjectiveQualities()[it->first]+graph->getVerticesSubQualitiesManager()->getSubjectiveQualities()[i])/2;
                }
                //if (totalNbOfTerm<minFreqTerm){
                //    score=0;
                //}
                if (score > 0) {
                    graph->getVertices()[i].periodicHashOcc[timeIndex].hashScores[it->first] = score;
                    graph->getVertices()[i].periodicHashOcc[timeIndex].positiveHashs.insert(it->first);
                }
            }
        }
    }
    
    
}
