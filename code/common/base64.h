#ifndef BASE64_H_INCLUDED
#define BASE64_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char encoding_table[] = {
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
'w', 'x', 'y', 'z', '0', '1', '2', '3',
'4', '5', '6', '7', '8', '9', '+', '/'
};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

// input XML file which want to transform, write base64 code string to file which file descriptor is fd, return encode data length, if ERROR return -1
int base64_encode(char* filename, FILE* fd);

// input data string, input data length, output data length, return decode data string
unsigned char* base64_decode(const char* data, int inlen, int *outlen);

void build_decoding_table();
void base64_cleanup();

#endif // BASE64_H_INCLUDED
