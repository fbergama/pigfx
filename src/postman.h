#ifndef _POSTMAN_H_
#define _POSTMAN_H_



typedef enum {
POSTMAN_SUCCESS      =0xC0,
POSTMAN_SEND_TIMEOUT =0xC1,
POSTMAN_RECV_TIMEOUT =0xC2,
POSTMAN_BAD_DATA     =0xC3,
POSTMAN_TOO_MANY_MSG =0xC4
} POSTMAN_RETURN_TYPE;


#define MAILBOX_WAIT_TIMEOUT    500000
#define MAILBOX_MAX_MSG_TO_SKIP 20


extern POSTMAN_RETURN_TYPE postman_send( unsigned int channel, unsigned int data );
extern POSTMAN_RETURN_TYPE postman_recv( unsigned int channel, unsigned int* out_data );


#endif
