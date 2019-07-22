//
//  PrefBasedSorter.cpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 17/01/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//

#include "PrefBasedSorter.h"
#include <algorithm>

using namespace std;


void PrefBasedSorter::sortResults(std::vector<Pattern> & results){
    for (Pattern & pattern : results){
        // calculate hash based score
        double hashBasedScore=0;
        double totalWeightHash=0;
        for (std::map<int, double>::iterator it=pattern.hashtags.begin();it!=pattern.hashtags.end();it++){
            totalWeightHash+=1;
            hashBasedScore+=graph->getHashtagCooccurrences()->getSubjectiveQualities()[it->first];
        }
        hashBasedScore/=totalWeightHash;
        
        // calculate verticesBasedScore
        double verticesWeightScore=0;
        double totalWeightVertices=0;
        for (int vertexIndex : pattern.vertexIndices){
            totalWeightVertices+=1;
            verticesWeightScore+=graph->getVerticesSubQualitiesManager()->getSubjectiveQualities()[vertexIndex];
        }
        verticesWeightScore/=totalWeightVertices;
        pattern.prefScore=(hashBasedScore+verticesWeightScore)/2;
    }
    sort(results.begin(),results.end(),comparePatternsByPref);
}
