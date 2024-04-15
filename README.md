# RPi OTP Derive Key

Small utility for deriving decryption keys from a key in OTP on a Raspberry Pi.

Intended for use with cryptsetup.

For device-unique keys, you can optionally use the board serial as salt.

```
Syntax: rpiotpdk -h | [-s] [-H] [-c count] <-p|-u> <-l label>
Where:
  -h    Show this help
  -s    Use the board serial as salt (zero salt otherwise)
  -H    Output derived key in hex format rather than raw binary
  -c  <count> Derive a key of length <count> bytes (default 32)
  -p    Use the device Private key in OTP as key (not always supported)
  -u    Use the User OTP data as key
  -l <label>  Derive key for label <label>
```

## Examples:

Derive a key for the label "rootfs" which is common across devices as long as
the same key has been loaded into the user OTP area:

```
rpiotpdk -u -l rootfs
```

Corresponding OpenSSL commandline (substitute hexkey value as necessary):

```
openssl kdf -digest sha256 -keylen 32 -kdfopt hexkey:0000000000000000000000000000000000000000000000000000000000000000 -kdfopt info:rootfs -kdfopt hexsalt:0000000000000000 -binary hkdf
```


Derive a key for the label "localfs" which is unique (but predictable) to the
device:

```
rpiotdk -u -s -l localfs
```

Corresponding OpenSSL commandline (substitute hexsalt with the serial number
from /proc/cpuinfo, in little-endian order):

```
openssl kdf -digest sha256 -keylen 32 -kdfopt hexkey:0000000000000000000000000000000000000000000000000000000000000000 -kdfopt info:localfs -kdfopt hexsalt:a8948ead00000010 hkdf -binary
```
I.e. for the above example, `grep Serial /proc/cpuinfo | awk '{print $3}'`
gives `10000000ad8e94a8`. An automatic way of reversing it correctly is via
```
grep Serial /proc/cpuinfo  | awk '{print $3}' | grep -o .. | tac | paste -sd ''
```
