#ifndef SERVER_STATE_H
#define SERVER_STATE_H
#include<iostream>
#include<string>
#include<stdexcept>

enum class ServerStatus {
    INITIALIZING,    //The server is starting its resources, including connecting to Redis.
    CONNECTED,       //The server is connected to Redis, but is not yet ready to handle requests.
    READY,           //The server is ready to handle requests.
    BUSY,            //The server is busy processing a request.
    TERMINATED       //The server is terminating execution.
};

class ServerState {

private:
      ServerStatus currentState; 
public:
    
   ServerState();
   std::string getStateString(ServerStatus state) const; 
   ServerStatus getCurrentState() const;
   void updateServerState(ServerStatus newState);    
};
#endif //SERVER_STATE_H

