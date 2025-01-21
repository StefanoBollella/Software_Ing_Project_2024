#ifndef REQUEST_FORMATS
#define REQUEST_FORMATS

namespace req_formats {

/*
the numbers are the index of the object in the redis message. F = field; V = value; 
*/

/**
 * 
 * request: type - carrierID - orderID -
 * reply: type - carrierID - orderID - code - msg -
 *
 *
 */
enum DELIVERY_REQ_FORMAT {
    DREQ_CARR_ID_F = 2,
    DREQ_CARR_ID_V = 3,
    DREQ_ORD_ID_F = 4,
    DREQ_ORD_ID_V = 5
};

enum DELIVERY_REPLY_FORMAT {
    DREP_CARR_ID_F = 2,
    DREP_CARR_ID_V = 3,
    DREP_ORD_ID_F = 4,
    DREP_ORD_ID_V = 5,
    DREP_CODE_F = 6,
    DREP_CODE_V = 7,
    DREP_MSG_F = 8,
    DREP_MSG_V = 9
};

/**
 * 
 * request: type - carrierID - orderID -
 * reply: type - carrierID - orderID - code - msg -
 *
 *
 */
enum LOSE_REQ_FORMAT {
    LREQ_CARR_ID_F = 2,
    LREQ_CARR_ID_V = 3,
    LREQ_ORD_ID_F = 4,
    LREQ_ORD_ID_V = 5
};

enum LOSE_REPLY_FORMAT {
    LREP_CARR_ID_F = 2,
    LREP_CARR_ID_V = 3,
    LREP_ORD_ID_F = 4,
    LREP_ORD_ID_V = 5,
    LREP_CODE_F = 6,
    LREP_CODE_V = 7,
    LREP_MSG_F = 8,
    LREP_MSG_V = 9
};

/**
 * 
 * request: type - id - quantity -
 * reply: type - id - numords - code - msg - ordids - 
 *
 *
 */
enum GETORD_REQ_FORMAT {
    GORDREQ_CARR_ID_F = 2, 
    GORDREQ_CARR_ID_V = 3,
    GORDREQ_ORD_QTY_F = 4,
    GORDREQ_ORD_QTY_V = 5

};

enum GETORD_REPLY_FORMAT {
    GORDREP_CARR_ID_F = 2,
    GORDREP_CARR_ID_V = 3,
    GORDREP_N_ORD_F = 4,
    GORDREP_N_ORD_V = 5,
    GORDREP_CODE_F = 6,
    GORDREP_CODE_V = 7,
    GORDREP_MSG_F = 8,
    GORDREP_MSG_V = 9,
    GORDREP_IDS_F = 10,
    GORDREP_IDS_V = 11
};

/**
 * 
 * request: type - quantity -
 * reply: type - code - ids - msg -
 *
 *
 */
enum REG_CARRIER_REQ_FORMAT {
    RCARREQ_QTY_F = 2,
    RCARREQ_QTY_V = 3
};

enum REG_CARRIER_REPLY_FORMAT {
    RCARREP_CODE_F = 2,
    RCARREP_CODE_V = 3,
    RCARREP_REG_IDS_F = 4,
    RCARREP_REG_IDS_V = 5,
    RCARREP_MSG_F = 6,
    RCARREP_MSG_V = 7
};


} // req_formats

#endif
