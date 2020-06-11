#ifndef SPLIT_KEYBOARD_UTIL_H
#define SPLIT_KEYBOARD_UTIL_H

#include <stdbool.h>
#include "eeconfig.h"

#define follower_I2C_ADDRESS           0x32

extern volatile bool isLeftHand;

// follower version of matix scan, defined in matrix.c
void matrix_follower_scan(void);

void split_keyboard_setup(void);
bool is_helix_master(void);

void matrix_master_OLED_init (void);

#endif
