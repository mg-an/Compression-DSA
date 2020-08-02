
	DATASTRUCTURES AND ALGORITHMS MINIPROJECT 2016 - 17

	NAME	: MILIND N. GAWDE

	MIS	: 111503021

	BRANCH	: S.Y. COMPUTER ENGINEERING

	PROJECT 	: COMPRESSION AND DECOMPRESSION USING VARIOUS ALGORITHMS

	A) HUFFMAN'S COMPRESSION ALGORITHM
	B) LEMPEL-ZIV-WELCH ALGORITHM
	C) BASIC RUN-LENGTH ALGORITHM
	
	
	To run this program
		> type 'make' in terminal
			(this compiles all programs in the present consortium)
		> type ./huff <filename> OR
		       ./lzw <filename> OR
		       ./rle <filename>
			(this executes the program and shows defined error if there is some problem)

		basically it creates 2 files namely
			1. <filename>.huff / <filename>.lzw / <filename>.rle (compressed form)
			2. <filename>.unhuff / <filename>.unlzw / <filename>.unrle (decompressed form)
		
		where 1. is the compressed form of the file and 2. is the decompressed form of it;
		which is 'technically ' the exact copy of the original file.
