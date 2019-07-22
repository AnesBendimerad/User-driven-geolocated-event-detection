//
//  VerticesSubQualitiesManager.cpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 03/01/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//

#include "VerticesSubQualitiesManager.h"
#include <fstream>
#include <iostream>
#include "string_management.h"
#include "jacobi.hpp"
#include "graph.h"
#include <math.h>
using namespace std;
VerticesSubQualitiesManager::VerticesSubQualitiesManager(Graph * graph){
    VerticesSubQualitiesManager::graph=graph;
    totalEventNumber=0;
    for (int i=0;i<graph->getVertices().size();i++){
        verticesLastEventIndices.push_back(-1);
    }
}
void VerticesSubQualitiesManager::initVerticesLastEventIndices(){
    verticesLastEventIndices.clear();
    totalEventNumber=0;
    for (int i=0;i<graph->getVertices().size();i++){
        verticesLastEventIndices.push_back(-1);
    }
}
void VerticesSubQualitiesManager::loadVerticesLastEventsIndices(std::string verticesLastEventsIndicesFilePath){
    std::ifstream verticesLastEventsIndicesFile(verticesLastEventsIndicesFilePath);
    if (!verticesLastEventsIndicesFile.is_open()){
        std::cout<<"cannot open file"<<verticesLastEventsIndicesFilePath<<std::endl;
        throw "cannot open file";
    }
    for (int i=0;i<verticesLastEventIndices.size();i++){
        verticesLastEventIndices[i]=-1;
    }
    string line;
    int cpt=0;
    while (std::getline(verticesLastEventsIndicesFile, line))
    {
        if (line.at(line.length() - 1) == '\r' || line.at(line.length() - 1) == '\n') {
            line = line.substr(0, line.length() - 1);
        }
        vector<string> elements=split(line,':');
        if (cpt==0){
            totalEventNumber=stod(elements[1].c_str());
        }
        else {
            verticesLastEventIndices[graph->getVerticesIds()[elements[0]]]=stod(elements[1].c_str());
        }
        cpt++;
    }
    verticesLastEventsIndicesFile.close();
}

void VerticesSubQualitiesManager::saveQualitiesOfVertices(std::string qualitiesOfVerticesFilePath){
    ofstream qualitiesOfVerticesFile(qualitiesOfVerticesFilePath);
    for (int i=0;i<subjectiveQualities.size();i++){
        qualitiesOfVerticesFile<< graph->getVertices()[i].identifier<<":"<<to_string(subjectiveQualities[i])<<endl;
    }
    qualitiesOfVerticesFile.close();
}

void VerticesSubQualitiesManager::saveVerticesLastEventsIndices(std::string verticesLastEventsIndicesFilePath){
    ofstream verticesLastEventsIndicesFile(verticesLastEventsIndicesFilePath);
    verticesLastEventsIndicesFile<<"totalEventNumber:"<<to_string(totalEventNumber)<<endl;
    for (int i=0;i<verticesLastEventIndices.size();i++){
        verticesLastEventsIndicesFile<<graph->getVertices()[i].identifier<<":"<<to_string(verticesLastEventIndices[i])<<endl;
    }
    verticesLastEventsIndicesFile.close();
}

void VerticesSubQualitiesManager::updateVerticesLastEventsIndices(std::set<int> vertexIndices){
    totalEventNumber++;
    for (int vertexIndex : vertexIndices){
        verticesLastEventIndices[vertexIndex]=totalEventNumber;
    }
}

void VerticesSubQualitiesManager::calculateSubjectiveQualities(double alpha,double subjectivePower){
    VerticesSubQualitiesManager::alpha=alpha;
    VerticesSubQualitiesManager::subjectivePower=subjectivePower;
    vector<map<int,double>> aMatrix;
    subjectiveQualities.clear();
    map<int,double> bMatrix;
    for (int i=0;i<graph->getVertices().size();i++){
        map<int,double> aMatrixI;
        Vertex & vertex=graph->getVertices()[i];
        if (vertex.neighbors.size()>0){
            for (int neighId: vertex.neighbors){
                aMatrixI[neighId]=alpha/((double)vertex.neighbors.size());
            }
            aMatrixI[i]=-1;
        }
        else {
            aMatrixI[i]=(alpha-1);
        }
        aMatrix.push_back(aMatrixI);
        if (totalEventNumber>0){
            if (verticesLastEventIndices[i]<1){
                bMatrix[i]=alpha-1;
            }
            else {
                bMatrix[i]=(alpha-1)*(1+subjectivePower/(1+log(totalEventNumber-verticesLastEventIndices[i]+1)));
            }
        }
        else {
            bMatrix[i]=alpha-1;
        }
    }
    solveEquation(aMatrix, bMatrix, subjectiveQualities);
}




