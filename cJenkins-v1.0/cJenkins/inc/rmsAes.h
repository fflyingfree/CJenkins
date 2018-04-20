#ifndef _RMS_PWD_COMM_
#define _RMS_PWD_COMM_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef _RMSAES_H_
#define _RMSAES_H_

// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES encryption in CBC-mode of operation.
// ECB enables the basic ECB 16-byte block algorithm. Both can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef CBC
  #define CBC 1
#endif

#ifndef ECB
  #define ECB 1
#endif

#define AES128 1
//#define AES192 1
//#define AES256 1

#if defined(ECB) && (ECB == 1)

void AES_ECB_encrypt(const uint8_t* input, const uint8_t* key, uint8_t *output, const uint32_t length);
void AES_ECB_decrypt(const uint8_t* input, const uint8_t* key, uint8_t *output, const uint32_t length);

#endif // #if defined(ECB) && (ECB == !)


#if defined(CBC) && (CBC == 1)

void AES_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
void AES_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);

#endif // #if defined(CBC) && (CBC == 1)


#endif //_RMSAES_H_



#ifndef _RMSAES_INTERFACE_
#define _RMSAES_INTERFACE_

//************************************************
// Summary:   RMS_AES_ENCODE_DECODE
// Method:    rmsAesEncode ¡¢ rmsAesDecode
// Access:    public
// Returns:   char*
// Parameter: char* src£¬int srcLength
// date:      2017/11/29
// author:    baiff3@chinaunicom.cn
//***********************************************/

#define PADDINGFLAG ' '
#define BLOCK_SIZE 16

char* rmsAesEncode(char* in, int inLen);
char* rmsAesDecode(char* in, int inLen);
int rmsAesFree(char** in);

static int _rms_hexStr2ByteArr(char* hexStr, int hexLen, uint8_t byteArr[]);
static int _rms_charStr2ByteArr(char* charStr, int strLen, uint8_t byteArr[]);
static int _rms_byteArr2HexStr(uint8_t byteArr[], int arrLen, char hexStr[]);
static int _rms_byteArr2CharStr(uint8_t byteArr[], int arrLen, char charStr[]);
static int _rms_hexCh2Int(char hexCh);
static int _rms_padding(char* charStr, int strLen, char objCharStr[], int objStrLen);
static int _rms_trim(char* charStr, int strLen, char objCharStr[]);
static int _rms_initKeyByteArr(uint8_t keyByteArr[], int keyLen);

#endif //_RMSAES_INTERFACE_


#endif //_RMS_PWD_COMM_

