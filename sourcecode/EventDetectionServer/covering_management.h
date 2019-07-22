#pragma once
#include "candidate.h"
#include "pattern.h"
bool isLBCovered(Graph *graph, Pattern & pattern, std::vector<Candidate> & candidates, std::vector<Pattern> & retrievedPatterns, double minCov, int minSize, double minQuality);
bool isSecondLBCoverFirst(Graph *graph, Pattern & pattern, std::vector<Candidate> & candidates, Pattern & retrievedPatterns, double minCov, int minSize, double minQuality);
bool isVerticesLBCovered(Pattern & pattern, Pattern & retPattern, std::vector<Candidate> & candidates,int minSize, double minCov);
bool isHashCov(Pattern & pattern, Pattern & retPattern, std::vector<Candidate> & candidates, double minCov, double minQuality);

bool isPatternCovered(Graph *graph, Pattern & pattern, std::vector<Pattern> & retrievedPatterns, double minCov);
bool isSecondCoverFirst(Pattern & pattern, Pattern & retPattern, double minCov);
