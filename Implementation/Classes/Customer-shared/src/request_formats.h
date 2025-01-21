#ifndef REQUEST_FORMATS
#define REQUEST_FORMATS

namespace req_formats {

/**
 * Forms of request and response:
 * request: type {req_type} usr_state {usr_state} customerID {id} 
 *          {prod_id} {prod_qty} [{prod_id} {prod_qty} ...].
 * response: type {req_type} customerID {id} orderID {ord_id} 
 *           code {status_code} msg {msg_body}
 * Each enum value corresponds to their index in the REDIS_ARRAY_REPLY
 */
enum ORDER_REQ_FORMAT {
    OREQ_CUST_STATE_F = 2,
    OREQ_CUST_STATE_V = 3,
    OREQ_CUST_ID_F = 4, 
    OREQ_CUST_ID_V = 5
    // the rest are indices for prod_id and prod_qty
};

enum ORDER_REPLY_FORMAT {
    OREP_CUST_ID_F = 2,
    OREP_CUST_ID_V = 3,
    OREP_ORD_ID_F = 4,
    OREP_ORD_ID_V = 5,
    OREP_CODE_F = 6,
    OREP_CODE_V = 7,
    OREP_MSG_F = 8,
    OREP_MSG_V = 9
};

/**
 * Forms of request and response:
 * request: type {req_type} usr_state {usr_state} customerID {id} 
 *          orderID {ord_id}
 * response: type {req_type} customerID {id} orderID {ord_id} 
 *           code {status_code} msg {msg_body}
 * Each enum value corresponds to their index in the REDIS_ARRAY_REPLY
 */
enum CANC_ORDER_REQ_FORMAT {
    COREQ_CUST_STATE_F = 2,
    COREQ_CUST_STATE_V = 3,
    COREQ_CUST_ID_F = 4, 
    COREQ_CUST_ID_V = 5,
    COREQ_ORD_ID_F = 6,
    COREQ_ORD_ID_V = 7
};

enum CANC_ORDER_REPLY_FORMAT {
    COREP_CUST_ID_F = 2,
    COREP_CUST_ID_V = 3,
    COREP_ORD_ID_F = 4,
    COREP_ORD_ID_V = 5,
    COREP_CODE_F = 6,
    COREP_CODE_V = 7,
    COREP_MSG_F = 8,
    COREP_MSG_V = 9
};

/**
 * Forms of request and response:
 * request: type {req_type} n_prod {number_products_requested}
 * response: type {req_type} code {status_code} n_prod
 *      {number_products_requested} msg {msg} 
 *      prodsRcvd [(prod_id, prod_qty), ...]
 * Each enum value corresponds to their index in the REDIS_ARRAY_REPLY
 */
enum GET_PRODS_REQ_FORMAT {
    GPREQ_N_PROD_F = 2,
    GPREQ_N_PROD_V = 3
};

enum GET_PRODS_REPLY_FORMAT {
    GPREP_CODE_F = 2,
    GPREP_CODE_V = 3,
    GPREP_N_PROD_F = 4,
    GPREP_N_PROD_V = 5,
    GPREP_MSG_F = 6,
    GPREP_MSG_V = 7,
    GPREP_PRODS_RCVD_F = 8,
    GPREP_PRODS_RCVD_V = 9
    // the rest are indices for prod_id and prod_qty
};

/**
 * Forms of request and response:
 * request: type {req_type} cust_qty {qty} 
 * response: type {req_type} code {status_code} regIDs {[x1, ..., xn]} msg {msg}
 * [x1, ..., xn] is a string representing an array of unsigned long.
 */
enum REGISTER_CUST_REQ_FORMAT {
    RCREQ_CUST_QTY_F = 2,
    RCREQ_CUST_QTY_V = 3
};

enum REGISTER_CUST_REPLY_FORMAT {
    RCREP_CODE_F = 2,
    RCREP_CODE_V = 3,
    RCREP_REG_IDS_F = 4,
    RCREP_REG_IDS_V = 5,
    RCREP_MSG_F = 6,
    RCREP_MSG_V = 7
};

} // req_formats

#endif
