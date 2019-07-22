//
//  HashSubjQualitiesManager.hpp
//  PatternMiningBasedEventDetector
//
//  Created by Anes Bendimerad on 19/12/2016.
//  Copyright © 2016 Anes Bendimerad. All rights reserved.
//

#ifndef HashSubjQualitiesManager_hpp
#define HashSubjQualitiesManager_hpp

#include <stdio.h>
#include <map>
#include <vector>
#include <unordered_map>
#define DEFAULT_ALPHA 0.85


typedef struct CoOccPerTime
{
    int timeIndex;
    int coOcc;
} CoOccPerTime;

class HashSubjQualitiesManager {
public:
    HashSubjQualitiesManager(std::vector<std::unordered_map<int,std::unordered_map<int,int> > > & hashtagCooccurrencesPerTime,int numberOfAllHashtags,std::vector<std::unordered_map<int,std::vector<CoOccPerTime> > > & hashCoOccInverted):hashtagCooccurrencesPerTime(hashtagCooccurrencesPerTime),numberOfAllHashtags(numberOfAllHashtags),totalEventNumber(0),hashCoOccInverted(hashCoOccInverted){};

    
    void loadHashtagsLastEventsIndices(std::string importanceOfHashtagsFile,std::unordered_map<std::string,int> & hashtagsMap);

    void saveHashtagsLastEventsIndices(std::string importanceOfHashtagsFile,std::vector<std::string> & hashtagsAsString);
    
    void saveQualitiesOfHashtags(std::string importanceOfHashtagsFile,std::vector<std::string> & hashtagsAsString);
    
    void updateHashtagsLastEventsIndices(std::map<int, double> & hashtags);
    
    void setValuesForBounds(int beginTime,int endTime);
    
    void calculateSubjectiveQualities(double alpha,double subjectivePower);
    void initHashtagsLastEventIndices();
    std::vector<double> & getSubjectiveQualities(){return subjectiveQualities;}
    
    int getNumberOfCoOccurrences(int termId1,int termId2, int beginTime,int endTime);
    bool isNbCoOcHigherThanThres(int termId1,int termId2, int beginTime,int endTime,int thres);
    
    std::vector<std::unordered_map<int,std::unordered_map<int,int> > > &  getHashtagCooccurrencesPerTime(){return hashtagCooccurrencesPerTime;}
    int getNumberOfAllHashtags(){return numberOfAllHashtags;}
    
    std::vector<std::unordered_map<int,std::unordered_map<int,int> > > hashtagCooccurrencesPerTime;
    
    
    std::vector<std::unordered_map<int,std::vector<CoOccPerTime> > > hashCoOccInverted;
private:
    
    
    std::unordered_map<int,std::unordered_map<int,int> > hashtagCooccurrences;
    std::map<int,int> hashtagsTotalDegree;
    
    std::map<int,double> hashtagsLastEventIndices;
    std::vector<double> subjectiveQualities;
    double totalEventNumber;
    double alpha;
    double subjectivePower;
    int numberOfAllHashtags;
    
};

bool compareCoOcc (CoOccPerTime i,CoOccPerTime j);

#endif /* HashSubjQualitiesManager_hpp */
