// SPDX-License-Identifier: MIT
#include "rpivc.h"
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/params.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void error(const char *msg)
{
  fprintf(stderr, "Error: %s\n", msg);
  exit(1);
}


static void syntax(const char *app)
{
  fprintf(stderr,
    "Syntax: %s -h | [-s] [-H] [-c count] <-p|-u> <-l label>\n"
    "Where:\n"
    "  -h    Show this help\n"
    "  -s    Use the board serial as salt (zero salt otherwise)\n"
    "  -H    Output derived key in hex format rather than raw binary\n"
    "  -c  <count> Derive a key of length <count> bytes (default 32)\n"
    "  -p    Use the device Private key in OTP as key (not always supported)\n"
    "  -u    Use the User OTP data as key\n"
    "  -l <label>  Derive key for label <label>\n"
    "\n"
  , app);
  exit(1);
}


// Reference:
//   https://www.openssl.org/docs/man3.0/man7/EVP_KDF-HKDF.html

static void derive_key(
  const uint8_t *key256, uint64_t salt, const char *label,
  uint8_t *out, unsigned out_sz)
{
  EVP_KDF *kdf = EVP_KDF_fetch(NULL, "HKDF", NULL);
  EVP_KDF_CTX *kctx = EVP_KDF_CTX_new(kdf);
  EVP_KDF_free(kdf);

  OSSL_PARAM params[] = {
    OSSL_PARAM_construct_utf8_string(
      OSSL_KDF_PARAM_DIGEST, SN_sha256, strlen(SN_sha256)),
     OSSL_PARAM_construct_octet_string(
       OSSL_KDF_PARAM_KEY, (uint8_t*)key256, 32),
     OSSL_PARAM_construct_octet_string(
       OSSL_KDF_PARAM_INFO, (char *)label, strlen(label)),
     OSSL_PARAM_construct_octet_string(
       OSSL_KDF_PARAM_SALT, &salt, sizeof(salt)),
     OSSL_PARAM_construct_end(),
  };

  if (EVP_KDF_derive(kctx, out, out_sz, params) <= 0)
    error("EVP_KDF_derive failed");

  EVP_KDF_CTX_free(kctx);
}


int main(int argc, char *argv[])
{
  int opt;
  bool use_priv_key = false;
  bool use_user_otp = false;
  bool use_salt = false;
  const char *label = NULL;
  bool hex_output = false;
  unsigned dk_size = 32;

  while ((opt = getopt(argc, argv, "c:hl:psuH")) != -1)
  {
    switch (opt) {
      case 'c': dk_size = atoi(optarg); break;
      case 'h': syntax(argv[0]); break;
      case 'l': label = optarg; break;
      case 'p': use_priv_key = true; break;
      case 's': use_salt = true; break;
      case 'u': use_user_otp = true; break;
      case 'H': hex_output = true; break;
      default:
        syntax(argv[0]);
    }
  }

  if (!use_priv_key && !use_user_otp)
    error("neither -p nor -u specified");
  if (!label)
    error("no label specified");
  if (dk_size > 1024)
    error("excessively large key size");

  uint64_t salt = 0;
  if (use_salt)
  {
    if (!get_board_serial(&salt))
      error("failed to read board serial");
  }

  otp_data_t key;
  if (use_priv_key)
  {
    if (!get_otp_priv_key(&key))
      error("failed to read device private key");
  }
  else if (use_user_otp)
  {
    if (!get_user_otp(&key))
      error("failed to read user OTP key");
  }

  uint8_t *derived_key = malloc(dk_size);
  if (!derived_key)
    error("out of memory");

  derive_key(key.data.u8, salt, label, derived_key, dk_size);

  if (hex_output)
  {
    for (unsigned i = 0; i < dk_size; ++i)
      printf("%02x", derived_key[i]);
    printf("\n");
  }
  else
    write(fileno(stdout), derived_key, dk_size);

  free(derived_key);

  return 0;
}
