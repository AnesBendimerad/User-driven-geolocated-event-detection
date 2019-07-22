//
//  RelativeBasedMeasure2.hpp
//  PatternMiningBasedEventDetector
//
//  Created by Anes Bendimerad on 23/11/2016.
//  Copyright © 2016 Anes Bendimerad. All rights reserved.
//

#ifndef RelativeBasedMeasure2_hpp
#define RelativeBasedMeasure2_hpp

#include <stdio.h>

#include "graph.h"
//#define minFreqTerm 1
class RelativeBasedMeasure {
public:
    RelativeBasedMeasure(Graph * graph,double stDevWeightFactor,int beginTime,int endTime,bool preferenceBased):graph(graph),stDevWeightFactor(stDevWeightFactor),beginTime(beginTime),endTime(endTime),preferenceBased(preferenceBased){};
    void calculateHashtagScores();
    private :
    int beginTime;
    int endTime;
    bool preferenceBased;
    Graph * graph;
    double stDevWeightFactor;
};

#endif /* RelativeBasedMeasure2_hpp */
