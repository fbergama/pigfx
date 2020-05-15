// https://github.com/leiradel/barebones-rpi

#include "prop.h"
#include "mbox.h"

#include <stdint.h>

uint32_t prop_revision(void) {
  typedef struct {
    mbox_msgheader_t header;
    mbox_tagheader_t tag;
    
    union {
      // No request.
      struct {
        uint32_t revision;
      }
      response;
    }
    value;

    mbox_msgfooter_t footer;
  }
  message_t;

  message_t msg __attribute__((aligned(16)));

  msg.header.size = sizeof(msg);
  msg.header.code = 0;
  msg.tag.id = UINT32_C(0x00010002); // Get board revision.
  msg.tag.size = sizeof(msg.value);
  msg.tag.code = 0;
  msg.footer.end = 0;

  if (mbox_send(&msg) != 0) {
    return 0;
  }

  return msg.value.response.revision;
}

uint32_t prop_fwrev(void) {
  typedef struct {
    mbox_msgheader_t header;
    mbox_tagheader_t tag;
    
    union {
      // No request.
      struct {
        uint32_t firmware;
      }
      response;
    }
    value;

    mbox_msgfooter_t footer;
  }
  message_t;

  message_t msg __attribute__((aligned(16)));

  msg.header.size = sizeof(msg);
  msg.header.code = 0;
  msg.tag.id = UINT32_C(0x00000001); // Get firmware revision.
  msg.tag.size = sizeof(msg.value);
  msg.tag.code = 0;
  msg.footer.end = 0;

  if (mbox_send(&msg) != 0) {
    return 0;
  }
  
  return msg.value.response.firmware;
}

uint64_t prop_macaddr(void) {
  typedef struct {
    mbox_msgheader_t header;
    mbox_tagheader_t tag;
    
    union {
      // No request.
      struct {
        uint8_t addr[6];
        uint8_t pad[2];
      }
      response;
    }
    value;

    mbox_msgfooter_t footer;
  }
  message_t;

  message_t msg __attribute__((aligned(16)));

  msg.header.size = sizeof(msg);
  msg.header.code = 0;
  msg.tag.id = UINT32_C(0x00010003); // Get board MAC address.
  msg.tag.size = sizeof(msg.value);
  msg.tag.code = 0;
  msg.footer.end = 0;

  if (mbox_send(&msg) != 0) {
    return 0;
  }

  return (uint64_t)msg.value.response.addr[0] << 40
       | (uint64_t)msg.value.response.addr[1] << 32
       | (uint64_t)msg.value.response.addr[2] << 24
       | (uint64_t)msg.value.response.addr[3] << 16
       | (uint64_t)msg.value.response.addr[4] <<  8
       | (uint64_t)msg.value.response.addr[5];
}

uint64_t prop_serial(void) {
  typedef struct {
    mbox_msgheader_t header;
    mbox_tagheader_t tag;
    
    union {
      // No request.
      struct {
        uint64_t serial;
      }
      response;
    }
    value;

    mbox_msgfooter_t footer;
  }
  message_t;

  message_t msg __attribute__((aligned(16)));

  msg.header.size = sizeof(msg);
  msg.header.code = 0;
  msg.tag.id = UINT32_C(0x00010004); // Get board serial.
  msg.tag.size = sizeof(msg.value);
  msg.tag.code = 0;
  msg.footer.end = 0;

  if (mbox_send(&msg) != 0) {
    return 0;
  }

  return msg.value.response.serial;
}
