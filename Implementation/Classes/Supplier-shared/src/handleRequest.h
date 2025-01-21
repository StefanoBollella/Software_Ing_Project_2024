#ifndef HANDLE_REQUEST_H
#define HANDLE_REQUEST_H

#include<iostream>
#include<sstream>
#include<string>

enum REQ_TYPE {
    NEW_PRODUCT = 1,    //Request for registration of a New Product
    INFO_PRODUCT = 2,   //Request for information on quantities of registered products
    UPDATE_PRODUCT = 3, //Request for updating quantities of registered products
    SUPPLIER_ID = 4     //Request to server to generate new unique ids for suppliers 
};

enum STATUS_REQ {
    REQ_SUCCESS = 305,    //Request successfully completed
    REQ_FAILED = 306,    //Request Failed
    BAD_REQUEST = 307,    //Wrong request due to formatting error
    TIMEOUT_FAILED = 308, //Request failed due to timeout
    NO_RESPONSE = 309       //Returned when the client receives no reply 
                         // from the server within a single communication attempt
};

class HandleRequest {

public:
    
    //Static method for verifying ON THE SERVER SIDE that the type of request sent by the client matches exactly one 
   //of the permitted REQ_TYPE 
   static REQ_TYPE verifyRequestType(const std::string commandName, const int commandValue);
  

   static std::string reqTypeToString(REQ_TYPE reqType);
  
};
#endif //HANDLE_REQUEST_H


