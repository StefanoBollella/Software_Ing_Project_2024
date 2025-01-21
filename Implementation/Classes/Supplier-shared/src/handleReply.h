#ifndef HANDLE_REPLY_H
#define HANDLE_REPLY_H

#include<iostream>
#include<sstream>
#include<string>

enum REPLY_TYPE {
    SUCCESS_REQ = 1,  //Response successfully to client request
    FAILED_REQ = 2,    //Processing request from server failed
    INVALID_FORMAT_REQ = 3 //Request with wrong format, parsing not possible
};

class HandleReply {
   public : 
      //Static method for verifying ON THE CLIENT SIDE that the type of reply sent by the server matches exactly one of the allowed REPLY_TYPE 
     static  REPLY_TYPE verifyReplyType(const std::string commandName, const int commandValue);  
};
#endif //HANDLE_REPLY_H


