#include "handleError.h"
//Constructor initializes the error state to no error.
HandleError::HandleError():lastErrorCode(ERR_NONE), 
      lastErrorMessage("") {}

/**
 * @brief Handles an error by storing its details and throwing an exception.
 *
 * This function updates the last error code and message, then throws a
 * runtime exception containing the error details.
 *
 * @param code The error code representing the type of error.
 * @param errorMsg A detailed message describing the error.
 *
 * @throws std::runtime_error Always throws with a message in the format:
 *         "Error [code]: errorMsg".
 */
void HandleError::handleError(ErrorCode code, const std::string& errorMsg){
    lastErrorCode = code;
    lastErrorMessage = errorMsg;
    throw std::runtime_error("Error [" + std::to_string(code) + "]: " + errorMsg);
}

/**
 * @brief Retrieves the most recent error code.
 *
 * @return The last error code encountered.
 */
ErrorCode HandleError::getLastErrorCode() const{
    return lastErrorCode;
}

/**
 * @brief Retrieves the most recent error message.
 *
 * @return A string containing the last error message encountered.
 */
std::string HandleError::getLastErrorMessage() const{
    return lastErrorMessage;
}
