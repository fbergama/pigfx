// Thanks to https://github.com/leiradel/barebones-rpi

#include "mbox.h"
#include "utils.h"
#include "console.h"

#include <stdint.h>

#define TAGS 8

uint32_t mem_arm2vc(const uint32_t address)
{
  return address | BUSALIAS;
}

uint32_t mem_vc2arm(const uint32_t address)
{
  return address & ~BUSALIAS;
}

int mbox_send(void* msg) {
  uint32_t value;

  // Write message to mailbox.
  do {
    value = R32(MBOX_STATUS1);
  }
  while ((value & UINT32_C(0x80000000)) != 0); // Mailbox full, retry.

  // Send message to channel 8: tags (ARM to VC).
  const uint32_t msgaddr = (mem_arm2vc((uint32_t)msg) & ~15) | TAGS;
  dmb();
  W32(MBOX_WRITE1, msgaddr);

  // Wait for the response.
  do {
    do {
      value = R32(MBOX_STATUS0);
    }
    while ((value & UINT32_C(0x40000000)) != 0); // Mailbox empty, retry.
    value = R32(MBOX_READ0);
    dmb();
  }
  while ((value & 15) != TAGS); // Wrong channel, retry.

  if (((mbox_msgheader_t*)msg)->code == UINT32_C(0x80000000)) {
    return 0; // Success!
  }
  
  return -1; // Ugh...
} 

void set_130(const int on) {
  typedef struct {
    mbox_msgheader_t header;
    mbox_tagheader_t tag;
    
    union {
      struct {
        uint32_t pin;
        uint32_t value;
      }
      request;
      // No response.
    }
    value;

    mbox_msgfooter_t footer;
  }
  message_t;

  message_t msg __attribute__((aligned(16)));

  msg.header.size = sizeof(msg);
  msg.header.code = 0;
  msg.tag.id = UINT32_C(0x00038041);
  msg.tag.size = sizeof(msg.value);
  msg.tag.code = 0;
  msg.value.request.pin = 130;
  msg.value.request.value = !!on;
  msg.footer.end = 0;

  mbox_send(&msg);
}
