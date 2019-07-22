//
//  VerticesSubQualitiesManager.hpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 03/01/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//

#ifndef VerticesSubQualitiesManager_hpp
#define VerticesSubQualitiesManager_hpp
#include <stdio.h>
#include <vector>
#include <string>
#include <set>
#define DEFAULT_VERTICES_ALPHA 0.6

typedef struct Graph;
class VerticesSubQualitiesManager {
public:
    VerticesSubQualitiesManager(Graph * graph);
    
    
    void loadVerticesLastEventsIndices(std::string verticesLastEventsIndicesFile);
    void saveVerticesLastEventsIndices(std::string verticesLastEventsIndicesFile);
    void saveQualitiesOfVertices(std::string qualitiesOfVerticesFile);
    void updateVerticesLastEventsIndices(std::set<int> vertexIndices);
    void calculateSubjectiveQualities(double alpha,double subjectivePower);
    std::vector<double> & getSubjectiveQualities(){return subjectiveQualities;}
    void initVerticesLastEventIndices();

private:
    Graph * graph;
    std::vector<int> verticesLastEventIndices;
    std::vector<double> subjectiveQualities;
    double totalEventNumber;
    double alpha;
    double subjectivePower;
    
};

#endif /* VerticesSubQualitiesManager_hpp */
