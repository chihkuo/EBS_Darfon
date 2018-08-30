#ifndef KEY_H
#define KEY_H

#include "md5.h"
#include "sha1.h"

struct amit_wifi_default 
{
    unsigned char mac[6];
    unsigned char ssid_key[8];
    unsigned char auth_key[16];
    unsigned char ssid_repeat_index;
    unsigned char auth_repeat_index;
    struct amit_wifi_default *prev;
    struct amit_wifi_default *next;
};

typedef struct amit_wifi_default KEY_TYPE;
#endif
