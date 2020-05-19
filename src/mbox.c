// Thanks to https://github.com/leiradel/barebones-rpi

#include "mbox.h"
#include "utils.h"
#include "console.h"

#include <stdint.h>

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
  while ((value & MAIL_FULL) != 0); // Mailbox full, retry.

  // Send message to channel 8: tags (ARM to VC).
  const uint32_t msgaddr = (mem_arm2vc((uint32_t)msg) & ~15) | MB_CHANNEL_TAGS;
  dmb();
  W32(MBOX_WRITE1, msgaddr);

  // Wait for the response.
  do {
    do {
      value = R32(MBOX_STATUS0);
    }
    while ((value & MAIL_EMPTY) != 0); // Mailbox empty, retry.
    value = R32(MBOX_READ0);
    dmb();
  }
  while ((value & 15) != MB_CHANNEL_TAGS); // Wrong channel, retry.

  if ((((mbox_msgheader_t*)msg)->code & MAIL_FULL) != 0) {
    return 0; // Success!
  }

  return -1; // Ugh...
} 

