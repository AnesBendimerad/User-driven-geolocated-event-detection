//
//  Server.cpp
//  Server2
//
//  Created by Anes Bendimerad on 30/12/2016.
//  Copyright © 2016 Anes Bendimerad. All rights reserved.
//

#include "Server.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <unistd.h>


Server::Server()
{
    //openedGraph=nullptr;
    mainGraph=nullptr;
    firstGraph=true;
}

std::string Server::readCommandFromSocket(int *csock){
    char buffer[1024];
    int buffer_len = 1024;
    int bytecount;
    try
    {
        memset(buffer, 0, buffer_len);
        if((bytecount = recv(*csock, buffer, buffer_len, 0))== -1){
            return "";
        }
        //std::this_thread::sleep_for(std::chrono::seconds(200));
        std::string command=buffer;
        return command;
    }
    catch (int e)
    {
        //cout << "An exception occurred. Exception Nr. " << e << '\n';
        return "";
    }
}


int Server::socketHandler(int* csock,std::string command){
    bool success=false;
    int bytecount;
    success=executeCommand(command);
    
    //strcat(buffer, " SERVER ECHO");
    std::string respond;
    if (success){
        respond="execution terminated successfully\n";
    }
    else {
        respond="execution terminated with error\n";
    }
    try
    {
        if((bytecount = send(*csock, respond.data(),respond.size(), 0))== -1){
            fprintf(stderr, "Error sending data %d\n", errno);
            close(*csock);
            free(csock);
            return -1;
        }
        if (showCouts){
            printf("Sent bytes %d\n", bytecount);
        }
    } catch (int e)
    {
        if (showCouts){
            std::cout<<"exception when trying to answer to socket"<<std::endl;
        }
        return -1;
    }
    try
    {
        close(*csock);
        free(csock);
    } catch (int e)
    {
        if (showCouts){
            std::cout<<"exception when trying to free socket"<<std::endl;
        }
        return -1;
    }
    return 0;
}


bool Server::executeCommand(std::string command){
    
    if (command.at(command.length() - 1) == '\r' || command.at(command.length() - 1) == '\n') {
        command = command.substr(0, command.length() - 1);
    }
    
    if (command.compare("alive")==0){
        return true;
    }
    if (command.compare("stop")==0){
        stop();
        return true;
    }
    
    std::vector<std::string> elements=split(command, ' ');
    if (elements[0].compare("objectiveEventDetection")==0){
        //delete openedGraph;
        //openedGraph=nullptr;
        std::string command=elements[1];
        doSimpleEventDetection(command);
    }
    
    else if (elements[0].compare("launchSubjectiveDetection")==0){
        std::string command=elements[1];
        launchPreferenceBased(command);
    }
    else if (elements[0].compare("loadMainGraph")==0){
        std::string command=elements[1];
        loadMainGraph(command);
    }
    else if (elements[0].compare("continueSubjectiveDetection")==0){
        std::string command=elements[1];
        return continuePreferenceBased(command);
    }
    else if (elements[0].compare("initSubjectiveQualities")==0){
        try
        {
            int userId=stoi(elements[1]);
            return initSubjectiveQualities(userId);
        }catch (int e){
            return false;
        }
        
    }
    else if (elements[0].compare("addNewLikes")==0){
        std::string command=elements[1];
        addNewLikes(command);
    }
    else if (elements[0].compare("closeUser")==0){
        try
        {
            int userId=stoi(elements[1]);
            closeUser(userId);
        }catch (int e){
            return false;
        }
    }
    else {
        // command not found
        return false;
    }
    
    return true;
}




bool Server::closeUser(int userId){
    mtxMapOfUsers.lock();
    if (mapOfUsers.find(userId)==mapOfUsers.end()){
        mtxMapOfUsers.unlock();
        return false;
    }
    OneUserData * userData=mapOfUsers[userId];
    std::mutex& mtxGraph=userData->mtxGraph;
    mapOfUsers.erase(userId);
    mtxMapOfUsers.unlock();
    mtxGraph.lock();
    Graph *openedGraph=userData->graph;
    delete openedGraph;
    mtxGraph.unlock();
    delete userData;
    return true;
}

bool Server::initSubjectiveQualities(int userId){
    mtxMapOfUsers.lock();
    if (mapOfUsers.find(userId)==mapOfUsers.end()){
        mtxMapOfUsers.unlock();
        return false;
    }
    Graph *openedGraph=mapOfUsers[userId]->graph;
    std::mutex& mtxGraph=mapOfUsers[userId]->mtxGraph;
    mtxMapOfUsers.unlock();
    mtxGraph.lock();
    openedGraph->getHashtagCooccurrences()->initHashtagsLastEventIndices();
    openedGraph->getVerticesSubQualitiesManager()->initVerticesLastEventIndices();
    mtxGraph.unlock();
    
    return true;
}

bool Server::addNewLikes(std::string parametersFilePath){
    DesignPoint designPoint;
    readParametersAddNewLikes(parametersFilePath, designPoint);
    std::vector<Pattern> previousPatterns;
    if (showCouts){
        std::cout<<"load previous results"<<std::endl;
    }
    mtxMapOfUsers.lock();
    if (mapOfUsers.find(designPoint.userId)==mapOfUsers.end()){
        mtxMapOfUsers.unlock();
        return false;
    }
    Graph *openedGraph=mapOfUsers[designPoint.userId]->graph;
    std::mutex& mtxGraph=mapOfUsers[designPoint.userId]->mtxGraph;
    mtxMapOfUsers.unlock();
    mtxGraph.lock();
    loadPreviousResult(designPoint.fileContainingLikedPatterns, previousPatterns,openedGraph->getHashtagsMap(),openedGraph->getVerticesIds());
    if (showCouts){
        std::cout<<"previous results loaded"<<std::endl;
        std::cout<<"update hashtags with chosen patterns"<<std::endl;
    }
    for (int i : designPoint.chosenPatterns){
        
        openedGraph->getHashtagCooccurrences()->updateHashtagsLastEventsIndices(previousPatterns[i].hashtags);
    }
    if (showCouts){
        std::cout<<"update hashtags with chosen patterns done"<<std::endl;
        std::cout<<"update vertices with chosen patterns"<<std::endl;
    }
    for (int i : designPoint.chosenPatterns){
        openedGraph->getVerticesSubQualitiesManager()->updateVerticesLastEventsIndices(previousPatterns[i].vertexIndices);
    }
    if (showCouts){
        std::cout<<"update vertices with chosen patterns done"<<std::endl;
        std::cout<<"calculate and save hashtag subjective qualities"<<std::endl;
    }
    openedGraph->getHashtagCooccurrences()->calculateSubjectiveQualities(designPoint.alpha, designPoint.subjectivePower);
    if (showCouts){
        std::cout<<"hashtag subjective qualities calculated and saved"<<std::endl;
        std::cout<<"calculate and save vertices subjective qualities"<<std::endl;
    }
    openedGraph->getVerticesSubQualitiesManager()->calculateSubjectiveQualities(designPoint.alpha, designPoint.subjectivePower);
    if (showCouts){
        std::cout<<"vertices subjective qualities calculated and saved"<<std::endl;
    }
    mtxGraph.unlock();
    return true;
}

bool Server::continuePreferenceBased(std::string parameterFilePath){
    DesignPoint designPoint;
    Statistics statistics;
    
    readParametersContinuePreferenceBased(parameterFilePath, designPoint);
    mtxMapOfUsers.lock();
    if (mapOfUsers.find(designPoint.userId)==mapOfUsers.end()){
        mtxMapOfUsers.unlock();
        // user not found
        return false;
    }
    Graph * openedGraph=mapOfUsers[designPoint.userId]->graph;
    std::mutex& mtxGraph=mapOfUsers[designPoint.userId]->mtxGraph;
    mtxMapOfUsers.unlock();
    
    /*if (designPoint.userId==1 && designPoint.beginMineTime>0){
     std::cout<<"hohohohoho"<<std::endl;
     //std::this_thread::sleep_for(std::chrono::seconds(600));
     while (true){
     }
     std::cout<<"finishedSleeping"<<std::endl;
     }
     else {
     std::cout<<"userId:"<<designPoint.userId<<std::endl;
     std::cout<<"beginMineTime:"<<designPoint.beginMineTime<<std::endl;
     }*/
    
    mtxGraph.lock();
    
    
    
    if (designPoint.endMineTime==-1){
        designPoint.endMineTime=(int)openedGraph->getVertices()[0].periodicHashOcc.size();
    }
    int endTime=designPoint.endMineTime;
    if (designPoint.modelIsAll){
        designPoint.endMineTime=(int)openedGraph->getVertices()[0].periodicHashOcc.size();
    }
    
    
    struct timeb start, end;
    ftime(&start);
    
    if (showCouts){
        std::cout<<"set values of bounds"<<std::endl;
    }
    
    
    
    
    openedGraph->getHashtagCooccurrences()->setValuesForBounds(0, designPoint.endMineTime);
    
    
    ftime(&end);
    statistics.graphLoadingTime = getTimeComputationInS(start, end);
    ftime(&start);
    bool lastEventsIndicesChanged=false;
    if (designPoint.fileContainingLikedPatterns.compare("")!=0){
        std::vector<Pattern> previousPatterns;
        if (showCouts){
            std::cout<<"load previous results"<<std::endl;
        }
        loadPreviousResult(designPoint.fileContainingLikedPatterns, previousPatterns,openedGraph->getHashtagsMap(),openedGraph->getVerticesIds());
        if (showCouts){
            std::cout<<"previous results loaded"<<std::endl;
            
            std::cout<<"update hashtags with chosen patterns"<<std::endl;
        }
        for (int i : designPoint.chosenPatterns){
            lastEventsIndicesChanged=true;
            openedGraph->getHashtagCooccurrences()->updateHashtagsLastEventsIndices(previousPatterns[i].hashtags);
        }
        if (showCouts){
            std::cout<<"update hashtags with chosen patterns done"<<std::endl;
            
            std::cout<<"update vertices with chosen patterns"<<std::endl;
        }
        for (int i : designPoint.chosenPatterns){
            lastEventsIndicesChanged=true;
            openedGraph->getVerticesSubQualitiesManager()->updateVerticesLastEventsIndices(previousPatterns[i].vertexIndices);
        }
        if (showCouts){
            std::cout<<"update vertices with chosen patterns done"<<std::endl;
        }
        
    }
    if (showCouts){
        std::cout<<"calculate and save hashtag subjective qualities"<<std::endl;
    }
    openedGraph->getHashtagCooccurrences()->calculateSubjectiveQualities(designPoint.alpha, designPoint.subjectivePower);
    if (showCouts){
        std::cout<<"hashtag subjective qualities calculated and saved"<<std::endl;
        
        std::cout<<"calculate and save vertices subjective qualities"<<std::endl;
    }
    openedGraph->getVerticesSubQualitiesManager()->calculateSubjectiveQualities(designPoint.alpha, designPoint.subjectivePower);
    
    if (showCouts){
        std::cout<<"vertices subjective qualities calculated and saved"<<std::endl;
        
        
        
        std::cout<<"calculate relative measure"<<std::endl;
    }
    RelativeBasedMeasure measure(openedGraph,designPoint.stDevWeightFactor,designPoint.beginMineTime,designPoint.endMineTime,true);
    measure.calculateHashtagScores();
    ftime(&end);
    statistics.initMeasureCalculationTime=getTimeComputationInS(start, end);
    ftime(&start);
    if (showCouts){
        std::cout<<"relative measure calculated"<<std::endl;
    }
    if (designPoint.modelIsAll && endTime>0 && endTime<=openedGraph->getVertices().size()){
        designPoint.endMineTime=endTime;
    }
    if (showCouts){
        std::cout<<"end mine time:"<<designPoint.endMineTime<<std::endl;
        std::cout<<"nb times: "<<openedGraph->getVertices()[0].periodicHashOcc.size()<<std::endl;
    }
    
    if (designPoint.approach.compare("complete")==0){
        if (showCouts){
            std::cout<<"begin extraction"<<std::endl;
        }
        PatternExtractor extractor(openedGraph);
        extractor.retrievePatterns(designPoint,statistics);
        if (showCouts){
            std::cout<<"pattern mining finished"<<"nb patterns:"<<extractor.getRetrievedPatterns().size()<<", post processing now"<<std::endl;
        }
        ftime(&end);
        statistics.patternMiningTime=getTimeComputationInS(start, end);
        
        ftime(&start);
        
        std::vector<Pattern> finalPatterns;
        if (designPoint.clusterize){
            postProcess(openedGraph, extractor.getRetrievedPatterns(), statistics,designPoint,finalPatterns);
        }
        if (showCouts){
            std::cout<<"post processing finished, writing now"<<std::endl;
        }
        ftime(&end);
        statistics.clusteringTime=getTimeComputationInS(start, end);
        
        writeResult(openedGraph, extractor.getRetrievedPatterns(), designPoint.outputFilePath,ORDER_RESULT,statistics,designPoint);
        writePostProcessedResult(openedGraph, finalPatterns, designPoint.outputFilePath,ORDER_RESULT,statistics,designPoint);
        
        if (showCouts){
            std::cout<<"end extraction, results written"<<std::endl;
        }
    }
    else if (designPoint.approach.compare("sampling")==0){
        PatternSampler sampler(openedGraph);
        sampler.setUseMinQuality(true);
        sampler.setMinQuality(designPoint.minQuality);
        if (showCouts){
            std::cout<<"begin sampling"<<std::endl;
        }
        sampler.samplePatterns(designPoint);
        ftime(&end);
        statistics.patternMiningTime=getTimeComputationInS(start, end);
        if (showCouts){
            std::cout<<"end sampling"<<std::endl;
        }
        if (designPoint.postProcessing.compare("removeRepetition")==0){
            sampler.removeRepetition();
        }
        else if (designPoint.postProcessing.compare("removeRedundancy")==0){
            sampler.removeRedundancy();
        }
        if (showCouts){
            std::cout<<"begin write results"<<std::endl;
        }
        
        ftime(&start);
        std::vector<Pattern> finalPatterns;
        if (designPoint.clusterize){
            postProcess(openedGraph, sampler.getSampledPatterns(), statistics,designPoint,finalPatterns);
        }
        ftime(&end);
        statistics.clusteringTime=getTimeComputationInS(start, end);
        
        
        writeResult(openedGraph, sampler.getSampledPatterns(), designPoint.outputFilePath,ORDER_RESULT,statistics,designPoint);
        writePostProcessedResult(openedGraph, finalPatterns, designPoint.outputFilePath,ORDER_RESULT,statistics,designPoint);
        if (showCouts){
            std::cout<<"end write results"<<std::endl;
        }
    }
    else {
        if (showCouts){
            std::cout<<"problem in designPoint.approach value"<<std::endl;
        }
        mtxGraph.unlock();
        throw "unrecognized parameter";
    }
    mtxGraph.unlock();
    return true;
}


void Server::launchPreferenceBased(std::string parameterFilePath){
    DesignPoint designPoint;
    readParametersLaunchPreferenceBased(parameterFilePath, designPoint);
    //std::cout<<"tweetsFilePath:"<<designPoint.tweetsFilePath<<std::endl;
    OneUserData * oneUserData=new OneUserData();
    oneUserData->idUser=designPoint.userId;
    mtxMapOfUsers.lock();
    OneUserData * oldOne=nullptr;
    if (mapOfUsers.find(oneUserData->idUser)!=mapOfUsers.end()){
        oldOne=mapOfUsers[oneUserData->idUser];
    }
    mapOfUsers[oneUserData->idUser]=oneUserData;
    oneUserData->mtxGraph.lock();
    mtxMapOfUsers.unlock();
    Graph * openedGraph ;
    if (mainGraph==nullptr){
        openedGraph = (new GraphLoader(&designPoint,true))->load();
    }
    else {
        openedGraph=mainGraph->cloneGraph();
        if (showCouts){
            std::cout<<"Cloned"<<std::endl;
            std::cout<<"nbvertices:"<<openedGraph->getVertices().size()<<std::endl;
            std::cout<<"nbTimes:"<<openedGraph->getVertices()[0].periodicHashOcc.size()<<std::endl;
        }
    }
    oneUserData->graph=openedGraph;
    oneUserData->mtxGraph.unlock();
    
    //mtxMapOfUsers.lock();
    //if (mainGraph==nullptr){
    //    mainGraph=openedGraph->cloneGraph();
    //}
    //mtxMapOfUsers.unlock();
    
    if (oldOne!=nullptr){
        delete oldOne->graph;
        delete oldOne;
    }
}
void Server::loadMainGraph(std::string parameterFilePath){
    mtxMapOfUsers.lock();
    if (mainGraph!=nullptr){
        delete mainGraph;
    }
    DesignPoint designPoint;
    readParametersLaunchPreferenceBased(parameterFilePath, designPoint);
    mainGraph= (new GraphLoader(&designPoint,true))->load();
    firstGraph=false;
    mtxMapOfUsers.unlock();
}


void Server::doSimpleEventDetection(std::string parameterFilePath){
    DesignPoint designPoint;
    Statistics statistics;
    if (showCouts){
        std::cout<<"read parameters"<<std::endl;
    }
    readParametersSimpleDetection(parameterFilePath, designPoint);
    if (showCouts){
        std::cout<<"read parameters finished"<<std::endl;
    }
    struct timeb start, end;
    ftime(&start);
    int endTime=designPoint.endMineTime;
    if (designPoint.modelIsAll){
        designPoint.endMineTime=-1;
    }
    if (showCouts){
        std::cout<<"load graph"<<std::endl;
    }
    //int endTime=designPoint.endMineTime;
    //designPoint.endMineTime=-1;
    Graph * graph = (new GraphLoader(&designPoint,false))->load();
    if (showCouts){
        std::cout<<"load graph finished, number of vertices"<<std::to_string(graph->getVertices().size())<<std::endl;
    }
    ftime(&end);
    statistics.graphLoadingTime = getTimeComputationInS(start, end);
    ftime(&start);
    if (showCouts){
        std::cout<<"calculate hashtag scores"<<std::endl;
    }
    graph->getHashtagCooccurrences()->setValuesForBounds(designPoint.beginMineTime, designPoint.endMineTime);
    
    RelativeBasedMeasure measure(graph,designPoint.stDevWeightFactor,designPoint.beginMineTime,designPoint.endMineTime,false);
    measure.calculateHashtagScores();
    if (showCouts){
        std::cout<<"calculate hashtag finished"<<std::endl;
    }
    ftime(&end);
    statistics.initMeasureCalculationTime=getTimeComputationInS(start, end);
    ftime(&start);
    if (showCouts){
        std::cout<<"retrieve patterns"<<std::endl;
    }
    //designPoint.endMineTime=endTime;
    if (designPoint.modelIsAll && endTime>0 && endTime<=graph->getVertices().size()){
        designPoint.endMineTime=endTime;
    }
    if (designPoint.approach.compare("complete")==0){
        PatternExtractor extractor(graph);
        extractor.retrievePatterns(designPoint,statistics);
        ftime(&end);
        if (showCouts){
            std::cout<<"retrieve patterns finished"<<std::endl;
        }
        statistics.patternMiningTime=getTimeComputationInS(start, end);
        
        ftime(&start);
        std::vector<Pattern> finalPatterns;
        if (designPoint.clusterize){
            postProcess(graph, extractor.getRetrievedPatterns(), statistics,designPoint,finalPatterns);
        }
        ftime(&end);
        statistics.clusteringTime=getTimeComputationInS(start, end);
        
        writeResult(graph, extractor.getRetrievedPatterns(), designPoint.outputFilePath,ORDER_RESULT,statistics,designPoint);
        writePostProcessedResult(graph, finalPatterns, designPoint.outputFilePath,ORDER_RESULT,statistics,designPoint);
        
        
    }
    else if (designPoint.approach.compare("sampling")==0){
        PatternSampler sampler(graph);
        sampler.setUseMinQuality(true);
        sampler.setMinQuality(designPoint.minQuality);
        sampler.samplePatterns(designPoint);
        ftime(&end);
        statistics.patternMiningTime=getTimeComputationInS(start, end);
        ftime(&start);
        if (designPoint.postProcessing.compare("removeRepetition")==0){
            sampler.removeRepetition();
        }
        else if (designPoint.postProcessing.compare("removeRedundancy")==0){
            sampler.removeRedundancy();
        }
        ftime(&end);
        statistics.removeRepForSamp=getTimeComputationInS(start, end);
        if (showCouts){
            std::cout<<"retrieve patterns finished"<<std::endl;
        }
        
        
        ftime(&start);
        std::vector<Pattern> finalPatterns;
        if (designPoint.clusterize){
            postProcess(graph, sampler.getSampledPatterns(), statistics,designPoint,finalPatterns);
        }
        ftime(&end);
        statistics.clusteringTime=getTimeComputationInS(start, end);
        
        writeResult(graph, sampler.getSampledPatterns(), designPoint.outputFilePath,ORDER_RESULT,statistics,designPoint);
        writePostProcessedResult(graph, finalPatterns, designPoint.outputFilePath,ORDER_RESULT,statistics,designPoint);
    }
    else {
        std::cout<<"problem in designPoint.approach value"<<std::endl;
        throw "unrecognized parameter";
    }
    if (showCouts){
        std::cout<<"remove graph"<<std::endl;
    }
    delete graph;
    if (showCouts){
        std::cout<<"remove graph finished"<<std::endl;
    }
    
    
}



bool Server::init(){
    struct sockaddr_in my_addr;
    int * p_int ;
    
    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        printf("Error initializing socket %d\n", errno);
        return false;
    }
    
    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;
    
    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
       (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        printf("Error setting options %d\n", errno);
        free(p_int);
        return false;
    }
    free(p_int);
    
    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);
    
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = INADDR_ANY ;
    
    
    if( bind( hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        fprintf(stderr,"Error binding to socket, make sure nothing else is listening on this port %d\n",errno);
        return false;
    }
    if(listen( hsock, 10) == -1 ){
        fprintf(stderr, "Error listening %d\n",errno);
        return false;
    }
    //Now lets do the server stuff
    
    addr_size = sizeof(sockaddr_in);
    return true;
}

void Server::start(){
    end=false;
    //std::vector<std::future<int>> futures;
    std::vector<std::thread *> allThreads;
    while(!end){
        if (showCouts){
            printf("waiting for a connection\n");
        }
        int* csock = (int*)malloc(sizeof(int));
        if((*csock = accept( hsock, (sockaddr*)&sadr, &addr_size))!= -1){
            try {
                if (showCouts){
                    printf("---------------------\nReceived connection from %s\n",inet_ntoa(sadr.sin_addr));
                }
                std::string command=readCommandFromSocket(csock);
                if (command.at(command.length() - 1) == '\r' || command.at(command.length() - 1) == '\n') {
                    command = command.substr(0, command.length() - 1);
                }
                
                if (showCouts){
                    std::cout<<"command is:'"<<command<<"'"<<std::endl;
                }
                if (command.compare("stop")==0){
                    if (showCouts){
                        std::cout<<"stop command"<<std::endl;
                    }
                    stop();
                    std::string respond;
                    respond="execution terminated successfully\n";
                    //char buffer[1024];
                    //int buffer_len = 1024;
                    int bytecount;
                    bytecount = send(*csock, respond.data(),respond.size(), 0);
                    close(*csock);
                    free(csock);
                }
                else {
                    if (showCouts){
                        std::cout<<"create thread"<<std::endl;
                    }
                    std::thread * first=new std::thread(&Server::socketHandler,this,csock,command);
                    allThreads.push_back(first);
                }
            }
            catch (int e){
                if (showCouts){
                    std::cout<<"exception in main server loop "<<std::endl;
                }
            }
        }
        else{
            fprintf(stderr, "Error accepting %d\n", errno);
        }
    }
    if (showCouts){
        std::cout<<"joining threads"<<std::endl;
    }
    //for (std::future<int> &fut : futures){
    //    fut.get();
    //}
    for (std::thread * curThread : allThreads){
        curThread->join();
        delete curThread;
    }
    allThreads.clear();
    if (showCouts){
        std::cout<<"threads are joined"<<std::endl;
    }
    //doSimpleEventDetection("parameters.txt");
    
}
void Server::stop(){
    end=true;
}


