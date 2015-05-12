#ifndef wav_header_h
#define wav_header_h

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
struct WaveHeader{
    char               wavName[300];
    char               chunkId[4];
    int                chunkSize;
    char               wavID[4];
    char               fmtID[4];
    unsigned short int fmtChunkSize;
    unsigned short int wFormatTag;
    short int          nChannels;
    int                sampleRate;
    int                byteRate;
    short int          blockAlign;
    short int          bps;
    char               datachunkId[4];
    unsigned int       datachunkSize;
    unsigned int       totalSamples;
    short int * chan1;
    short int * chan2;
    short int * chan3;
    short int * chan4;
};


int readHeader(FILE *fp, struct WaveHeader *header);
void printHeader(struct WaveHeader *header);
void advanceToNextChunk(FILE *fp);
int readAllData(FILE *fp, struct WaveHeader *header);
void freeChannelMemory(struct WaveHeader *wav);
#endif
