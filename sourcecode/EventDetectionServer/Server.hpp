//
//  Server.hpp
//  Server2
//
//  Created by Anes Bendimerad on 30/12/2016.
//  Copyright © 2016 Anes Bendimerad. All rights reserved.
//

#ifndef Server_hpp
#define Server_hpp

#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string>


#include "string_management.h"
#include "utils.hpp"
#include "graph_loader.h"
#include "RelativeBasedMeasure.h"
#include "result_management.h"
#include "pattern_sampler.h"
#include "PostProcessingManagement.h"
#define DEFAULT_PORT 20500
#define ORDER_RESULT true
#include <mutex> 
#include <future>
#include <iostream> 
#include <unistd.h>
//using namespace std;

typedef struct OneUserData
{
    int idUser;
    Graph * graph ;
    long lastUpdatedTimeStamp;
    std::mutex mtxGraph;
} OneUserData;


class Server
{
public:
    Server();
    
    bool init();
    void start();
    void stop();
    void doSimpleEventDetection(std::string parameterFilePath);
    int socketHandler(int* csock,std::string command);
    unsigned int host_port = DEFAULT_PORT;
private:
    //int* csock;
    int hsock;
    bool end;
    sockaddr_in sadr;
    socklen_t addr_size = 0;
    pthread_t thread_id=0;
    
    bool executeCommand(std::string command);
    void launchPreferenceBased(std::string parameterFilePath);
    void loadMainGraph(std::string parameterFilePath);
    bool continuePreferenceBased(std::string parameterFilePath);
    bool addNewLikes(std::string parametersFilePath);
    bool initSubjectiveQualities(int userId);
    bool closeUser(int userId);
    //std::string executeRequest(std::string query) ;
    //Graph *openedGraph;
    std::map<int,OneUserData*> mapOfUsers;
    std::mutex mtxMapOfUsers;
    Graph * mainGraph;
    bool firstGraph;
    std::string readCommandFromSocket(int *csock);
};

//int launchThreadSocketHandler(Server & server, int * csock);

//void* socketHandler(void*);

#endif /* Server_hpp */
