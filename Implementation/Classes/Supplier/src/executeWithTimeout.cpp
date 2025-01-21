#include "main.h"

/** * Executes the provided communication function 'func' within a specified timeout interval. 
    * This function acts as a wrapper for 'func', ensuring it completes within a given 
    * time frame (defined by 'timeout_ms' in milliseconds). If 'func' does not finish 
    * within the allowed interval, 'executeWithTimeout' returns a status indicating 
    * a timeout failure. 
    * 
    * @param func The communication function to execute, which interacts with the server.
    * This function is expected to perform a server-related task and 
    * must follow the 'CommunicationFunction' signature. 
    * 
    * @param timeout_ms The maximum time (in milliseconds) allowed for `func` to complete. 
    * If 'func' exceeds this duration, the execution is terminated. 
    * 
    * @return STATUS_REQ Returns a status code based on the outcome: 
    * - Success: If 'func' completes within 'timeout_ms'. 
    * - Timeout failure: If 'func' does not complete in time. 
    */
STATUS_REQ executeWithTimeout(CommunicationFunction func, int timeout_ms) {
    auto start = std::chrono::steady_clock::now();
    
    while(true){
        
        //Executes the communication function
        STATUS_REQ status = func();

        //Checks whether the function completed successfully or failed
        if(status == REQ_SUCCESS || status == REQ_FAILED || status == BAD_REQUEST){
            return status;
        }

        //If the function answered with NO_RESPONSE, wait and try again
        if(status == NO_RESPONSE){
            std::cout << "No response received from Supplier-server, retrying...\n";
        }

        //Calculates the elapsed time since the start of execution
        auto now = std::chrono::steady_clock::now();
        int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

        //Checks whether the timeout has expired
        if (elapsed_ms >= timeout_ms) {
            std::cout << "Timeout expired during function execution\n";
            return TIMEOUT_FAILED;
        }

        //Waits 100 ms before retrying
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

