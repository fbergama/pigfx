// Thanks to https://github.com/leiradel/barebones-rpi

#ifndef MBOX_H__
#define MBOX_H__

#include "peri.h"
#include <stdint.h>

/*
From [Mailbox property interface](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface)
Buffer contents:
* u32: buffer size in bytes (including the header values, the end tag and padding)
* u32: buffer request/response code
  * Request codes:
    * 0x00000000: process request
    * All other values reserved
  * Response codes:
    * 0x80000000: request successful
    * 0x80000001: error parsing request buffer (partial response)
    * All other values reserved
* u8...: sequence of concatenated tags
* u32: 0x0 (end tag)
* u8...: padding
Tag format:
* u32: tag identifier
* u32: value buffer size in bytes
* u32:
  * Request codes:
    * b31 clear: request
    * b30-b0: reserved
  * Response codes:
    * b31 set: response
    * b30-b0: value length in bytes
* u8...: value buffer
* u8...: padding to align the tag to 32 bits.
*/

typedef struct {
  uint32_t size; /* Total size of the message in bytes. */
  uint32_t code; /* Set to 0 before sending the message. */
}
mbox_msgheader_t;

typedef struct {
  uint32_t id;   /* The function to execute. */
  uint32_t size; /* The size of the value buffer in bytes. */
  uint32_t code; /* The size of the request in bytes. */
}
mbox_tagheader_t;

typedef struct {
  uint32_t end; /* Set to 0 before sending the message. */
}
mbox_msgfooter_t;

extern uint32_t mem_arm2vc(const uint32_t address);

extern uint32_t mem_vc2arm(const uint32_t address);

int mbox_send(void* message);

#endif /* MBOX_H__ */ 
