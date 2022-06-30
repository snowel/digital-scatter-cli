#ifndef DS_MD5_C
#define DS_MD5_C

#include "ds_md5.h"

unsigned char* hashFile(uint8_t* bytes, long filesize){
		  uint8_t* new = malloc(MD5_DIGEST_LENGTH * sizeof(uint8_t));
		  if(new == NULL) return NULL;
		  MD5(bytes, filesize, new);

		  return new;

		  //TODO When reading from a pointer create by hashFile, some values ptr[0] for example, result in number far larger than 255(ff), yet if ig cast (uint8_t)ptr[0] it works as expected?
}			//But I only got this problem when I was casting my pointer to a char* from an unsigned char*... so it's on me


char* hashString(unsigned char* hash){

		  char* string = malloc(sizeof(char) * 33);
		  if(string == NULL) return NULL;
		  string[33] = '\0';

		  char* singleByte; 
		  int byteOffset, secondCharOffset, i;
		  long nthByte;
		  for(i = 0; i < 16; i++){
					 nthByte = (long)hash[i];
					 singleByte = decimalToHexString(nthByte);

					 byteOffset = i * 2;
					 secondCharOffset = byteOffset + 1;
					 
					 if(strlen(singleByte) == 2){
								string[byteOffset] = singleByte[0];
								string[secondCharOffset] = singleByte[1];
					 } else {
								string[byteOffset] = singleByte[0];
								string[secondCharOffset] = '0';
					 }

					 free(singleByte);

					 //strcat(string, singleByte);
		  }


		  return string;
}
#endif // DS_MD5_C
