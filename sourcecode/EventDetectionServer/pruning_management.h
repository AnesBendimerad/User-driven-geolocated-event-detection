#pragma once
#include "candidate.h"
#include "pattern.h"

int isUB0Bad(Graph * graph,std::vector<Candidate> & candidates, int nextTimeIndex,double minQuality);
int isUB1Bad(Graph * graph, std::vector<Candidate> & candidates, double minQuality);
int isUB2Bad(Graph * graph, Pattern & pattern, std::vector<Candidate> & candidates, double minQuality);
void pruneCandidates(Graph *graph, Pattern & pattern, std::vector<Candidate> & candidates, double minQuality, std::set<int> & neighbors);
void pruneWithConnectivity(Graph *graph, Pattern & pattern, std::vector<Candidate> & candidates, std::set<int> & neighbors);
