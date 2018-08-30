#include "base64.h"

void build_decoding_table() {

    int i = 0;

    decoding_table = (char*) malloc(256);
    memset(decoding_table, 0x00, 256);

    for (i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
    //for (i = 0; i < 256; i++)
    //    printf("decoding_table[%d] = %d\n", i, decoding_table[i]);
}

void base64_cleanup() {
    if ( decoding_table )
        free(decoding_table);
}

int base64_encode(char *filename, FILE *fd)
{
    FILE *pfd = NULL;
    char inbuf[3072] = {0};
    char outbuf[4096] = {0};
    int len = 0, i = 0, j = 0, outlen = 0;

    printf("======== base64_encode start ========\n");
    pfd = fopen(filename, "rb");
    if ( pfd == NULL ) {
        printf("#### base64_encode Open %s Fail ####\n", filename);
        return -1;
    }

    while (1) {
        len = fread (inbuf, 1, 3072, pfd);
        if ( len > 0 ) {
            outlen = 4 * ((len + 2) / 3);
            printf("len = %d, outlen = %d\n", len, outlen);
            //printf("inbuf = \n%s\n", inbuf);

            for (i = 0, j = 0; i < len; ) {
                unsigned int octet_a = i < len ? (unsigned char)inbuf[i++] : 0;
                unsigned int octet_b = i < len ? (unsigned char)inbuf[i++] : 0;
                unsigned int octet_c = i < len ? (unsigned char)inbuf[i++] : 0;

                unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

                outbuf[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
                outbuf[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
                outbuf[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
                outbuf[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
            }

            for (i = 0; i < mod_table[len % 3]; i++)
                outbuf[outlen - 1 - i] = '=';

            fwrite(outbuf , sizeof(char), outlen, fd);
            //printf("base64_encode : \n%s\n", outbuf);

        } else {
            break;
        }

        if (feof(pfd))
            break;
    }
    fclose(pfd);

    printf("========= base64_encode end =========\n");
    return outlen;
}

unsigned char* base64_decode(const char* data, int inlen, int *outlen)
{
    unsigned char *decoded_data = NULL;
    int i = 0, j = 0;

    printf("======== base64_decode start ========\n");
    if ( decoding_table == NULL )
        build_decoding_table();

    if ( inlen%4 != 0 ){
        *outlen = 0;
        printf("Input data length error!\n");
        return NULL;
    }

    *outlen = inlen/4 * 3;
    if ( data[inlen - 1] == '=' )
        (*outlen)--;
    if ( data[inlen - 2] == '=' )
        (*outlen)--;

    decoded_data = (unsigned char*)malloc(*outlen + 1);
    memset(decoded_data, 0x00, *outlen);
    if ( decoded_data == NULL )
        return NULL;

    for ( i = 0, j = 0; i < inlen; ) {
        unsigned int sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
        unsigned int sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
        unsigned int sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
        unsigned int sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];

        unsigned int triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if (j < *outlen)
            decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *outlen)
            decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *outlen)
            decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }
    decoded_data[*outlen] = 0;
    //printf("base64_decode : \n%s\n", decoded_data);

    printf("========= base64_decode end =========\n");
    return decoded_data;
}
