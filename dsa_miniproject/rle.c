#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define LINE_LEN 13
int main(int argc, char *argv[]) {
	FILE *src, *dest;
	unsigned int next_char, prev_char;
	int i, rep_count = 1;
	char *dest_name;
	if (argc != 2){
		printf("Usage: ./rle <filename>\n");
		return EINVAL;
	}
	if((src = fopen(argv[1], "rb")) == NULL) {
		perror("cp: can't open file ");
		exit(errno);
	}
	dest_name = malloc(strlen(argv[1]) + 5);
	strcpy(dest_name, argv[1]);
	strcat(dest_name, ".rle");
	if ((dest = fopen(dest_name, "wb")) == NULL) {
		perror("cp: can't open file ");
		exit(errno);
	}
	next_char = getc(src);
	prev_char = 0;
	while (prev_char != EOF) {
		for(i = 0; i < LINE_LEN; i++) {
			prev_char = next_char;
			next_char = getc(src);
			if(next_char == prev_char)
				rep_count++;
			else if (prev_char != EOF) {
				fprintf(dest, "%02d %x ", rep_count, prev_char);
				rep_count = 1;
			}
		}
		fprintf(dest, "\n");
		i = 0;
	}    
	fclose(src);
	fclose(dest);
	unsigned int ch;
	char *r;    
	if((src = fopen(dest_name, "rb")) == NULL) {
		perror("cp: can't open file ");
		exit(errno);
	}
	r = malloc(strlen(argv[1]) + 7);
	strcpy(r, argv[1]);
	strcat(r, ".unrle");
	if((dest = fopen(r, "wb")) == NULL) {
		perror("cp: can't open file ");
		exit(errno);
	}
	while(fscanf(src, "%2d %2x", &rep_count, &ch) != EOF) {
		if(rep_count > 1) {
			for(i = 0; i < rep_count; i++) {
				putc((char) ch, dest);
			}
		} else {
			putc((char) ch, dest);
		}
	}
	free(dest_name);
	return 0;
}
