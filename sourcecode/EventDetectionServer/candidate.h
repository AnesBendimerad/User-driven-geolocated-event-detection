#pragma once
#include "graph.h"
#include <unordered_set>
typedef struct Candidate
{
	int index;
    std::map<int, double> validHashtags;
    std::unordered_set<int> validHashtagsSet;
} Candidate;
