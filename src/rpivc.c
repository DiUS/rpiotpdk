// SPDX-License-Identifier: MIT
#include "rpivc.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

// Reference:
//   https://github.com/raspberrypi/documentation/blob/master/documentation/asciidoc/computers/raspberry-pi/raspberry-pi-industrial.adoc

#define VC_RESP_LEN 7

static const uint32_t otp_cmd[] = { 0x40, 0, 0x30021, 0x28, 0x28, 0, 8 };
static const uint32_t key_cmd[] = { 0x40, 0, 0x30081, 0x28, 0x28, 0, 8 };
static const uint32_t ser_cmd[] = { 0x28, 0, 0x10004, 0x10 };

static bool issue_vc_request(uint32_t *buf)
{
  bool ret = false;

  int fd = open("/dev/vcio", 0);
  if (fd == -1)
    goto out;
  if ((ioctl(fd, _IOWR(100, 0, char *), buf) != 0) ||
      buf[1] != 0x80000000)
    goto out;

  ret = true;
out:
  close(fd);
  return ret;
}


bool get_board_serial(uint64_t *serial)
{
  uint32_t buf[VC_RESP_LEN];
  memcpy(buf, ser_cmd, sizeof(ser_cmd));
  if (!issue_vc_request(buf))
    return false;

  *serial = (((uint64_t)buf[6]) << 32) | buf[5];

  return true;
}


bool get_otp_priv_key(otp_data_t *key)
{
  uint32_t buf[VC_RESP_LEN + 8];
  memcpy(buf, key_cmd, sizeof(key_cmd));
  if (!issue_vc_request(buf))
    return false;

  for (unsigned i = 0; i < 8; ++i)
    key->data.u32[i] = buf[7+i];

  return true;
}


bool get_user_otp(otp_data_t *data)
{
  uint32_t buf[VC_RESP_LEN + 8];
  memcpy(buf, otp_cmd, sizeof(otp_cmd));
  if (!issue_vc_request(buf))
    return false;

  for (unsigned i = 0; i < 8; ++i)
    data->data.u32[i] = buf[7+i];

  return true;
}
