# Non-Functional Monitor: `serverActivityChecker`

## Overview

The `serverActivityChecker` is a non-functional monitor designed to continuously evaluate the activity and responsiveness of three servers:
- **Supplier Server**
- **Customer Server**
- **Carrier Server**

This monitor uses a `PING/PONG` message exchange system via Redis to track server responsiveness in real-time. It ensures consistent activity tracking, highlights delays, and logs server statuses for further analysis.

## Core Features

1. **Connection to Redis**:
   - The monitor establishes a connection to Redis using the `hiredis` library.
   - If the connection fails, the system terminates with an error message.

2. **Stream Communication**:
   - **Request Streams (PING)**: Dedicated streams for sending PING messages to each server (`ping_supplier`, `ping_customer`, `ping_carrier`).
   - **Response Streams (PONG)**: Dedicated streams for receiving PONG messages from each server (`pong_supplier`, `pong_customer`, `pong_carrier`).

3. **Real-Time Monitoring**:
   - Operates in a continuous `while` loop to send PINGs, wait for PONGs, and update server statuses in real-time.

4. **Evaluation Logic**:
   - Tracks server responses using sequence numbers (`seq_num`) to determine the state of each server:
     - **Active**: Responds with the current sequence number.
     - **Late**: Responds with the previous sequence number.
     - **Non-Responsive**: Fails to respond within the expected time interval.

5. **Logging**:
   - Logs server statuses into the database for further analysis.
   - Generates warnings for late or non-responsive servers and informational logs for active servers.

## Flowchart Diagram

```mermaid
flowchart TD
    Start[Start Monitoring] --> CheckConnection[Check Redis Connection]
    CheckConnection -->|Connection Successful| InitStreams[Initialize Streams]
    CheckConnection -->|Connection Failed| Terminate[Log Error and Terminate]
    InitStreams --> PingServers[Send PING to Servers]
    PingServers --> WaitPONG[Wait for PONG Responses]
    WaitPONG --> EvaluateResponses[Evaluate Server Responses]
    EvaluateResponses -->|Active| LogActive[Log Servers as Active]
    EvaluateResponses -->|Late| LogLate[Log Servers as Late]
    EvaluateResponses -->|Non-Responsive| LogNonResponsive[Log Non-Responsive Servers]
    LogActive --> NextIteration[Prepare for Next Iteration]
    LogLate --> NextIteration
    LogNonResponsive --> NextIteration
    NextIteration --> PingServers
    Terminate --> End[End Process]
