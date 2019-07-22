//#include "stdafx.h"
#include "covering_management.h"
#include <algorithm>
#include <iterator>
#include <math.h>
#include <iostream>
using namespace std;
bool isLBCovered(Graph * graph, Pattern & pattern, vector<Candidate>& candidates, vector<Pattern>& retrievedPatterns, double minCov, int minSize, double minQuality)
{
	if (pattern.vertexIndices.size() == 0) {
		return false;
	}
	for (Pattern & retPattern : retrievedPatterns) {
		if (isSecondLBCoverFirst(graph,pattern, candidates,retPattern, minCov, minSize, minQuality)) {
			return true;
		}
	}
	return false;
}

bool isSecondLBCoverFirst(Graph * graph, Pattern & pattern, vector<Candidate>& candidates, Pattern & retPattern, double minCov,int minSize,double minQuality)
{
	double timeIntersect = ((double)(max(min(pattern.endTime, retPattern.endTime) - max(pattern.beginTime, retPattern.beginTime) + 1, 0))) / ((double)(pattern.endTime - pattern.beginTime + 1));
	if (timeIntersect < minCov) {
		return false;
	}
	if (!isVerticesLBCovered(pattern, retPattern, candidates, minSize, minCov)) {
		return false;
	}
	if (!isHashCov(pattern, retPattern, candidates, minCov, minQuality)) {
		return false;
	}
	return true;
}
bool isVerticesLBCovered(Pattern & pattern, Pattern & retPattern, vector<Candidate> & candidates,int minSize,double minCov) {
	set<int> sIntersection;
    
	set_intersection(pattern.vertexIndices.begin(), pattern.vertexIndices.end(), retPattern.vertexIndices.begin(), retPattern.vertexIndices.end(),
		std::inserter(sIntersection, sIntersection.begin()));
	int candMinusRetPS = 0;
	for (Candidate & candidate : candidates) {
		if (retPattern.vertexIndices.find(candidate.index) == retPattern.vertexIndices.end()) {
			candMinusRetPS++;
		}
	}
    
    double numerator;
    double denominator;
    if (minSize>pattern.vertexIndices.size() + candMinusRetPS){
         numerator=(double)(sIntersection.size());
        int patternSize=(int)(pattern.vertexIndices.size());
        numerator+= (double)(minSize - patternSize - candMinusRetPS);
         denominator=(double)(pattern.vertexIndices.size());
        denominator+=(double)(minSize - patternSize);
        
    }
    else {
         numerator=(double)(sIntersection.size());
         denominator=(double)(pattern.vertexIndices.size());
        denominator+=(double)(candMinusRetPS);
    }
    return numerator/denominator;
    
	
}

bool isHashCov(Pattern & pattern, Pattern & retPattern, vector<Candidate> & candidates, double minCov,double minQuality) {
	int setminusSize = 0;
	double maxScore = 0;
	vector<double> intersectionScores;
	for (map<int, double>::iterator it = pattern.hashtags.begin(); it != pattern.hashtags.end(); it++) {
		if (retPattern.hashtags.find(it->first) == retPattern.hashtags.end()) {
			setminusSize++;
			maxScore += it->second;
			for (Candidate & candidate : candidates) {
				if (candidate.validHashtagsSet.find(it->first) != candidate.validHashtagsSet.end()) {
					maxScore += candidate.validHashtags[it->first];
				}
			}
		}
		else {
			double intersectionScore= it->second;
			for (Candidate & candidate : candidates) {
				if (candidate.validHashtagsSet.find(it->first) != candidate.validHashtagsSet.end()) {
					intersectionScore += candidate.validHashtags[it->first];
				}
			}
			intersectionScores.push_back(intersectionScore);
		}
	}
	sort(intersectionScores.begin(), intersectionScores.end());
	if (setminusSize == 0) {
		if (pattern.hashtags.size() > 0) {
			return true;
		}
		else {
			return false;
		}
	}
	int bestSize = (int)ceil((minCov/(1-minCov))*((double)setminusSize)-1);
	if (bestSize > intersectionScores.size()) {
		bestSize = (int)intersectionScores.size();
	}
	int nbTaken = 0;
	if (bestSize>0){
		vector<double>::iterator it = intersectionScores.end();
		while (nbTaken < bestSize) {
			it--;
			maxScore += (*it);
			nbTaken++;
		}
	}
	return (maxScore<minQuality);
}

bool isPatternCovered(Graph * graph, Pattern & pattern, vector<Pattern>& retrievedPatterns, double minCov)
{
	for (Pattern & retPattern : retrievedPatterns) {
		if (isSecondCoverFirst(pattern, retPattern, minCov)) {
			return true;
		}
	}
	return false;
}

bool isSecondCoverFirst(Pattern & pattern, Pattern & retPattern, double minCov)
{
	double timeIntersect = ((double)(max(min(pattern.endTime, retPattern.endTime) - max(pattern.beginTime, retPattern.beginTime) + 1, 0))) / ((double)(pattern.endTime - pattern.beginTime + 1));
	if (timeIntersect < minCov) {
		return false;
	}
	set<int> vertexIntersection;
	set_intersection(pattern.vertexIndices.begin(), pattern.vertexIndices.end(), retPattern.vertexIndices.begin(), retPattern.vertexIndices.end(),
		std::inserter(vertexIntersection, vertexIntersection.begin()));
	if (((double(vertexIntersection.size())) / ((double)pattern.vertexIndices.size())) < minCov) {
		return false;
	}
	int hashIntersectionSize = 0;
	for (map<int, double>::iterator it = pattern.hashtags.begin(); it != pattern.hashtags.end(); it++) {
		if (retPattern.hashtags.find(it->first) != retPattern.hashtags.end()) {
			hashIntersectionSize++;
		}
	}
	if ((((double)hashIntersectionSize) / ((double)pattern.hashtags.size())) < minCov) {
		return false;
	}
	return true;
}
