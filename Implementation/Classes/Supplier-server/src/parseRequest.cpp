#include "main.h"
#define INDEX_REQUEST_TYPE 0 

/**
 * @brief Extracts and validates the type of a request 
 * from a Redis message. It determines the request type (`REQ_TYPE`) based 
 * on the values in the message and performs error handling if the format 
 * is incorrect.
 *
 * @param reply          Pointer to the Redis reply object containing the message.
 * @param stream_index   Index of the stream within the Redis reply.
 * @param message_index  Index of the message within the stream.
 * @param error          Reference to a `HandleError` instance for managing errors.
 * 
 * @return `REQ_TYPE` The parsed request type, if valid.
 */

REQ_TYPE parseRequest(redisReply* reply, size_t stream_index, size_t message_index, HandleError& error) {
    char key[100];
    char value[100];
    REQ_TYPE reqType;

    //Extracts the command type and value from the Redis message
    ReadStreamMsgVal(reply, stream_index, message_index, INDEX_REQUEST_TYPE, key);
    ReadStreamMsgVal(reply, stream_index, message_index, INDEX_REQUEST_TYPE + 1, value);

    try {
        //Checks whether the value is a positive integer
        if(!isPositiveInteger(value)){
            error.handleError(ERR_STREAM_PARSE_FAILED, "Incorrect command code format.");
        }
        //Checks the request type with the extracted values
        reqType = HandleRequest::verifyRequestType(key, std::stoi(value));
        //std::cout << "Request type is valid, Code: " << static_cast<int>(reqType) << std::endl;
    } 
    catch (const std::invalid_argument& e){
        std::cerr << e.what() << std::endl;
        error.handleError(ERR_STREAM_PARSE_FAILED, "Wrong request type from client.");
    }

    return reqType;  //Returns the correctly parsed request type
}
