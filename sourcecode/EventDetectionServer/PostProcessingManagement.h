//
//  PostProcessingManagement.hpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 30/08/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//

#ifndef PostProcessingManagement_hpp
#define PostProcessingManagement_hpp

#include <stdio.h>

#include "pattern_extractor.h"

#define MIN_WEIGHT_COOCC 1
#define MAX_PAT_CLUST_PER_IT 300
#define MIN_SIM_PAT 0.2

typedef struct TempoPattern{
    int fatherIndex;
    double totalScore;
    std::unordered_map<int, double> hashtags;
} TempoPattern;



void postProcess(Graph *graph,std::vector<Pattern> & retrievedPatterns,Statistics & statistics,DesignPoint designPoint,std::vector<Pattern> &finalPatterns);

double getSimilarityOfTwoPatterns(Pattern & p1,Pattern &  p2);
double getSimilarityOfTwoPatterns2(Graph *graph,TempoPattern & p1,TempoPattern & p2,std::vector<Pattern> & retrievedPatterns);





#endif /* PostProcessingManagement_hpp */


