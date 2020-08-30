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
  msg.tag.id = MAILBOX_TAG_GET_BOARD_REVISION; // Get board revision.
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
  msg.tag.id = MAILBOX_TAG_GET_VERSION; // Get firmware revision.
  msg.tag.size = sizeof(msg.value);
  msg.tag.code = 0;
  msg.footer.end = 0;

  if (mbox_send(&msg) != 0) {
    return 0;
  }

  return msg.value.response.firmware;
}

int prop_macaddr(unsigned char* pOutAddr) {
  unsigned char off;
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
  msg.tag.id = MAILBOX_TAG_GET_BOARD_MAC_ADDRESS; // Get board MAC address.
  msg.tag.size = sizeof(msg.value);
  msg.tag.code = 0;
  msg.footer.end = 0;

  if (mbox_send(&msg) != 0) {
    return 0;
  }

  for( off=0; off<6; ++off )
  {
      pOutAddr[off] = msg.value.response.addr[off];
  }
  return 1;
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
  msg.tag.id = MAILBOX_TAG_GET_BOARD_SERIAL; // Get board serial.
  msg.tag.size = sizeof(msg.value);
  msg.tag.code = 0;
  msg.footer.end = 0;

  if (mbox_send(&msg) != 0) {
    return 0;
  }

  return msg.value.response.serial;
}

void prop_VCRAM(tSysRam* ram)
{
  typedef struct {
    mbox_msgheader_t header;
    mbox_tagheader_t tag;

    union {
      // No request.
      struct {
        uint32_t base;
        uint32_t vcRam;
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
  msg.tag.id = MAILBOX_TAG_GET_VC_MEMORY; // Get VC Memory.
  msg.tag.size = sizeof(msg.value);
  msg.tag.code = 0;
  msg.footer.end = 0;

  mbox_send(&msg);

  ram->baseAddr = msg.value.response.base;
  ram->size = msg.value.response.vcRam / (1024*1024);
}

void prop_ARMRAM(tSysRam* ram)
{
  typedef struct {
    mbox_msgheader_t header;
    mbox_tagheader_t tag;

    union {
      // No request.
      struct {
        uint32_t base;
        uint32_t armRam;
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
  msg.tag.id = MAILBOX_TAG_GET_ARM_MEMORY; // Get ARM Memory.
  msg.tag.size = sizeof(msg.value);
  msg.tag.code = 0;
  msg.footer.end = 0;

  mbox_send(&msg);

  ram->baseAddr = msg.value.response.base;
  ram->size = msg.value.response.armRam / (1024*1024);
}
