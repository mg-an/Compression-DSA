#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "huff.h"
int main(int argc, char *argv[]) {
	if (argc != 2){
		printf("Usage: ./huff <filename>\n");
		return EINVAL;
	}
	FILE *fhd = fopen(argv[1], "rb");
	if (fhd == NULL) {
		perror("cp: can't open file ");
		exit(errno);
	}
	unsigned long int storage_table[257][2] = {{0,0}};   
	int curr_char = 0;
	while ((fread(&curr_char, 1, 1, fhd)) > 0) {
		(storage_table[curr_char][0])++;
	}
	int i = 0;
	Node *list = NULL;
	for(i = 255; i >= 0; i--) { 
		if (storage_table[i][0] != 0) {
			list = insert_node(list, node_construct(storage_table[i][0], i));
		}
	}   
	list = insert_node(list, node_construct(1, 256));
	Node *otherlist = NULL;
	int done = 0;
	//ALL variable declaration
	Node *second = NULL;
	Node *tail = NULL;
	Node *temp_parent = NULL;
	Node *first = NULL;
	while (done == 0) {
		if (list == NULL) { 
			first = otherlist;
			otherlist = otherlist->next;
		}
		else { 
			if (otherlist == NULL) {
				first = list;
				list = list->next;
			}
			else{
				if (list->count <= otherlist->count) {
					first = list;
					list = list->next;
				}
				else{
					first = otherlist;
					otherlist = otherlist->next;
				}
			}
		}
		if (list == NULL) {
			if (otherlist == NULL) {
				done = 1;
			}
			else {
				second = otherlist;
				otherlist = otherlist->next;
			}
		}
		else {
			if (otherlist == NULL) {
				second = list;
				list = list->next;
			}
			else {
				if (list->count <= otherlist->count) {
					second = list;
					list = list->next;
				}
				else {
					second = otherlist;
					otherlist = otherlist->next;
				}
			}
		}	
		if (done == 0) {
			temp_parent = node_construct((first->count) + (second->count), '\0');
			if (first->height >= second->height) {
				temp_parent->right = first;
				temp_parent->left = second;
				temp_parent->height = first->height + 1;
			}
			else {
				temp_parent->right = second;
				temp_parent->left = first;
				temp_parent->height = second->height + 1;
			}
			first->next = temp_parent;
			second->next = temp_parent;
			if (otherlist == NULL) {
				tail = temp_parent;
				otherlist = tail;
			}
			else {
				tail->next = temp_parent;
				tail = temp_parent;
			}//incase not noticed we have hit a century
		}//you just saw the number gottcha!
	}
	Node* codetree = first;
	unsigned long int output = 0;
	unsigned long int code = 0xC010000111503021;//secret code; my MIS no. indeed! for compression pattern detection
	unsigned long int lengthy_code = 0;
	int remaining_bits = sizeof(output) * 8;
	char *outname = huff_output_filename(argv[1]);
	FILE *wfp = fopen(outname, "wb");
	fwrite(&code, sizeof(unsigned long), 1, wfp);
	code = 0;
	get_codes(codetree, storage_table, &lengthy_code, &code);	
	print_header(wfp, codetree, &remaining_bits, &output);
	unsigned char cbuff[256];
	int result;
	rewind(fhd);
	if(wfp) {
		result = fread(cbuff, sizeof(char), 256, fhd);
		while(result > 0){
			i = 0;
			while(i < result) {
				lengthy_code = storage_table[(unsigned int)cbuff[i]][1];
				code = storage_table[(unsigned int)cbuff[i]][0];
				if(lengthy_code < remaining_bits) {
					output = (output << lengthy_code) | code;
					remaining_bits -= lengthy_code;
				}
				else if(lengthy_code >= remaining_bits) {
					output = (output << remaining_bits) | (code >> (lengthy_code - remaining_bits));
					fwrite(&output, sizeof(output), 1, wfp);
					output = code;
					remaining_bits = sizeof(output) * 8 - (lengthy_code - remaining_bits);
				}
				i++;
			}
			result = fread(cbuff, sizeof(char), 256, fhd);
		}
		lengthy_code = storage_table[256][1];
		code = storage_table[256][0];
		if(lengthy_code < remaining_bits) {
			output = (output << lengthy_code) | code;
			remaining_bits -= lengthy_code;
		} 
		else if(lengthy_code >= remaining_bits) {
			output = (output << remaining_bits) | (code >> (lengthy_code - remaining_bits));
			fwrite(&output, sizeof(output), 1, wfp);
			output = code;
			remaining_bits = sizeof(output) * 8 - (lengthy_code - remaining_bits);
		}
		if (remaining_bits != sizeof(unsigned long) * 8) {
			output = output << (remaining_bits);
			fwrite(&output, sizeof(output), 1, wfp);
		}
	}
	else {
		printf("Failed to open output file\n");
	}
	fclose(fhd);
	fclose(wfp);
	fhd = fopen(outname, "rb");
	if (fhd == NULL) {
		printf("ERROR: %s can't open.\n", outname);
		exit(errno);
	}
	head = NULL;
	Node *curr_pos = NULL;
	int read_result = 0, remaining_bits_un = 0;
	unsigned char mybit, mybyte;
	mybyte = 0;
	unsigned long int buffer;
	read_result = fread(&buffer, sizeof(unsigned long), 1, fhd);
	if(buffer != 0xC010000111503021) {//Incase not noticed but the decompression key is my MIS no. indeed! but cannot be used as compression and decompression is written in one program
		printf("Error: Not compressed using corresponding program.\n");
		return 0;
	}
	do {
		if(remaining_bits_un == 0) {
			read_result = fread(&buffer, sizeof(unsigned long), 1, fhd);
			remaining_bits_un = sizeof(unsigned long) * 8;
		}
		mybit = (buffer >> (--remaining_bits_un)) & 0x0000000000000001;
		if(mybit) {
			if(remaining_bits_un >= 8) {
				mybyte = (buffer >> (remaining_bits_un - 8)) & 0x00000000000000FF;
				remaining_bits_un -= 8;
			}
			else {
				mybyte = (buffer << (8 - remaining_bits_un)) & 0x00000000000000FF;
				read_result = fread(&buffer, sizeof(unsigned long), 1, fhd);
				mybyte = mybyte | ((buffer >> (sizeof(unsigned long) * 8 - 8 + remaining_bits_un)) & 0x00000000000000FF);
				remaining_bits_un = sizeof(unsigned long) * 8 - (8 - remaining_bits_un);
			}
		}
		curr_pos = tree(mybit, mybyte, curr_pos);
	} while(curr_pos != NULL);
	curr_pos = head;
	char *outname2 = unhuff_output_filename(argv[1]);
	ofhd = fopen(outname2, "wb");
	free(outname2);//double century
	do {//You saw again! caught you!
		if(remaining_bits_un == 0) {
			read_result = fread(&buffer, sizeof(unsigned long), 1, fhd);
			remaining_bits_un = sizeof(unsigned long)*8;
			if(read_result < 1) {
				printf("\n**ERROR:END OF FILE<EOF>**\n");
				return 0;
			}
		}
		mybit = (buffer >> (--remaining_bits_un)) & 0x0000000000000001;
		curr_pos = tree2(mybit, curr_pos);
	} while (curr_pos != NULL);
	free_tree(head);
	fclose(fhd);
	fclose(ofhd);
	return 0;
}
