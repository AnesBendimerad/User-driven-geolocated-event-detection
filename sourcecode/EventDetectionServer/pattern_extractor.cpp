//#include "stdafx.h"
#include "pattern_extractor.h"
#include <algorithm>
#include <iterator>
#include "covering_management.h"
#include <iostream>
#include "utils.hpp"
using namespace std;
vector<Pattern>& PatternExtractor::retrievePatterns(DesignPoint designPoint,Statistics &statistics)
{
	PatternExtractor::designPoint = designPoint;
    int maxTime=(int)graph->getVertices()[0].periodicHashOcc.size();
    statistics.nbExploredPatterns=0;
    if (designPoint.endMineTime>=0 && designPoint.endMineTime<=maxTime){
        maxTime=designPoint.endMineTime;
    }
	for (int beginTime = designPoint.beginMineTime; beginTime < (maxTime-designPoint.minTime+1); beginTime++) {
        if (showCouts){
            std::cout<<"beginTime:"<<beginTime<<std::endl;
        }
		bool continu = true;
		int endTime = beginTime + designPoint.minTime - 1;
		vector<Candidate> candidates;
        if (showCouts){
            std::cout<<"creating candidates"<<std::endl;
        }
		createCandidates(candidates, beginTime, endTime);
        if (showCouts){
            std::cout<<"exploring with end time"<<std::endl;
        }
		while (continu && endTime < graph->getVertices()[0].periodicHashOcc.size()) {
			if (!isUB1Bad(graph, candidates, designPoint.minQuality) && candidates.size() >= designPoint.minSize) {
				Pattern pattern;
				pattern.beginTime = beginTime;
				pattern.endTime = endTime;
				set<int> neighbors;
				variateSubgraphs(pattern, candidates, neighbors,statistics);
			}
			if (isUB0Bad(graph, candidates, endTime + 1, designPoint.minQuality) || candidates.size() < designPoint.minSize) {
				continu = false;
			}
			else {
				endTime++;
				if (endTime<graph->getVertices()[0].periodicHashOcc.size()){
					updateCandidates(candidates, endTime);
				}
			}
		}
	}
    statistics.nbFoundPatterns=(int)retrievedPatterns.size();
	return retrievedPatterns;
}


void PatternExtractor::variateSubgraphs(Pattern & pattern, vector<Candidate>& candidates, set<int> & neighbors,Statistics &statistics)
{
    
    
    statistics.nbExploredPatterns++;
    
    
    
    
    pruneCandidates(graph, pattern, candidates, designPoint.minQuality, neighbors);
	pruneWithConnectivity(graph, pattern, candidates, neighbors);
	if (isUB2Bad(graph, pattern, candidates, designPoint.minQuality) || (pattern.vertexIndices.size() + candidates.size()) < designPoint.minSize) {
		return;
	}
	if (isLBCovered(graph, pattern, candidates, retrievedPatterns, designPoint.minCov,designPoint.minSize,designPoint.minQuality)) {
		return;
	}
	vector<Candidate>::iterator it = candidates.begin();
	bool continu = true;
	if (pattern.vertexIndices.size() > 0) {
		while (continu && it != candidates.end()) {
			if (neighbors.find(it->index) != neighbors.end()) {
				continu = false;
			}
			else {
				it++;
			}
		}
	}
	else {
		continu = false;
	}
	if (!continu) {
		Pattern newPattern;
		newPattern.beginTime = pattern.beginTime;
		newPattern.endTime = pattern.endTime;
		newPattern.vertexIndices = pattern.vertexIndices;
		newPattern.vertexIndices.insert(it->index);
		if (pattern.vertexIndices.size()>0){
		for (map<int, double>::iterator itHash = pattern.hashtags.begin(); itHash != pattern.hashtags.end(); itHash++) {
			if (it->validHashtagsSet.find(itHash->first) != it->validHashtagsSet.end()) {
				newPattern.hashtags[itHash->first] = itHash->second + it->validHashtags[itHash->first];
			}
		}
		}
		else {
			newPattern.hashtags = it->validHashtags;
		}
		Candidate currentCand = *it;
		neighbors.erase(it->index);
		candidates.erase(it);
		vector<Candidate> newCandidates = candidates;
		vector<Candidate> newCandidates2 = candidates;
		set<int> newNeigh=neighbors;
		for (Candidate & cand : candidates) {
			if (graph->getVertices()[currentCand.index].neighbors.find(cand.index) != graph->getVertices()[currentCand.index].neighbors.end()) {
				newNeigh.insert(cand.index);
			}
		}
		Pattern newPattern2 = pattern;
		set<int> newNeigh2 = neighbors;
		variateSubgraphs(newPattern, newCandidates, newNeigh,statistics);
        
		variateSubgraphs(newPattern2, newCandidates2, newNeigh2,statistics);
        
	}
    else {
        if (getMeasure(pattern) >= designPoint.minQuality && !isPatternCovered(graph, pattern, retrievedPatterns, designPoint.minCov) && pattern.vertexIndices.size() >= designPoint.minSize) {
            retrievedPatterns.push_back(pattern);
        }
    }
	
}

void PatternExtractor::createCandidates(vector<Candidate>& candidates, int beginTime, int endTime)
{
	for (int i = 0; i < graph->getVertices().size(); i++) {
		Candidate candidate;
		candidate.index = i;
		set<int> validHashtags;
		validHashtags.insert(graph->getVertices()[i].periodicHashOcc[beginTime].positiveHashs.begin(), graph->getVertices()[i].periodicHashOcc[beginTime].positiveHashs.end());
		for (int j = beginTime + 1; j <= endTime; j++) {
			set<int> setIntersection;
			set_intersection(validHashtags.begin(), validHashtags.end(), graph->getVertices()[i].periodicHashOcc[j].positiveHashs.begin(), graph->getVertices()[i].periodicHashOcc[j].positiveHashs.end(),
				std::inserter(setIntersection, setIntersection.begin()));
			validHashtags = setIntersection;
		}
		if (validHashtags.size() > 0) {
			for (int hashtagIndex : validHashtags) {
				double totalScore = 0;
				for (int j = beginTime; j <= endTime; j++) {
					totalScore += graph->getVertices()[i].periodicHashOcc[j].hashScores[hashtagIndex];
				}
				candidate.validHashtags[hashtagIndex] = totalScore;
				candidate.validHashtagsSet.insert(hashtagIndex);
			}
			candidates.push_back(candidate);
		}
	}
}

void PatternExtractor::updateCandidates(vector<Candidate>& candidates, int newTime)
{
	vector<Candidate> newCandidates;
	for (Candidate & candidate : candidates) {
		map<int, double> newHashtagScores;
		unordered_set<int> newHashtagsSet;
		for (map<int, double>::iterator it = candidate.validHashtags.begin(); it != candidate.validHashtags.end(); it++) {
			if (graph->getVertices()[candidate.index].periodicHashOcc[newTime].positiveHashs.find(it->first) != graph->getVertices()[candidate.index].periodicHashOcc[newTime].positiveHashs.end()) {
				newHashtagsSet.insert(it->first);
				newHashtagScores[it->first] = it->second + graph->getVertices()[candidate.index].periodicHashOcc[newTime].hashScores[it->first];
			}
		}
		if (newHashtagsSet.size() > 0) {
			candidate.validHashtags = newHashtagScores;
			candidate.validHashtagsSet = newHashtagsSet;
			newCandidates.push_back(candidate);
		}
	}
	candidates = newCandidates;
}

double PatternExtractor::getMeasure(Pattern pattern)
{
	double measure=0;
	for (map<int, double>::iterator it = pattern.hashtags.begin(); it != pattern.hashtags.end(); it++) {
		measure += it->second;
	}
	return measure;
}
