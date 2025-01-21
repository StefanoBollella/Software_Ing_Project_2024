

\c :dbname postgres

-- user already exists
GRANT ALL PRIVILEGES ON DATABASE :dbname to :username ;


ALTER TABLE CUSTOMER OWNER TO :username;
ALTER TABLE SUPPLIER OWNER TO :username;
ALTER TABLE CARRIER OWNER TO :username;
ALTER TABLE PRODUCT OWNER TO :username;
ALTER TABLE ORDER_T OWNER TO :username;
ALTER TABLE ORDPROD OWNER TO :username;
ALTER TABLE TRAVELING OWNER TO :username;
ALTER TABLE CANCELED OWNER TO :username;
ALTER TABLE DELIVERED OWNER TO :username;
ALTER TABLE LOST OWNER TO :username;


-- grant all privileges on all tables in schema public to :username ;
-- grant all privileges on all sequences in schema public to :username ;

GRANT ALL ON SCHEMA public TO :username ;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO :username ;
