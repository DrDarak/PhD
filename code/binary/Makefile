CC = gcc
# CFLAGS = -O2 -pg -Wall
CFLAGS = -O2 -ansi -pedantic -Wall

LIBS = -lm

CODESRCS = code.c vq.c mem_hand.c image_io.c blocks.c quad.c file_io.c huff.c

DECODESRCS = decode.c vq.c mem_hand.c image_io.c blocks.c quad.c file_io.c huff.c

CODEOBJS = ${CODESRCS:.c=.o}

DECODEOBJS = ${DECODESRCS:.c=.o}

HEADERS =  vq.h mem_hand.h image_io.h blocks.h quad.c file_io.h huff.h

all: code decode

code: ${CODEOBJS} ${HEADERS}
	${CC} ${CFLAGS} -o code ${CODEOBJS} ${LIBS}

decode: ${DECODEOBJS} ${HEADERS}
	${CC} ${CFLAGS} -o decode ${DECODEOBJS} ${LIBS}

depend:
	makedepend -- ${CFLAGS} -- ${SRCS}

clean:
	/bin/rm -f code core *.o
# DO NOT DELETE
