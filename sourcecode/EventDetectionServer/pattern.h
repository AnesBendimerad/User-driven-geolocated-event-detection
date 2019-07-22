
#ifndef Pattern_H
#define Pattern_H

#include <set>
#include <map>


typedef struct Pattern
{
    std::set<int> vertexIndices;
	int beginTime;
	int endTime;
	std::map<int, double> hashtags;
    double totalScore;
    double prefScore;

} Pattern;

bool equals(Pattern & p1,Pattern & p2);
bool comparePatterns (Pattern & i,Pattern & j) ;
bool comparePatternsByPref (Pattern & i,Pattern & j) ;

#endif


