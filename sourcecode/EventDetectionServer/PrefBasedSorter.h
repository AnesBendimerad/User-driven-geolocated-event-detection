//
//  PrefBasedSorter.hpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 17/01/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//

#ifndef PrefBasedSorter_hpp
#define PrefBasedSorter_hpp

#include <stdio.h>
#include "graph.h"
#include "pattern.h"

class PrefBasedSorter {

public:
    PrefBasedSorter(Graph * inGraph):graph(inGraph){};
    
    void sortResults(std::vector<Pattern> & results);

private :
    Graph * graph;
};



#endif /* PrefBasedSorter_hpp */
