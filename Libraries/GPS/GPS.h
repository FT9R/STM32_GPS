#ifndef GPS_H
#define GPS_H

#include "minmea.h"
#include "string.h"

/**
*	@brief  Set of functions to handle with common GPS sentences
* @param  Pointer to a Buffer, containing all sentences in a row
* @param  Pointer to structure, representing the processing frame format
* @retval None
*/
void GPS_parseRMC(unsigned char *Buffer, uint16_t BufSize, struct minmea_sentence_rmc *RMCframe);
void GPS_parseVTG(unsigned char *Buffer, uint16_t BufSize, struct minmea_sentence_vtg *VTGframe);
void GPS_parseGGA(unsigned char *Buffer, uint16_t BufSize, struct minmea_sentence_gga *GGAframe);
void GPS_parseGSA(unsigned char *Buffer, uint16_t BufSize, struct minmea_sentence_gsa *GSAframe);
void GPS_parseGSV(unsigned char *Buffer, uint16_t BufSize, struct minmea_sentence_gsv *GSVframe);

/**
*	@brief  Intrinsic maintenance function, determines the position of sentence inside the buffer
* @param  Sentence ID template, for example "GGA" or "RMC"
* @param  Pointer to a Buffer, containing desired sentence
* @param  Buffer size in bytes, i.e total chars qty
* @retval pointer to "$" of current sentence in the buffer
* @note		retval = NULL in case if desired sentence was not found
*/
unsigned char *SentencePos(unsigned char *SentenceID, unsigned char *Buffer, uint16_t BufSize);

#endif