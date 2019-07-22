#pragma once
#include "graph.h"
#include <unordered_map>
#include "pattern_extractor.h"
class GraphLoader {
public:
	GraphLoader(DesignPoint * point,bool preferenceBased) :designPoint(point), preferenceBased(preferenceBased) {}
	Graph * load();
private:
    DesignPoint *designPoint;
    bool preferenceBased;
    int getHashtagId(std::string hashtag, std::unordered_map<std::string, int>& hashtagIds, int & currentNumberOfHashtags, std::vector<std::string> & hashtagIdsVector);
};
