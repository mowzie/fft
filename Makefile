CFLAGS= -Wall -lfftw3 -lm

all:
	gcc  src/main.c src/wavheader.c $(CFLAGS) -o wavinfo
clean:
	rm sample/*.dat
	rm sample/*.png
	rm wavinfo
	rm spec.gp
