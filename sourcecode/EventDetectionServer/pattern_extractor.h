#pragma once
#include "graph.h"
#include "pruning_management.h"



typedef struct DesignPoint
{
    // 1. thresholds of patterns qualities and dimensions
    double minQuality;
    double minSize;
    double minTime;
    double minCov;
    double stDevWeightFactor;
    int beginMineTime;
    int endMineTime;
    bool modelIsAll;
    bool prefBased;
    bool clusterize;
    
    // 2. main files
    std::string graphFilePath;
    std::string outputFilePath;
    std::string tweetsFilePath;
    bool writeTweetsInDetails;
    bool filterBySizes;
    int userId;
    
    // 3. parameters for preference based
    std::vector<int> chosenPatterns;
    std::string fileContainingLikedPatterns;
    
    // 3.1. hashtag bias files
    std::string hashtagCooccurrencesFilePath;
    
    
    // 3.2. parameters of propagation attenuation
    double alpha;
    double subjectivePower;
    
    // 3.3. vertices bias files
    
    
    // 4. parameters for sampling approach
    double samplingTimeInMS;
    std::string approach;
    std::string postProcessing;
} DesignPoint;


class PatternExtractor {
public:
    PatternExtractor(Graph * graph) :graph(graph) {};
    std::vector<Pattern> & retrievePatterns(DesignPoint designPoint,Statistics &statistics);
    std::vector<Pattern> & getRetrievedPatterns() { return retrievedPatterns; }
private:
    Graph * graph;
    DesignPoint designPoint;
    std::vector<Pattern> retrievedPatterns;
    void variateSubgraphs(Pattern & pattern, std::vector<Candidate> & candidates,std::set<int> & neighbors,Statistics &statistics);
    void createCandidates(std::vector<Candidate> & candidates, int beginTime,int endTime);
    void updateCandidates(std::vector<Candidate> & candidates, int newTime);
    double getMeasure(Pattern pattern);
};
