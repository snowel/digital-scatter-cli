
/*
 * Header file for reading and struct opperations.
 */

#ifndef READ_C
#define READ_C

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

//#include "qrcodegen.h"
//#include "qrcodegen.c"
#include "ds_md5.h"


#define RESULT_ERROR -1
#define BYTE_SIZE sizeof(uint8_t)
#define HEADER_STOP 0xab
#define M0_HEADER_DEFAULT 41

typedef struct {
		  long pages; // total number of pages needed for that file
		  uint32_t bytesPerPage;
		  uint32_t bytesOnLastPage;
		  const uint8_t* bytePages; //pointer tot he array of arrays of byte data in order of pages
		  char hash[33]; //TODO do I need a character end? can't I literally have a pure array of hash?

}databook;


/*---File handling---*/

void writeBinFile(uint8_t* bytes, uint32_t length, char* filename);
//Writes a binary file with the specified bytes

unsigned long lengthOfFile(FILE* file);
//POSIX compliant and not will need to be reviewed

long fs_POSIX(const char * filename);
//Get filesize for POSIX compliant systems

uint8_t* readBin(const char * filename, long filesize);
//returns a malloced array of the file's bytes

bool fileFrac(FILE* fl, databook* fracture);

void printBin(const char* filename);

char* generateFilename(char* ofilename, uint32_t page);

/*---Handlling page (size, number of pages) ---*/

int computePages(long pageWidth, long filesize, uint8_t* bytes, databook * tome);
//Page width is bytesPerPage

uint8_t* nthPage(databook* tome, unsigned int page);
//Return pointer to the first element in the databook array


/*---Handling the pagination information for the header.---*/

uint8_t* longTo4Byte(long num);
//creates an array or

long bytesToLong(uint8_t* num);

int countDigitis(long num);
//counts the digits in a decimal number

int countHexDigits(long num);
//Counts the digits in a hexadecimal number

char singleHex(int num);
//Take an int between decima 0 and 15 and returns the hexadecimal char, if out of bounds it returns '\0'

char* decimalToHexString(long decimal);
//Return a string version of the input number's hexadecimal representation
// Used for non 0 modes. in a QR the header would be encoded as ALPHAANUMERICAL

uint8_t* m0_headerGen(uint8_t* generic, int page);
//Currently mutates the generic, so:
// -- Write before generating next header
// -- Free after all header have been generated
//Header for mode 0 i.e. pure binary file pages
//create a header in the from of a string for a given page of a dataset
//Using the follwoing moddel:
/*
 *	bytes 0-31 : hash
 *	bytes 32-35 : uint32_t - page
 *	bytes 36-39 : uint32_t - total pages (4.2b pages max)
 * byte 40 : stopper (0xab)
 * */

uint8_t* m0_genericHeaderGen(databook* tome, uint8_t* hash);
//Header generator for mode 0, indiferent to the page its for

/*---Mode 0 create files---*/

bool m0_singleFile(databook* tome, uint32_t page, char* filename, uint8_t* header, uint32_t headerLen);
//Writes a single page

bool m0_deconstruct(char* filename, uint32_t pageWidth);
//deconstrusts a file and write a number of files to that dirrectory accordign to the pageWidth


/* -- code for uniform size peices, with a buffer in the ehader of the fila file
uint8_t* m0_finalHeaderGen(databook* tome, uint8_t* generic);
// allocated header for the final page

uint32_t m0_finalHeaderLen(databook* tome);
//Returns the byte length of the final pages header

uint32_t m0_finalHeaderLen(databook* tome){
		  uint32_t buffer = tome->bytesPerPage - bytesOnLastPage; // the number of bytes needed to keep the data per page the same
		  uint32_t fheaderLen = buffer + 41; // the length of the final page header, the standard 41 bytes plus the lenght of the buffer
		  
		  return fHeaderLen;
}

uint8_t* m0_finalHeaderGen(databook* tome, uint8_t* generic){
		  uint32_t fpage = tome->pages; // the last page number
		  uint32_t fheaderLen = m0_finalHeaderLen(tome);

		  // set the generic buffer to the right page numbers
		  m0_headerGen(generic, fpage);

		  // alloc the final header pointer
		  uint8_t* fheader = malloc(fheaderLen);

		  for(int i = 0; i < 40; i++){
					 fheader[i] = generic[i];
		  }

		  // fill the padding with anyything but the ehader stopper
		  for(int i = 40; int len = fheaderLen - 1; i < len; i++){
					 do{
								fheader[i] = ((fheader[i]++ + i) * 34) % 255;
					 }
					 while (fheader[i] = HEADER_STOP);
		  }

		  fheader[fheaderLen - 1] = HEADER_STOP;

		  return fheader;
}

bool m0_singleFile(databook* tome, uint32_t page, char* filename, uint8_t* header, uint32_t headerLen){
		  if(page > tome->pages) return false;

		  uint32_t bytesLen;
		  if(page == tome->pages && tome->bytesOnLastPage != 0){ // for the final page
					 bytesLen = tome->bytesOnLastPage;
		 }	else { // For any other page
					 bytesLen = tome->bytesPerPage;
		 }

		  // create the byte array
		  uint8_t* arr = malloc(bytesLen + headerLen);

		  // assigned the bytes for the header
		  for(uint32_t i = 0; i < headerLen; i++){
					arr[i] = header[i]; 
		  }

		  uint8_t* pageBytes = nthPage(tome, page);
		  uint32_t totalLen = headerLen + bytesLen;

		  for(uint32_t i = headerLen; i < totalLen; i++){
					 arr[i] = pageBytes[i];
		  }

		  writeBinFile(arr, totalLen);

		  // clean up
		  free(arr);

		  return true;
}

bool m0_deconstruct(char* filename, uint32_t pageWidth){
		  
		  long filesize = fs_POSIX(filename); //TODO addapt the OS agnostic version
		  uint8_t* fileBytes = readBin(filename); // must be freed
		  uint8_t* fileHash = hashFile(fileBytes, filesize);
		  
		  databook* tome = malloc(sizeof(databook));
		  computePages(pageWidth, filesize, fileBytes, tome);
		  uint8_t* genericHead = m0_genericHeaderGen(tome, fileHash);
		  long numPages = tome->pages;

		  // pre-proces the final header
		  uint8_t* fheader = m0_finalHeaderGen(tome, gneericHead);
		  uint32_t fheaderLen = m0_finalHeaderLen(tome);

		  uint8_t* fragmentHeader;
		  char* fragmentName;
		  long fragmentHeaderLen;
		  for(long i = 1; i <= numPages; i++){
					 // create the spcific header
					 // NOTE fragment buffer isn't freed as the generic header is reused
					 if(i == numPages){
								fragmentHeader = fheader;
								fragmentHeaderLen = fheaderLen;
					 } else {
								fragmentHeader = m0_headerGen(genericHead, i);
								fragmentHeaderLen = M0_HEADER_DEFAULT;
					 }
					 //create the fragment buffer

					 // generate the name for that file
					 fragmentName = generateFilename(filename, i);

					 // write the file
					 m0_singleFile(tome, i, fragmentName, fragmentHeader, fragmentHeaderLen);

					 // free all pointed block involved - excepted the header info
					 free(fragmentName);
		  }

		  free(fheader);
		  free(genericHead);
}
 *
 * */

/*---Source code - Above is/was the header file---*/

uint8_t * readBin(const char * filename, long filesize){
		  //long filesize = fs_POSIX(filename); TODO YANK
		  uint8_t* arr = malloc(filesize);
		  
		  FILE* file = fopen(filename, "rb");
		  fread(arr, BYTE_SIZE, filesize, file);
		  fclose(file);

		  return arr;
}

void printBin(const char* filename){
		  
		  long filesize = fs_POSIX(filename);

		  uint8_t* bins = readBin(filename, filesize);

		  for(int i = 1; i < filesize; i++){
					 printf("%d\n", bins[i]);
		  }

		  printf("This file was: %d bytes long\n", filesize);
		  free(bins);
}

void writeBinFile(uint8_t* bytes, uint32_t length, char* filename){
		  FILE* file = fopen(filename, "wb");
		  fwrite(bytes, BYTE_SIZE, lenght, file);
		  fclose(file);
}

long fs_POSIX(const char * filename){
		  struct stat f_status;
		  if(stat(filename, &f_status) < 0){
					 return RESULT_ERROR;
		  }

		  return f_status.st_size;
}


int computePages(long pageWidth, long filesize, uint8_t* bytes, databook * tome){
		  long spillover = (filesize % pageWidth);

		  tome->bytesOnLastPage = spillover;
		  tome->bytesPerPage = pageWidth;
		  if(spillover == 0)
					 tome->pages = (filesize - spillover) / pageWidth;
		  if(spillover != 0)
					 tome->pages = (filesize - spillover) / pageWidth + 1;

		  tome->bytePages = bytes;

		  return 0;
}

uint8_t* nthPage(databook* tome, unsigned int page){
		  const uint8_t* arr = tome->bytePages;
		  unsigned int offset = (page - 1) * tome->bytesPerPage;
		  uint8_t* single = arr + offset;

		  return single;
}
/*---Handling the pagination information for the header.---*/

int countDigitis(long num){
		  int counter;
		  long mod = num;

		  while(mod > 0){
					 mod = mod - (mod % 10);
					 mod /= 10;
					 counter++;
		  }

		  return counter;
}

int countHexDigits(long num){
		  int counter = 0;
		  long mod = num;

		  while(mod > 0){
					 mod = mod - (mod % 16);
					 mod /= 16;
					 counter++;
		  }

		  return counter;
}

char singleHex(int num){
		  char result;

		  if(num >= 0 && num <= 9){
					 result = (char)(num + '0');//is this cast necessairy?
					 return result;
		  }
		  
		  if(num >= 10 && num <= 15){
					 result = (char)(num - 10 + 'A');
					 return result;
		  }

		  result = '\0';
		  return result;
}

char* decimalToHexString(long decimal){
		  int length = countHexDigits(decimal);
		  int strLength = length + 1;
		  
		  char* newstr = malloc(sizeof(char) * strLength);
		  if(newstr == NULL)
					 return NULL;
		  newstr[length] = '\0';

		  long mod = decimal;
		  int tool, backCount;
		  char singleSymbol;
		  for(int i = 0; i < length; i++){
					 tool = (int)(mod % 16);
					 mod = mod - (mod % 16);
					 mod /= 16;

					 backCount = length - 1 -i;
					 singleSymbol = singleHex(tool);
					 newstr[backCount] = singleSymbol;
		  }

		  return newstr;
}

uint8_t* longTo4Byte(long num){
		  uint8_t* new = malloc(4);
		  
		  //big-endian
		  new[3] = (uint8_t)(num & 0xff);
		  new[2] = (uint8_t)((num >> 8) & 0xff);
		  new[1] = (uint8_t)((num >> 16) & 0xff);
		  new[0] = (uint8_t)((num >> 24) & 0xff);

		  return new;
}

long bytesToLong(uint8_t* num){
		  long new = 0;
		  
		  new += num[0] 
		  new = (new << 8) + num[1]; //TODO why does: new += num [1] << 8 work?
		  new = (new << 16) + num[2];
		  new = (new << 24) + num[3];

		  return new;
}

/*---Mode 0 : pure binary files, fixed number of pages (unsigned long), different size fina page (for now, code for uniform pages is above)---*/

uint8_t* m0_genericHeaderGen(databook* tome, uint8_t* hash){
		  uint8_t* generic = malloc(41 * sizeof(uint8_t));

		  for(int i = 0; i < 32; i++){
					 generic[i] = hash[i];
		  }

		  totalPages = longTo4Byte(tome->pages);
		  
		  for(int i = 36; i < 40; i++){
					 generic[i] = totalPages[i - 36];
		  }

		  free(totalPages);

		  generic[40] = HEADER_STOP; //0xab is the stop byte, allowing for padding on the final page

		  return generic;
}


uint8_t* m0_headerGen(uint8_t* generic, uint32_t page){
		  currentPages = longTo4Byte(page);

		  for(int i = 32; i < 36; i++){
					 generic[i] = totalPages[i - 32];
		  }
		 
		  free(currentPage);

		  return generic;
}



bool m0_singleFile(databook* tome, uint32_t page, char* filename, uint8_t* header, uint32_t headerLen){
		  if(page > tome->pages) return false;

		  uint32_t bytesLen;
		  if(page == tome->pages && tome->bytesOnLastPage != 0){ // for the final page
					 bytesLen = tome->bytesOnLastPage;
		 }	else { // For any other page
					 bytesLen = tome->bytesPerPage;
		 }

		  // create the byte array
		  uint8_t* arr = malloc(bytesLen + headerLen);

		  // assigned the bytes for the header
		  for(uint32_t i = 0; i < headerLen; i++){
					arr[i] = header[i]; 
		  }

		  uint8_t* pageBytes = nthPage(tome, page);
		  uint32_t totalLen = headerLen + bytesLen;

		  for(uint32_t i = headerLen; i < totalLen; i++){
					 arr[i] = pageBytes[i];
		  }

		  writeBinFile(arr, totalLen);

		  // clean up
		  free(arr);

		  return true;
}



char* generateFilename(char* ofilename, uint32_t page){
		  size_t onameLen = strlen(ofilename);
		  char* hexStr = decimalToHexString(page);
		  size_t pageHexLen = strlen(page);
		  size_t nameLen = onameLen + pageHexLen;

		  char* name = malloc((nameLen + 1) * sizeof(char));
		  
		  for(int i = 0; i < onameLen; i++){
					 name[i] = oname[i];
		  }

		  // assigning the hex string
		  for(int i = onameLen; i < nameLen; i++){
					 name[i] = hexStr[i - onameLen];
		  }
		  
		  name[nameLen] = '\0';

		  return name;


}

bool m0_deconstruct(char* filename, uint32_t pageWidth){
		  
		  long filesize = fs_POSIX(filename); //TODO addapt the OS agnostic version
		  uint8_t* fileBytes = readBin(filename); // must be freed
		  uint8_t* fileHash = hashFile(fileBytes, filesize);
		  
		  databook* tome = malloc(sizeof(databook));
		  computePages(pageWidth, filesize, fileBytes, tome);
		  uint8_t* genericHead = m0_genericHeaderGen(tome, fileHash);
		  long numPages = tome->pages;

		  // pre-proces the final header
		  uint8_t* fheader = m0_finalHeaderGen(tome, gneericHead);
		  uint32_t fheaderLen = m0_finalHeaderLen(tome);

		  uint8_t* fragmentHeader;
		  char* fragmentName;
		  long fragmentHeaderLen;
		  for(long i = 1; i <= numPages; i++){
					 // create the spcific header
					 // NOTE fragment buffer isn't freed as the generic header is reused
					 if(i == numPages){
								fragmentHeader = fheader;
								fragmentHeaderLen = fheaderLen;
					 } else {
								fragmentHeader = m0_headerGen(genericHead, i);
								fragmentHeaderLen = M0_HEADER_DEFAULT;
					 }
					 //create the fragment buffer

					 // generate the name for that file
					 fragmentName = generateFilename(filename, i);

					 // write the file
					 m0_singleFile(tome, i, fragmentName, fragmentHeader, fragmentHeaderLen);

					 // free all pointed block involved - excepted the header info
					 free(fragmentName);
		  }

		  free(fheader);
		  free(genericHead);
}

#endif // READ_C
