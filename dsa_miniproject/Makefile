all: huff lzw rle
rle: rle.o
	cc rle.o -o rle
rle.o: rle.c
	cc -Wall -c rle.c
huff: huffman.o
	cc huffman.o -o huff
huff.o: huffman.c huff.h
	cc -Wall -c huffman.c
lzw: lzw.o
	cc lzw.o -o lzw
lzw.o: lzw.c
	cc -Wall -c lzw.c
