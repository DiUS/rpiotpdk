#ifndef RPIVC_H
#define RPIVC_H

#include <stdint.h>
#include <stdbool.h>

typedef struct otp_data_t
{
  union {
    uint8_t u8[32];
    uint32_t u32[8];
  } data;
} otp_data_t;

bool get_board_serial(uint64_t *serial);
bool get_otp_priv_key(otp_data_t *key);
bool get_user_otp(otp_data_t *data);

#endif
