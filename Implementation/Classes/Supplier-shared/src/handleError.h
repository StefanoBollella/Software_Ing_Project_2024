#ifndef HANDLE_ERROR
#define HANDLE_ERROR

#include<iostream>
#include<string>
#include<stdexcept> 

enum ErrorCode {
       ERR_NONE = 1000,                     //No errors
       ERR_CONNECTION_FAILED = 1001,        //Connection to Redis failed
       ERR_STREAM_DELETE_FAILED = 1002,     //Elimination of stream failed
       ERR_STREAM_INIT_FAILED = 1003,       //Stream initialisation failed
       ERR_PROCESSING_FAILED = 1004,        //Error while processing a request, could not parse the response read from the redis stream
       ERR_STREAM_PARSE_FAILED = 1005,      //A request parsing error or a problem with the format of the received data
       ERR_CONNECTION_TEST_FAILED = 1006,   //Initial connection to Redis successful, but connection test fails
};


class HandleError{
  private : 
         ErrorCode lastErrorCode;     //Last error code encountered
         std::string lastErrorMessage; //Last error message
  public : 
      HandleError();
      void handleError(ErrorCode code, const std::string& errorMsg);
      ErrorCode getLastErrorCode() const;
      std::string getLastErrorMessage() const; 
}; 

#endif //HANDLE_ERROR
