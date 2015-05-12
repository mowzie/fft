#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include "wavheader.h"  //local file
#include <fftw3.h>      //3rd party library, needs to be installed

void showUsage();
int writeGpScript(char *datName, int rate, int);
int writeFFT(int N, struct WaveHeader *wav);

//-----------------------------------------------------------------------------
//   Function:    main()
//
//   Description: Opens wav file up and performs FFT
//
//   Programmer:  Ian Littke
//
//   Date:        05/10/2015
//
//   Version:     1.0
//
//   Environment: Ubuntu 10.4
//                Software: gcc 4.8.2
//
//   Parameters:  -file <file.wav>  file to read from
//                -spec <N>         calculate FFT with N samples
//                -showhead         display header info
//
//   Calls:       showUsage()
//                readHeader()
//                printHeader()
//                readAllData(FILE *,struct WaveHeader)
//                writeFFT(char*,int,struct WaveHeader)
//                writeGpScript(char*,rate)
//                freeChannelMemory()
//
//   Returns:     0 (success)
//                1 (fail)
//
//   History Log:
//             05-10-2015  IL completed version 1
//             05-11-2015  IL seperated code, added usage info
//
// ----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
  int i = 0;
  unsigned int N = 0;
  int spec = 0;
  int showHead = 0;
  //int samplerate = 0;
  //unsigned int numOfSamples = 0;
  char fileName[300];
  struct WaveHeader *wav;
  FILE *fWavIn = NULL;

  //ToDO:  better error handling

  if (argc < 3) {
    showUsage();
    return 1;
  }

  printf("Total args %d\n", argc);
  for (i = 1; i < (argc); i++) {

    if (strcasecmp("-spec", argv[i])==0) {
      spec = 1;

      if (i+1 == argc) {
        fprintf(stderr,"\n**ERROR: No sample size associated with \"-spec\"**\n");
        return 1;
      }

      N = atoi(argv[++i]);
      if (N < 5) {
        fprintf(stderr, "\n**ERROR: <%s> is not a valid sample size (use 5+)**\n", argv[i]);
        return 1;
      }
      continue;
    }
    if (strcmp("-?", argv[i])==0) {
      showUsage();
      return 1;
    }

    if (strcasecmp("-showhead", argv[i])==0) {
      showHead = 1;
      continue;
    }
    if (strcasecmp("-file", argv[i])==0) {
      //require a filename to go along with the -file
      //check -file isn't the last arg, otherwise it throws a sigsegv
      if (i+1 == argc) {
        fprintf(stderr,"\n**ERROR: No filename listed with \"-file\"\n");
        return 1;
      }
      strncpy(fileName, argv[++i],300);
      fWavIn = fopen(fileName, "rb");
      if (!fWavIn) {
        fprintf(stderr,"\n**ERROR: Could not open file \"%s\"\n", fileName);
        return 1;
      }
      continue;
    }
  }

  //it's a little hacky, but this checks if we have a file
  if(fWavIn==NULL) {
    showUsage();
    return 1;
  }

  //return 0;
  wav = malloc(sizeof(struct WaveHeader));
  strncpy(wav->wavName,fileName ,300);
  if (readHeader(fWavIn, wav) != 0) {
    fprintf(stderr, "Error reading (%s), check if it is a valid wav file\n",fileName);
    return 1;
  }
  if (showHead == 1) {
    printHeader(wav);
  }
  readAllData(fWavIn,wav);

  //process the data
  if (spec == 1) {
    writeFFT(N, wav);
  }
  /*
  //test code to write out raw data as it was read in
  for (i = 0; i < numOfSamples; i++) {
    fwrite(&wav->chan1[i],1,2,fOut);
    if (wav->nChannels > 1)
      fwrite(&wav->chan2[i],1,2,fOut);
    if (wav->nChannels > 2)
      fwrite(&wav->chan3[i],1,2,fOut);
    if (wav->nChannels > 3)
      fwrite(&wav->chan4[i],1,2,fOut);
  }
  */

  //free up memory
  freeChannelMemory(wav);
  free(wav);
  fclose(fWavIn);

  return 0;
}


void showUsage() {
  printf("Usage: ./wavInfo -file <file.wav> [-spec <N>] [-showHead]\n");
  printf("       <file.wav> can be up to 4 channels\n");
  printf("       -spec writes FFT data to <file.wav.dat> and generates gnuplot script\n");
  printf("         <N> is the samplesize for the FFT, for best results use 2^N\n");
  printf("           dat file is written in same location as the wav file\n");
  printf("           gnuplot script is written as \"spec.gp\" in the same folder as \"wavInfo\"\n");
  printf("       -showHead displays the pertinant header information (assuming wav file)\n");
  printf("\n");
}



//TODO: nest in channel for multi-channel plot
int writeFFT(int N, struct WaveHeader *wav){
  int i, j;
  double *in;
  double mag = 0.0;
  double freqBin = 0.0;
  double correction = 0.0;
  fftw_complex * out;
  fftw_plan my_plan;
  FILE * fDatOut;
  int nameLength = 0;
  char datFilename[300];

  //add ".dat" to the end of the file
  //will fix later to rename ".wav"
  nameLength  = sizeof(wav->wavName)/sizeof(wav->wavName[0]);
  snprintf(datFilename, nameLength+4, "%s.dat", wav->wavName);
  fDatOut = fopen(datFilename, "w");
  //Not sure how to test this one
  if (!fDatOut) {
    fprintf(stderr,"\n**ERROR opening dat file for writing!**\n");
    return 1;
  }

  correction = (double)wav->sampleRate / (double)N;
  in = (double*) fftw_malloc(sizeof(double)*N);
  out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*((N/2)+1));
  my_plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_MEASURE);

  printf("Calcuting FFT, placing dat file at \'%s\'\n",datFilename);

  //Iterate through the entire wav file, overlap & add by buffer size
  for (i = 0; i < wav->totalSamples; i=i+(N-N/2)) {
    //Iterate through the sample size
    for (j = 0; j < N; j++) {
      if ((i+j) < wav->totalSamples) {
        in[j] =  wav->chan1[(i+j)]*(0.54 - 0.46 * cos(2 * M_PI * j / (N-1)));
      }
    }

    //Perform FFT for the sample size
    fftw_execute(my_plan);

    //Iterate through sample size
    //skip the first element, since it is the average of the sample
    for (j = 1; j<(N/2)+1; j++) {
      mag = 2*(out[j][0]*out[j][0] + out[j][1]*out[j][1])/N;

      //Set the frequency bin
      freqBin = (double)(j) * correction;

      //Print out (sample FreqBin dB (unscaled))
      //This can be used with gnuplot or matlab to generate spectrogram
      fprintf(fDatOut,"%d %f %f  \n",i,freqBin,(10. * log10(mag+0.001))/log10(10));
    }
    //gnuplot requires a line break between sample sets for spectrograms
    fprintf(fDatOut,"\n");
  }
  fftw_free(in);
  fftw_free(out);
  fftw_destroy_plan(my_plan);
  fftw_cleanup();
  fclose(fDatOut);
  //write gnuplot script
  printf("Writing spec.gp\n");
  writeGpScript(datFilename,wav->sampleRate, N);

  return 0;
}


int writeGpScript(char *datName, int rate,int N){
  FILE *gnuplotFile = fopen("spec.gp", "w");

  fprintf(gnuplotFile,"########\n");
  fprintf(gnuplotFile,"DATFILE = \"%s\"\n",datName);
  fprintf(gnuplotFile,"set yrange[0:%d] #top frequency to show\n",rate/2);
  fprintf(gnuplotFile,"#set xrange[0:]  #number of seconds to display\n");
  fprintf(gnuplotFile,"########\n");
  fprintf(gnuplotFile,"set terminal png enhanced size 800,600\n");
  fprintf(gnuplotFile,"set view 0,0\n");
 // fprintf(gnuplotFile, "set contour base\n");
  fprintf(gnuplotFile,"set lmargin at screen 0.15\n");
  fprintf(gnuplotFile,"set rmargin at screen 0.85\n");
  fprintf(gnuplotFile,"set bmargin at screen 0.15\n");
  fprintf(gnuplotFile,"set tmargin at screen 0.85\n");
  fprintf(gnuplotFile,"set title DATFILE\n");
  fprintf(gnuplotFile,"set output DATFILE[:strlen(DATFILE)-4].\'.png\'\n");
  fprintf(gnuplotFile,"set ytics rotate\n");
  fprintf(gnuplotFile,"set xlabel \"Time (s)\" offset character 0, 2,0\n");
  fprintf(gnuplotFile,"###\n");
  fprintf(gnuplotFile,"###I couldn't get the label to show =(\n");
  fprintf(gnuplotFile,"###set label 2 \"Frequency (Hz)\"\n");
  fprintf(gnuplotFile,"###\n");
  fprintf(gnuplotFile,"show label\n");
  fprintf(gnuplotFile,"set ytics border mirror\n");
  fprintf(gnuplotFile,"unset ztics\n");
  fprintf(gnuplotFile,"set grid			#Put gridlines on the plot\n");
  //fprintf(gnuplotFile,"set format cb \"\%10.1\f\"\n");
  fprintf(gnuplotFile,"\n");
  fprintf(gnuplotFile,"set cbtics border in scale 0,0 autojustify mirror offset -8,0\n");
  fprintf(gnuplotFile,"set colorbox user\n");
  fprintf(gnuplotFile,"\n");
  fprintf(gnuplotFile,"set colorbox vertical user origin 0.1, 0.29 size 0.01,0.45\n");
  fprintf(gnuplotFile,"#set logscale y 2\n");
  fprintf(gnuplotFile,"color(x) = x    #this just scales down the colorbox numbers\n");
  //fprintf(gnuplotFile,"#set yrange[500:2000]\n");
  fprintf(gnuplotFile,"\n");
  fprintf(gnuplotFile,"splot DATFILE using ($1/%d):2:(color($3)) with pm3d notitle\n",rate);
  fclose(gnuplotFile);
  return 0;
}

/*
char *GetFileName(const char *path)
{
    char *filename = strrchr(path, '\\');
    if (filename == NULL)
        filename = path;
    else
        filename++;
    return filename;
}
*/
