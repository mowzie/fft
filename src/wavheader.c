#include "wavheader.h"

struct WaveHeader *headerCreate() {
    struct WaveHeader *wav = malloc(sizeof(struct WaveHeader));
    return wav;
}

void printHeader(struct WaveHeader *header) {
    unsigned int i = 0;
    for (i = 0; i < sizeof(header->chunkId); i++)
    {
      printf("%c", header->chunkId[i]);
    }
    printf("\n");
    printf("%d\n", header->chunkSize+8);
    printf("wav ID: ");
      for (i = 0; i < sizeof(header->chunkId); i++)
    {
      printf("%c", header->wavID[i]);
    }
    printf("\n");
    printf("Fmt ID: ");
      for (i = 0; i < sizeof(header->chunkId); i++)
    {
      printf("%c", header->fmtID[i]);
    }
    printf("\n");
        printf("fmt size: %d\n", header->fmtChunkSize);
    printf("format tag: %hu\n", header->wFormatTag);
    printf("N channels: %hu\n", header->nChannels);
    printf("Sample per sec: %d\n", header->sampleRate);
    printf("bytes per sec: %d\n", header->byteRate);
    printf("blockalign: %d\n", header->blockAlign);
    printf("bits per sample: %d\n", header->bps);

    for (i = 0; i < sizeof(header->chunkId); i++)
    {
      printf("%c", header->datachunkId[i]);
    }
    printf("\r\nData size: %d\n", header->datachunkSize);
    return;
}

int readHeader(FILE *fp, struct WaveHeader *header) {
  char name[4];
  int cont = 0;

  if (fp)
  {
    fread(&header->chunkId, sizeof(char), 4, fp); //read in first four bytes

    if (header->chunkId[0] != 'R' || header->chunkId[1] != 'I' || header->chunkId[2] != 'F' || header->chunkId[3] != 'F')
      return 1;


    fread(&header->chunkSize, sizeof(int), 1, fp); //read in 32bit size value

    fread(&header->wavID, sizeof(char), 4, fp);
    fread(&header->fmtID, sizeof(char), 4, fp);
    fread(&header->fmtChunkSize, sizeof(unsigned int), 1, fp);
    fread(&header->wFormatTag, sizeof(unsigned short int), 1, fp);
    fread(&header->nChannels, sizeof(short int), 1, fp);
    fread(&header->sampleRate, sizeof(int), 1, fp);
    fread(&header->byteRate, sizeof(int), 1, fp);
    fread(&header->blockAlign, sizeof(short int), 1, fp);
    fread(&header->bps, sizeof(short int), 1, fp);
    //data chucnk should be the next one
    //implement a catch just in case
    //this hasn't been tested yet...
    fread(&name, sizeof(char), 4, fp);
    while (!cont) {
      if (name[0] != 'd' || name[1] != 'a' || name[2] != 't' || name[3] != 'a') {
        printf("**%s\n", name);
        advanceToNextChunk(fp);
        fread(&name, sizeof(char), 4, fp);
      }
      else
        cont = 1;
    }
    fseek(fp,-4,SEEK_CUR);
    fread(&header->datachunkId, sizeof(char),4,fp);
    fread(&header->datachunkSize, sizeof(int),1,fp);
  }
  header->totalSamples = header->datachunkSize / (header->nChannels * (header->bps / 8));

  return 0;
}

void advanceToNextChunk(FILE *fp) {
  int size = 0;
  fread(&size, sizeof(int), 1, fp);

  fseek(fp,size,SEEK_CUR);
}

int readAllData(FILE *fp, struct WaveHeader *header) {
  int i = 0;


    header->chan1 = malloc(header->totalSamples  * sizeof(short int));
  if (header->nChannels > 1)
    header->chan2 = malloc((header->totalSamples  * sizeof(short int)));
  if (header->nChannels > 2)
    header->chan3 = malloc((header->totalSamples  * sizeof(short int)));
  if (header->nChannels > 3)
    header->chan4 = malloc((header->totalSamples  * sizeof(short int)));

  for (i = 0; i < header->datachunkSize / header->blockAlign; i++) {
    fread(&header->chan1[i],1,2,fp);
    if (header->nChannels > 1)
      fread(&header->chan2[i],1,2,fp);
    if (header->nChannels > 2)
      fread(&header->chan3[i],1,2,fp);
    if (header->nChannels > 3)
      fread(&header->chan4[i],1,2,fp);
  }
  return 0;
}

void freeChannelMemory(struct WaveHeader *wav)
{
    free(wav->chan1);
  if (wav->nChannels >1)
    free(wav->chan2);
  if (wav->nChannels >2)
    free(wav->chan3);
  if (wav->nChannels >3)
    free(wav->chan4);
}
