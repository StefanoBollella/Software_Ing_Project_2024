#include "handleReply.h"
/**
 * @brief Verifies and matches a reply type based on the command name and value.
 *
 * This function takes the command name and value as inputs, validates them,
 * and maps the command to its corresponding reply type. If the command name
 * or value does not match the expected format, it throws an exception.
 *
 * @param commandName A string representing the name of the command (e.g., "SUCCESS_REQ").
 * @param commandValue An integer representing the value associated with the command.
 *
 * @return A `REPLY_TYPE` corresponding to the verified command name and value.
 *
 * @throws std::invalid_argument If the command name is not recognized or if the
 *         command value does not match the expected value for the given command name.
 */
REPLY_TYPE HandleReply::verifyReplyType(const std::string commandName, const int commandValue){
   
   REPLY_TYPE current_reply; 
  
    
    if(commandName == "SUCCESS_REQ"){
        current_reply = REPLY_TYPE::SUCCESS_REQ;
    }
    else if(commandName == "FAILED_REQ"){
        current_reply = REPLY_TYPE::FAILED_REQ;
    } 
    else if(commandName == "INVALID_FORMAT_REQ"){
        current_reply = REPLY_TYPE::INVALID_FORMAT_REQ;
    }
    else{
        throw std::invalid_argument("Invalid command name: " + commandName);
    }

    if(static_cast<int>(current_reply) == commandValue){
        return current_reply; 
    } 
    else{
        throw std::invalid_argument("Invalid command value for " + commandName + ": " + std::to_string(commandValue));
    }
}

      
