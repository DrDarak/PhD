CC = gcc
#CFLAGS = -O2 -Wall
CFLAGS =  -O3 -g -Wall

LIBS = -lm

DECODESRCS = zoom.c rd_swtch.c frac.c file_io.c image_io.c mem_hand.c render.c blocks.c img_huff.c huff.c quad.c quantise.c 
CODESRCS = code.c rd_swtch.c frac.c file_io.c image_io.c mem_hand.c render.c blocks.c img_huff.c huff.c quad.c quantise.c 

DECODEOBJS = ${DECODESRCS:.c=.o}
CODEOBJS = ${CODESRCS:.c=.o}

HEADERS = implicit.c bft.c rd_swtch.h file_io.h frac.h image_io.h mem_hand.h render.h struct.h blocks.h img_huff.h huff.h quad.h quantise.h

all: code decode 

decode: ${DECODEOBJS} ${HEADERS}
	${CC} ${CFLAGS} -o decode ${DECODEOBJS} ${LIBS}


code: ${CODEOBJS} ${HEADERS}
	${CC} ${CFLAGS} -o code ${CODEOBJS} ${LIBS}

depend:
	makedepend -- ${CFLAGS} -- ${SRCS}

clean:
	/bin/rm -f code core *.o

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

# DO NOT DELETE
