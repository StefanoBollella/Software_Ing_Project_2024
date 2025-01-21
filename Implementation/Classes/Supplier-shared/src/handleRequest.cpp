#include "handleRequest.h"

/**
 * @brief Verifies and matches a request type based on the command name and value.
 *
 * This function maps a command name to its corresponding request type
 * and verifies whether the numeric command value matches the expected
 * value for the determined request type.
 *
 * @param commandName A string representing the name of the command (e.g., "NEW_PRODUCT").
 * @param commandValue An integer representing the numeric value associated with the command.
 *
 * @return A `REQ_TYPE` corresponding to the verified command name and value.
 *
 * @throws std::invalid_argument If the command name is not recognized or if the
 *         command value does not match the expected value for the given command name.
 */
REQ_TYPE HandleRequest::verifyRequestType(const std::string commandName, const int commandValue){
   
    REQ_TYPE current_req; 
  
    if(commandName == "NEW_PRODUCT"){
        current_req = REQ_TYPE::NEW_PRODUCT;
    }
    else if(commandName == "INFO_PRODUCT"){
        current_req = REQ_TYPE::INFO_PRODUCT;
    } 
    else if(commandName == "UPDATE_PRODUCT"){
        current_req = REQ_TYPE::UPDATE_PRODUCT;
    }  
    else if(commandName == "SUPPLIER_ID"){
        current_req = REQ_TYPE::SUPPLIER_ID;
    }
    else{
        throw std::invalid_argument("Invalid command name: " + commandName);
    }

    if(static_cast<int>(current_req) == commandValue){
        return current_req;
    } 
    else{
        throw std::invalid_argument("Invalid command value for " + commandName + ": " + std::to_string(commandValue));
    }
}

 /**
 * @brief Converts a request type to its string representation.
 *
 * This function is used to map a `REQ_TYPE` enumeration value to its
 * corresponding string representation, useful for logging or debugging.
 *
 * @param reqType The `REQ_TYPE` value to be converted to a string.
 *
 * @return A string representing the `REQ_TYPE` (e.g., "NEW_PRODUCT").
 *         If the type is unknown, returns "UNKNOWN_REQUEST_TYPE".
 */
std::string HandleRequest::reqTypeToString(REQ_TYPE reqType) {
    switch(reqType){
        case NEW_PRODUCT:
            return "NEW_PRODUCT";
        case INFO_PRODUCT:
            return "INFO_PRODUCT";
        case UPDATE_PRODUCT:
            return "UPDATE_PRODUCT";
        case SUPPLIER_ID:
            return "SUPPLIER_ID";
        default:
            return "UNKNOWN_REQUEST_TYPE";
    }
} 
    



   
   
  
