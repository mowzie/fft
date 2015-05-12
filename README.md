simple little fft program
requires FFTW to be installed
possible gnuplot (if you want gnuplot graphs)

only tested on linux (ubuntu)


HOW TO RUN
after installing fftw:

make
./wavinfo sample/police.wav
    data is stored in sample/police.wav.dat
gnuplot spec.gp
    assuming you have ImageMajick installed
display sample/pierce.wav.png
make clean


