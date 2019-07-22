#pragma once
#include "pattern_extractor.h"
void writeResult(Graph *graph,std::vector<Pattern> & retrievedPatterns,std::string outputFilePath,bool orderResult,Statistics & statistics,DesignPoint &designPoint);
bool sortHashtags(std::pair<int, double>  first, std::pair<int, double>  second);
void loadPreviousResult(std::string inputFilePath,std::vector<Pattern> & previousPatterns,std::unordered_map<std::string, int> hashtagIds,std::unordered_map<std::string, int> verticesIds);

void writePostProcessedResult(Graph *graph,std::vector<Pattern> & retrievedPatterns,std::string outputFilePath,bool orderResult,Statistics & statistics,DesignPoint &designPoint);
