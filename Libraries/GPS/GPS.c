#include "GPS.h"


void GPS_parseRMC(unsigned char *Buffer, uint16_t BufSize, struct minmea_sentence_rmc *RMCframe)
{
	/* Pulling out desired sentence from the frame */
	unsigned char *Buffer_p;
	if ((Buffer_p = SentencePos("RMC", Buffer, BufSize)) == NULL) return;	// desired sentence was not found

	unsigned char CurrSentence[MINMEA_MAX_SENTENCE_LENGTH];
	unsigned char *Sentence_p = CurrSentence;
	do
	{
		(*Sentence_p++) = (*Buffer_p++);
	}
	while (*(Buffer_p-1) != '\n');

	/* Parsing */
	if ((strncmp((char const *)CurrSentence+3, "RMC", 3)) == 0)
	{
		minmea_parse_rmc(RMCframe, (char const *)CurrSentence);
	}
}

void GPS_parseVTG(unsigned char *Buffer, uint16_t BufSize, struct minmea_sentence_vtg *VTGframe)
{
	/* Pulling out desired sentence from the frame */
	unsigned char *Buffer_p;
	if ((Buffer_p = SentencePos("VTG", Buffer, BufSize)) == NULL) return;	// desired sentence was not found

	unsigned char CurrSentence[MINMEA_MAX_SENTENCE_LENGTH];
	unsigned char *Sentence_p = CurrSentence;
	do
	{
		(*Sentence_p++) = (*Buffer_p++);
	}
	while (*(Buffer_p-1) != '\n');

	/* Parsing */
	if ((strncmp((char const *)CurrSentence+3, "VTG", 3)) == 0)
	{
		minmea_parse_vtg(VTGframe, (char const *)CurrSentence);
	}
}

void GPS_parseGGA(unsigned char *Buffer, uint16_t BufSize, struct minmea_sentence_gga *GGAframe)
{
	/* Pulling out desired sentence from the frame */
	unsigned char *Buffer_p;
	if ((Buffer_p = SentencePos("GGA", Buffer, BufSize)) == NULL) return;	// desired sentence was not found

	unsigned char CurrSentence[MINMEA_MAX_SENTENCE_LENGTH];
	unsigned char *Sentence_p = CurrSentence;
	do
	{
		(*Sentence_p++) = (*Buffer_p++);
	}
	while (*(Buffer_p-1) != '\n');

	/* Parsing */
	if ((strncmp((char const *)CurrSentence+3, "GGA", 3)) == 0)
	{
		minmea_parse_gga(GGAframe, (char const *)CurrSentence);
	}
}

void GPS_parseGSA(unsigned char *Buffer, uint16_t BufSize, struct minmea_sentence_gsa *GSAframe)
{
	/* Pulling out desired sentence from the frame */
	unsigned char *Buffer_p;
	if ((Buffer_p = SentencePos("GSA", Buffer, BufSize)) == NULL) return;	// desired sentence was not found

	unsigned char CurrSentence[MINMEA_MAX_SENTENCE_LENGTH];
	unsigned char *Sentence_p = CurrSentence;
	do
	{
		(*Sentence_p++) = (*Buffer_p++);
	}
	while (*(Buffer_p-1) != '\n');

	/* Parsing */
	if ((strncmp((char const *)CurrSentence+3, "GSA", 3)) == 0)
	{
		minmea_parse_gsa(GSAframe, (char const *)CurrSentence);
	}
}

void GPS_parseGSV(unsigned char *Buffer, uint16_t BufSize, struct minmea_sentence_gsv *GSVframe)
{
	/* Pulling out desired sentence from the frame */
	unsigned char *Buffer_p;
	if ((Buffer_p = SentencePos("GSV", Buffer, BufSize)) == NULL) return;	// desired sentence was not found

	unsigned char CurrSentence[MINMEA_MAX_SENTENCE_LENGTH];
	unsigned char *Sentence_p = CurrSentence;
	do
	{
		(*Sentence_p++) = (*Buffer_p++);
	}
	while (*(Buffer_p-1) != '\n');

	/* Parsing */
	if ((strncmp((char const *)CurrSentence+3, "GSV", 3)) == 0)
	{
		minmea_parse_gsv(GSVframe, (char const *)CurrSentence);
	}
}

unsigned char *SentencePos(unsigned char *SentenceID, unsigned char *Buffer, uint16_t BufSize)
{
	uint16_t Char_cnt = 0;
	unsigned char *Buffer_p = Buffer;	// pointer to the first symbol, located in the buffer
	while (true)
	{
		if (*Buffer_p == '$')
		{
			Buffer_p+=3, Char_cnt+=3;
			if (*Buffer_p == SentenceID[0])
			{
				Buffer_p++, Char_cnt++;
				if (*Buffer_p == SentenceID[1])
				{
					Buffer_p++, Char_cnt++;
					if (*Buffer_p == SentenceID[2])
					{
						return Buffer_p-5;
					}
				}
			}
		}
		Buffer_p++, Char_cnt++;
		if (Char_cnt > BufSize) return NULL;
	}
}