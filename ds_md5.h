/* 
 * Header file for MD% hashing functionlity for Digital-Paper/Digital Scatter by Jeremy Loulou.
 *
 * */


#ifndef DS_MD5_H
#define DS_MD5_H

#include <openssl/md5.h>
#include <stdint.h>


unsigned char* hashFile(uint8_t* bytes, long filesize);

char* hashString(unsigned char* hash);

#endif //DS_MD5_H
