CREATE DOMAIN INTGZ AS INTEGER CHECK (VALUE > 0);

CREATE DOMAIN INTGEZ AS INTEGER CHECK (VALUE >= 0);

CREATE TABLE CUSTOMER (
  ID SERIAL PRIMARY KEY,
  SPAWNTIME TIMESTAMP NOT NULL
);

CREATE TABLE SUPPLIER (
  ID SERIAL PRIMARY KEY,
  SPAWNTIME TIMESTAMP NOT NULL
);

CREATE TABLE CARRIER (
  ID SERIAL PRIMARY KEY,
  SPAWNTIME TIMESTAMP NOT NULL
);

CREATE TABLE PRODUCT (
  ID SERIAL PRIMARY KEY,
  PRICE INTGZ,
  QUANTITY INTGEZ NOT NULL,
  SUPPLIER INTEGER NOT NULL,
  DATETIME TIMESTAMP NOT NULL,
  FOREIGN KEY (SUPPLIER) REFERENCES SUPPLIER(ID)
);

-- NOTE: ORDER is a reserved word
CREATE TABLE ORDER_T (
  ID SERIAL PRIMARY KEY,
  DATETIME TIMESTAMP NOT NULL,
  CUSTOMER INTEGER NOT NULL,
  FOREIGN KEY (CUSTOMER) REFERENCES CUSTOMER(ID)
);

CREATE TABLE ORDPROD (
  ORDER_ID INTEGER NOT NULL,
  PRODUCT_ID INTEGER NOT NULL,
  QUANTITY INTGZ NOT NULL,
  PRIMARY KEY(ORDER_ID, PRODUCT_ID),
  FOREIGN KEY (ORDER_ID) REFERENCES ORDER_T(ID),
  FOREIGN KEY (PRODUCT_ID) REFERENCES PRODUCT(ID)
);

--this table has been removed since it is redundant. 
/*
CREATE TABLE WAITING (
  ORDER_ID INTEGER PRIMARY KEY,
  DATETIME TIMESTAMP NOT NULL,
  FOREIGN KEY (ORDER_ID) REFERENCES ORDER_T(ID)
);
*/

CREATE TABLE TRAVELING (
  ORDER_ID INTEGER PRIMARY KEY,
  DATETIME TIMESTAMP NOT NULL,
  CARRIER_ID INTEGER NOT NULL,
  FOREIGN KEY (ORDER_ID) REFERENCES ORDER_T(ID),
  FOREIGN KEY (CARRIER_ID) REFERENCES CARRIER(ID)
);

CREATE TABLE CANCELED (
  ORDER_ID INTEGER PRIMARY KEY,
  DATETIME TIMESTAMP NOT NULL,
  FOREIGN KEY (ORDER_ID) REFERENCES ORDER_T(ID)
);

CREATE TABLE DELIVERED (
  ORDER_ID INTEGER PRIMARY KEY,
  DATETIME TIMESTAMP NOT NULL,
  CARRIER_ID INTEGER NOT NULL,
  FOREIGN KEY (ORDER_ID) REFERENCES TRAVELING(ORDER_ID),
  FOREIGN KEY (CARRIER_ID) REFERENCES CARRIER(ID)
);

CREATE TABLE LOST (
  ORDER_ID INTEGER PRIMARY KEY,
  DATETIME TIMESTAMP NOT NULL,
  CARRIER_ID INTEGER NOT NULL,
  FOREIGN KEY (ORDER_ID) REFERENCES TRAVELING(ORDER_ID),
  FOREIGN KEY (CARRIER_ID) REFERENCES CARRIER(ID)
);


  


  
