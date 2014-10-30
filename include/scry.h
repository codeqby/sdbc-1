#ifndef SCRYPT_H
#define SCRYPT_H

#include <bignum.h>
#include <crc32.h>

#ifdef __cplusplus
extern "C" {
#endif
		
char * prikey128(char *keybuf,u_int ind[4],u_int *family);
extern u_int family[];
int get_mac(char* out);
int fingerprint(char *out);

#ifdef __cplusplus
}
#endif


#endif

