//
//  pattern.cpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 03/01/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//

#include <stdio.h>
#include "pattern.h"


bool equals(Pattern & p1,Pattern & p2){
    if (p1.beginTime!=p2.beginTime){
        return false;
    }
    if (p1.endTime!=p2.endTime){
        return false;
    }
    if (p1.vertexIndices!=p2.vertexIndices){
        return false;
    }
    // then, hashtags equality is implied
    return true;
    
    
}

bool comparePatterns (Pattern & i,Pattern & j) {
    return (i.totalScore>j.totalScore);
}

bool comparePatternsByPref (Pattern & i,Pattern & j) {
    return (i.prefScore>j.prefScore);
}
