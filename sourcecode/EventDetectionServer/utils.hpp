//
//  utils.hpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 02/01/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//

#ifndef utils_hpp
#define utils_hpp

#define showCouts false

#include <stdio.h>
#include "pattern_extractor.h"
#include <time.h>
#include <sys/timeb.h>
double getTimeComputationInMS(struct timeb &start, struct timeb &end);
double getTimeComputationInS(struct timeb &start, struct timeb &end);
void readParametersSimpleDetection(std::string paramFilePath,DesignPoint & designPoint);
void readParametersLaunchPreferenceBased(std::string paramFilePath,DesignPoint & designPoint);
void readParametersContinuePreferenceBased(std::string paramFilePath,DesignPoint & designPoint);
void readParametersAddNewLikes(std::string paramFilePath,DesignPoint & designPoint);
void setToDefaultValues(DesignPoint & designPoint);
#endif /* utils_hpp */
