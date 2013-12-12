rejpeg: jpeg-6b/libjpeg.a blockrecord.o datasrc.o parse_image.o
	gcc -g -gdwarf-3 -std=c99 -o rejpeg jpeg-6b/*.o blockrecord.o datasrc.o parse_image.o

blockrecord.o: blockrecord.c
	gcc -g -gdwarf-3 -O0 -std=c99 -c blockrecord.c
datasrc.o: datasrc.c
	gcc -g -gdwarf-3 -O0 -std=c99 -c datasrc.c
parse_image.o: parse_image.c
	gcc -g -gdwarf-3 -O0 -std=c99 -c parse_image.c

clean:
	rm *.o rejpeg

all: rejpeg
