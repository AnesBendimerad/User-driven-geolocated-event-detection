//
//  pattern_sampler.hpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 03/01/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//

#ifndef pattern_sampler_hpp
#define pattern_sampler_hpp

#include <stdio.h>
#include "pattern_extractor.h"

class PatternSampler {
public:
    PatternSampler(Graph * graph):graph(graph),useMinQuality(false){};
    std::vector<Pattern> & samplePatterns(DesignPoint designPoint);
    std::vector<Pattern> & getSampledPatterns() { return sampledPatterns; }
    void setMinQuality(double minQuality1){minQuality=minQuality1;}
    void setUseMinQuality(bool use1){useMinQuality=use1;}
    void removeRepetition();
    void removeRedundancy();
    ~ PatternSampler();
    
    
private:
    DesignPoint designPoint;
    Graph * graph;
    std::vector<Pattern> sampledPatterns;
    std::vector<std::set<int>> validHashtagsPerProbaTableEl;
    double * initProbaTable;
    int maxProbaTable;
    void initializeInitProbaTable();
    
    void sampleOnePattern();
    bool samplePatternByExtention(Pattern & currentPattern);
    double minQuality;
    bool useMinQuality;
};
#endif /* pattern_sampler_hpp */
