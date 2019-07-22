//
//  main.cpp
//  EventDetectionServer
//
//  Created by Anes Bendimerad on 02/01/2017.
//  Copyright © 2017 Anes Bendimerad. All rights reserved.
//

#include <iostream>
#include "Server.hpp"

//void* SocketHandler(void*);


int main(int argc, const char * argv[]) {
    if (argc==2 || argc==3){
        Server *appServer=new Server();
        
        std::string commandName=argv[1];
        if (commandName.compare("launchServer")==0){
            if (showCouts){
                std::cout<<"server is launched"<<std::endl;
            }
            if (argc>=3){
                int portNumber=std::stoi(argv[2]);
                appServer->host_port=portNumber;
                // the third parameter is the port number
            }
            appServer->init();
            appServer->start();
        }
        else if (commandName.compare("objectiveEventDetection")==0){
            if (argc==3){
                //std::cout<<"objective event detection"<<std::endl;
                std::string parametersFile=argv[2];
                appServer->doSimpleEventDetection(parametersFile);
                //std::thread first(&Server::doSimpleEventDetection,appServer,parametersFile);
                //std::cout<<"blabla"<<std::endl;
                //std::cout<<"blabla"<<std::endl;
                //first.join();
            
            }
            else {
                std::cerr<<"wrong number of arguments"<<std::endl;
            }
        }
        else {
            std::cerr<<"wrong first argument"<<std::endl;
        }
    }
    else {
        std::cerr<<"wrong number of arguments"<<std::endl;
    }
    
    
    return 0;
}
