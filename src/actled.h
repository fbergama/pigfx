#ifndef ACTLED_H__
#define ACTLED_H__

extern void (*led_set)(const int on);

int led_init(board_t board);

#endif /* ACTLED_H__ */ 
