//#include "stdafx.h"
#include "pruning_management.h"
#include <algorithm>
#include <iterator>
using namespace std;
int isUB0Bad(Graph * graph, vector<Candidate> & candidates, int nextTimeIndex, double minQuality)
{
	for (Candidate & candidate : candidates) {
		double currentScore = 0;
		for (map<int, double>::iterator it = candidate.validHashtags.begin(); it != candidate.validHashtags.end(); it++) {
			for (Candidate & candidate2 : candidates) {
				if (candidate2.validHashtags.find(it->first) != candidate2.validHashtags.end()) {
					currentScore += candidate2.validHashtags[it->first];
				}
				for (int i = nextTimeIndex; i < graph->getVertices()[0].periodicHashOcc.size(); i++) {
					if (graph->getVertices()[candidate2.index].periodicHashOcc[i].hashScores.find(it->first) != graph->getVertices()[candidate2.index].periodicHashOcc[i].hashScores.end()) {
						currentScore += graph->getVertices()[candidate2.index].periodicHashOcc[i].hashScores[it->first];
					}
				}
			}
		}
		if (currentScore >= minQuality) {
			return false;
		}
	}
	return true;
}

int isUB1Bad(Graph * graph, vector<Candidate> & candidates, double minQuality)
{
	for (Candidate & candidate : candidates) {
		double currentScore = 0;
		for (map<int, double>::iterator it = candidate.validHashtags.begin(); it != candidate.validHashtags.end(); it++) {
			for (Candidate & candidate : candidates) {
				if (candidate.validHashtags.find(it->first) != candidate.validHashtags.end()) {
					currentScore += candidate.validHashtags[it->first];
				}
			}
		}
		if (currentScore >= minQuality) {
			return false;
		}
	}
	return true;
}
int isUB2Bad(Graph * graph, Pattern & pattern, vector<Candidate> & candidates, double minQuality)
{
	if (pattern.vertexIndices.size() == 0) {
		return false;
	}
	double score = 0;
	for (map<int, double>::iterator it = pattern.hashtags.begin(); it != pattern.hashtags.end(); it++) {
		score += it->second;
		for (Candidate & candidate : candidates) {
			if (candidate.validHashtagsSet.find(it->first) != candidate.validHashtagsSet.end()) {
				score += candidate.validHashtags[it->first];
			}
		}
		if (score >= minQuality) {
			return false;
		}
	}
	return true;
}

void pruneCandidates(Graph * graph, Pattern & pattern, vector<Candidate>& candidates, double minQuality, set<int> & neighbors)
{
	if (pattern.vertexIndices.size() == 0) {
		return;
	}
	bool continu = true;

	while (continu) {
		continu = false;
		vector<Candidate>::iterator itCand = candidates.begin();
		while (itCand !=candidates.end()) {
			double score = 0;
			for (map<int, double>::iterator it = pattern.hashtags.begin(); it != pattern.hashtags.end(); it++) {
				if (itCand->validHashtagsSet.find(it->first) != itCand->validHashtagsSet.end()) {
					score += it->second + itCand->validHashtags[it->first];
					for (Candidate & candidate2 : candidates) {
						if (candidate2.validHashtagsSet.find(it->first) != candidate2.validHashtagsSet.end()) {
							score += candidate2.validHashtags[it->first];
						}
					}
				}
				if (score >= minQuality) {
					break;
				}
			}
			if (score < minQuality) {
				continu = true;
				neighbors.erase(itCand->index);
				itCand=candidates.erase(itCand);
			}
			else {
				itCand++;
			}
		}		
	}
}

void pruneWithConnectivity(Graph *graph, Pattern & pattern, vector<Candidate> & candidates, set<int> & myNeighbors)
{
	if (pattern.vertexIndices.size() == 0) {
		return;
	}
	set<int> rest;
	for (Candidate & candidate : candidates) {
		rest.insert(candidate.index);
	}
	set<int> notExploredYet;
	notExploredYet = pattern.vertexIndices;
	while (notExploredYet.size() > 0 && rest.size()>0) {
		set<int> neighbors;
		for (int vIndex : notExploredYet) {
			neighbors.insert(graph->getVertices()[vIndex].neighbors.begin(), graph->getVertices()[vIndex].neighbors.end());
		}
		set<int> connected;
		set_intersection(neighbors.begin(), neighbors.end(), rest.begin(),rest.end() ,
			std::inserter(connected, connected.begin()));
		notExploredYet = connected;
		set<int> result;
		std::set_difference(rest.begin(), rest.end(), connected.begin(), connected.end(),
			std::inserter(result, result.end()));
		rest = result;
	}
	if (rest.size()>0){
	vector<Candidate>::iterator it = candidates.begin();
	while(it!=candidates.end()) {
		if (rest.find(it->index) != rest.end()) {
			myNeighbors.erase(it->index);
			it = candidates.erase(it);
		}
		else {
			it++;
		}
	}
	}
}
