#include "main.h"

/**
 * @brief Parses a Redis reply to determine the type of response.
 *
 * This function extracts the command type and code from a Redis message,
 * validates their format, and determines the reply type (e.g., SUCCESS, FAILED, BAD_REQUEST).
 * In case of invalid formatting or unexpected values, the error is handled appropriately.
 *
 * @param reply Pointer to the Redis reply object.
 * @param stream_index Index of the stream in the reply array.
 * @param message_index Index of the message within the stream.
 * @param error Reference to an error-handling object.
 * @return The type of reply (e.g., SUCCESS_REQ, FAILED_REQ).
 */

 REPLY_TYPE parseReply(redisReply* reply, size_t stream_index, size_t message_index, HandleError& error){
 
 char key[100];
 char value[100];
 REPLY_TYPE replyType;

    //Extracts the command type and associated value from the Redis message
    ReadStreamMsgVal(reply, stream_index, message_index,0, key); //0 Position in the command message
    ReadStreamMsgVal(reply, stream_index, message_index,1, value);//1 Position in the command code message

    try {
        
        if(!isPositiveInteger(value)){
            error.handleError(ERR_STREAM_PARSE_FAILED, "Incorrect command code format.");
        }
        //Checks the type of request sent by the client
        replyType = HandleReply::verifyReplyType(key, std::stoi(value));
        //std::cout << "Request type is valid, Code: " << static_cast<int>(reqType) << std::endl;
    } 
    catch (const std::invalid_argument& e){
        std::cerr << e.what() << std::endl;
        error.handleError(ERR_STREAM_PARSE_FAILED, "Wrong request type from client.");
    }

    return replyType;  //Returns the request type 
}
        
