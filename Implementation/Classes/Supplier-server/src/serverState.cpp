#include "serverState.h"
/**
 * @brief Constructor for ServerState.
 *
 * Initializes the server's state to `INITIALIZING`, representing the starting phase of the server.
 */
ServerState::ServerState() 
    : currentState(ServerStatus::INITIALIZING) {}

/**
 * @brief Converts a `ServerStatus` enum value to its corresponding string representation.
 * 
 * @param state The state to convert.
 * @return A string representing the given state.
 */
std::string ServerState::getStateString(ServerStatus state) const{
    switch(state) {
        case ServerStatus::INITIALIZING: return "INITIALIZING";
        case ServerStatus::CONNECTED: return "CONNECTED";
        case ServerStatus::READY: return "READY";
        case ServerStatus::BUSY: return "BUSY";
        case ServerStatus::TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Retrieves the current state of the server.
 *
 * @return The current `ServerStatus` of the server.
 */
ServerStatus ServerState::getCurrentState() const{
    return currentState;
}

/**
 * @brief Updates the current state of the server, ensuring the transition is valid.
 *
 * Valid transitions:
 * - `INITIALIZING` -> `CONNECTED`, `TERMINATED`
 * - `CONNECTED` -> `READY`, `TERMINATED`
 * - `READY` -> `BUSY`, `TERMINATED`
 * - `BUSY` -> `READY`, `TERMINATED`
 *
 * @param newState The new state to transition to.
 *
 * @throws std::runtime_error If attempting to transition from `TERMINATED`.
 * @throws std::invalid_argument If the transition from the current state to the new state is invalid.
 */
void ServerState::updateServerState(ServerStatus newState){
  
    if(currentState == ServerStatus::TERMINATED){
        throw std::runtime_error("Cannot change state: server is in TERMINATED state.");
    }
    
    switch(currentState){
        case ServerStatus::INITIALIZING:
            if(newState == ServerStatus::CONNECTED || newState == ServerStatus::TERMINATED){
                currentState = newState;
                if (newState == ServerStatus::TERMINATED) {
                    std::cout << "Server is shutting down." << std::endl;
                }
            } 
            else {
                throw std::invalid_argument("Invalid transition from " + getStateString(currentState) 
                                            + " to " + getStateString(newState));
            }
            break;

        case ServerStatus::CONNECTED:
            if (newState == ServerStatus::READY || newState == ServerStatus::TERMINATED){
                currentState = newState;
                if (newState == ServerStatus::TERMINATED) {
                    std::cout << "Server is shutting down." << std::endl;
                }
            } 
            else{
                throw std::invalid_argument("Invalid transition from " + getStateString(currentState) 
                                            + " to " + getStateString(newState));
            }
            break;

        case ServerStatus::READY:
            if(newState == ServerStatus::BUSY || newState == ServerStatus::TERMINATED){
                currentState = newState;
                if (newState == ServerStatus::TERMINATED) {
                    std::cout << "Server is shutting down." << std::endl;
                }
            } 
            else{
                throw std::invalid_argument("Invalid transition from " + getStateString(currentState) 
                                            + " to " + getStateString(newState));
            }
            break;

        case ServerStatus::BUSY:
            if(newState == ServerStatus::READY || newState == ServerStatus::TERMINATED){
                currentState = newState;
                if(newState == ServerStatus::TERMINATED){
                    std::cout << "Server is shutting down." << std::endl;
                }
            } 
            else {
                throw std::invalid_argument("Invalid transition from " + getStateString(currentState) 
                                            + " to " + getStateString(newState));
            }
            break;

        default:
            throw std::runtime_error("Current status unknown.");
    }
}

