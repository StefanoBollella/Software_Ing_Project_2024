CREATE TYPE LOG_LEVEL AS ENUM ('INFO', 'DEBUG', 'WARNING', 'ERROR', 'CRITICAL');
CREATE TYPE COMPONENT AS ENUM ('CUSTOMER', 'CARRIER', 'SUPPLIER', 'MONITOR');
CREATE DOMAIN STR4INFO AS VARCHAR(1000);
CREATE DOMAIN STR4CONTEXT AS VARCHAR(255);
CREATE DOMAIN OP_NAME AS VARCHAR(50);

CREATE TABLE operation_type (
    name OP_NAME PRIMARY KEY
);

CREATE TABLE operation (
    id serial PRIMARY KEY,
    op_type OP_NAME NOT NULL,
    user_id INT,
    user_state INT,
    req_id INT,
    component COMPONENT NOT NULL,  -- CUSTOMER, CARRIER, SUPPLIER, MONITOR
    source_context STR4CONTEXT,  -- part of the system where the event occurred 
    FOREIGN KEY(op_type) REFERENCES operation_type(name)
);

CREATE TABLE log_table (
    id serial PRIMARY KEY,
    level LOG_LEVEL NOT NULL,
    pid INT NOT NULL,  -- process id
    operation_id INT NOT NULL,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
    nanosec bigint NOT NULL,  -- nanoseconds after seconds in timestamp
    duration_nanos bigint, -- duration in nanoseconds of operation
    loginfo STR4INFO,
    FOREIGN KEY(operation_id) REFERENCES operation(id)
);

