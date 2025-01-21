# Classes - Implementation

This directory contains the source code organized into subdirectories for different components of the system.

## Directory Structure

| **Directory**                  | **Description**                                                                                                                                               |
|---------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **[Carrier-server](./Classes/Carrier-server/)**     | Server-side implementation for the **Carrier** module, handling tasks like order delivery and management.                                                  |
| **[Customer-server](./Classes/Customer-server/)**   | Server-side implementation for the **Customer** module, processing actions like placing orders.                                                             |
| **[Supplier-server](./Classes/Supplier-server/)**   | Server-side implementation for the **Supplier** module, managing tasks like product registration and updates.                                               |
| **[Carrier-shared](./Classes/Carrier-shared/)**     | Shared utilities and resources for the **Carrier** and **Carrier-server** modules, aiding in reusable functionality.                                         |
| **[Customer-shared](./Classes/Customer-shared/)**   | Shared code and utilities for the **Customer** and **Customer-server** modules, ensuring consistent behavior.                                                |
| **[Supplier-shared](./Classes/Supplier-shared/)**   | Shared resources for the **Supplier** and **Supplier-server** modules, facilitating common functionality.                                                    |
| **[Carrier](./Classes/Carrier/)**                   | Client-side implementation for the **Carrier** module, enabling features like order management and delivery.                                                 |
| **[Customer](./Classes/Customer/)**                 | Client-side implementation for the **Customer** module, supporting actions like browsing products and creating orders.                                        |
| **[Supplier](./Classes/Supplier/)**                 | Client-side implementation for the **Supplier** module, focusing on tasks like updating product quantities and interacting with the Supplier server.         |
| **[Clock](./Classes/Clock/)**                       | Utility functions for time tracking and synchronization, essential for measuring delays and performance metrics.                                              |
| **[con2db](./Classes/con2db/)**                     | Utilities for database interactions, including connecting to PostgreSQL and executing queries.                                                               |
| **[con2redis](./Classes/con2redis/)**               | Code for connecting to Redis and managing message streams between clients, servers, and monitors.                                                            |
| **[ecommerce-db-scripts](./Classes/ecommerce-db-scripts/)** | Scripts for setting up and managing the e-commerce database, storing system activity.                                                                      |
| **[log-db-scripts](./Classes/log-db-scripts/)**     | Scripts for setting up and managing the log database, storing system logs and activities.                                                                    |
| **[logger](./Classes/logger/)**                     | Logging utilities for all system components, integrating with the log database for persistent storage.                                                       |
| **[monitors](./Classes/monitors/)**                 | Monitoring tools, including the `serverActivityChecker`, for ensuring the responsiveness and activity of servers.                                             |
| **[init-dbs.sh](../init-dbs.sh)**                   | Script for initializing the databases and resetting their state with necessary schemas.                                                                       |
| **[system-init.sh](../system-init.sh)**             | Script to set up the entire system, including Redis streams and other infrastructure.                                                                        |
