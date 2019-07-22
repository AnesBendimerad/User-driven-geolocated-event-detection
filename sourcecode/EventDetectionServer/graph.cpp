//#include "stdafx.h"
#include "graph.h"

Graph::~Graph(){
    if (hashtagCooccurrencesGraph!=nullptr)
    {
        delete hashtagCooccurrencesGraph;
    }
    if (verticesSubQualitiesManager!=nullptr){
        delete verticesSubQualitiesManager;
    }
}

void Graph::inactivatePreferences(){
    hashtagCooccurrencesGraph=nullptr;
    verticesSubQualitiesManager=nullptr;
}

Graph * Graph::cloneGraph(){
    Graph * cloned = new Graph(cycle,numberOfUsedTweets,totalHashtagOccurrences);
    //cloned->setTweets(tweets);
    for (std::map<std::string,Tweet>::iterator it=tweets.begin();it!=tweets.end();it++){
        Tweet tweet=it->second;
        cloned->getTweets()[it->first]=tweet;
    }
    
    //cloned->setVerticesIds(verticesIds);
    cloned->verticesIds=verticesIds;
    
    //cloned->setHashtagsMap(hashtagsMap);
    cloned->hashtagsMap=hashtagsMap;
    
    
    cloned->getHashtagsRepetition().clear();
    cloned->areTweetsLoaded=areTweetsLoaded;
    
    //cloned->setHashtags(hashtags);
    cloned->hashtags=hashtags;
    
    
    for (Vertex & vertex : vertices){
        Vertex newV;
        newV.identifier=vertex.identifier;
        newV.neighbors=vertex.neighbors;
        //newV.avgOccurrences=vertex.avgOccurrences;
        //newV.hashtagsTotalOccurrences=vertex.hashtagsTotalOccurrences;
        //newV.hashtagsStandardDev=vertex.hashtagsStandardDev;
        for (TimeInterval & interval : vertex.periodicHashOcc){
            TimeInterval newInterval;
            newInterval.hashScores=interval.hashScores;
            newInterval.hashtagsOccurrences=interval.hashtagsOccurrences;
            newInterval.positiveHashs=interval.positiveHashs;
            newInterval.tweetIds=interval.tweetIds;
            newV.periodicHashOcc.push_back(newInterval);
        }
        cloned->getVertices().push_back(newV);
    }
    // hashtagsubjective Qualities
    std::vector<std::unordered_map<int,std::unordered_map<int,int> > > hashtagCooccurrencesPerTime=hashtagCooccurrencesGraph->hashtagCooccurrencesPerTime;
    HashSubjQualitiesManager * hashManager=new HashSubjQualitiesManager(hashtagCooccurrencesPerTime,hashtagCooccurrencesGraph->getNumberOfAllHashtags(),hashtagCooccurrencesGraph->hashCoOccInverted);
    cloned->setHashtagCooccurrencesGraph(hashManager);
    // verticesQualities
    cloned->setVerticesSubQualitiesManager(new VerticesSubQualitiesManager(cloned));
    
    return cloned;
}
