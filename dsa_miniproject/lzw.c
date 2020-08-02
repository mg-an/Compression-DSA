#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#define R (256)
#define TMP (-1)
#define WORD_CAPACITY (1 << 3)
#define DICT_SIZE (1 << 16)
#define DICT_CAPACITY (1 << 16)
typedef unsigned short tree_t;
typedef unsigned char tree_key_t;
struct tree_node;
typedef struct tree_node {
	tree_t value;
	struct tree_node *children[R];
}tree_node;
typedef struct tree {
	tree_node *root;
	tree_t count;
}tree;
typedef struct string {
	tree_key_t *buffer;
	int capacity;
	int size;
}string;
tree_node *newNode() {
	int i;
	tree_node *tn = (tree_node *)malloc(sizeof(tree_node));
	tn->value = TMP;
	for(i = 0; i < R; i++)
		tn->children[i] = NULL;
	return tn;
}
void destroyNode(tree_node *tn) {
	int i;
	for(i = 0; i < R; i++) {
		tree_node *child = tn->children[i];
		if (child)
			destroyNode(child);
	}
	free(tn);
}
tree* newtree() {
	tree *t = (tree *)malloc(sizeof(tree));
	t->root = newNode();
	t->count = 0;
	return t;
}
void destroytree(tree *t) {
	destroyNode(t->root);
	free(t);
}
string *newString() {
	string *s = (string *)malloc(sizeof(string));
	s->capacity = WORD_CAPACITY;
	s->size = 0;
	s->buffer = (tree_key_t *)malloc(s->capacity * sizeof(tree_key_t));
	return s;
}
void destroyString(string *s) {
	free(s->buffer);
	free(s);
}
void append(string *s, tree_t c) {
	if (s->size == s->capacity) {
		s->capacity *= 2;
		s->buffer = (tree_key_t *)realloc(s->buffer, s->capacity * sizeof(tree_key_t));
	}
	s->buffer[s->size++] = c;
}
tree_t search(tree *t, string *s) {
	tree_node *curr = t->root;
	int i;
	for(i = 0; i < s->size; i++) {
		curr = curr->children[s->buffer[i]];
		if (curr == NULL)
			return TMP;
	}
	return curr->value;
}
void insert(tree *t, string *s) {
	if (t->count == DICT_SIZE - 1)
		return;
	tree_node *curr = t->root;
	int i;
	for(i = 0; i < s->size; i++) {
		if (!curr->children[s->buffer[i]])
			curr->children[s->buffer[i]] = newNode();
		curr = curr->children[s->buffer[i]];
	}
	curr->value = t->count++;
}
tree *init() {
	tree *t = newtree();
	tree_key_t c = 0;
	do {
		t->root->children[c] = newNode();
		t->root->children[c]->value = (tree_t) c;
	} while (++c);
	t->count = 1 << 8;
	return t;
}
void iWrite(FILE *out, tree_t idx) {
	fwrite(&idx, sizeof(tree_t), 1, out);
}
void compress(FILE *input, FILE *output) {
	tree *t = init();
	tree_node *curr = t->root;
	tree_key_t new_symbol;
	string* successful_entry = newString();
	while (sizeof(tree_key_t) == fread(&new_symbol, sizeof(tree_key_t), 1, input)) {
		tree_node* next = curr->children[new_symbol];
		append(successful_entry, new_symbol);
		if (!next) {
			iWrite(output, curr->value);
			insert(t, successful_entry);
			destroyString(successful_entry);
			successful_entry = newString();
			append(successful_entry, new_symbol);
			next = t->root->children[new_symbol];
		}
		curr = next;
	}
	iWrite(output, curr->value);
	destroyString(successful_entry);
	destroytree(t);
}
char *huff_output_filename(char *filename) {
	int i = 0;
	char *newname;
	while(*(filename+i) != '\0')
		i++;
	newname = malloc(sizeof(char) * (i + 5));
	i = 0;
	while(*(filename + i) != '\0') {
		*(newname + i) = *(filename + i);
		i++;
	}
	*(newname + i++) = '.';
	*(newname + i++) = 'l';
	*(newname + i++) = 'z';
	*(newname + i++) = 'w';
	*(newname + i) = '\0';
	return(newname);
}
string *_newString(int capacity) {
	string *s = (string *)malloc(sizeof(string));
	s->capacity = capacity;
	s->size = 0;
	s->buffer = (tree_key_t *)malloc(capacity * sizeof(tree_key_t));
	return s;
}
string *n_ewString() {
	return _newString(WORD_CAPACITY);
}
string *concatenate(string *s1, string *s2) {
	string *s = _newString(s1->size + 1);
	int idx;
	for(idx = 0; idx < s1->size; idx++)
		append(s, s1->buffer[idx]);
	append(s, s2->buffer[0]);
	return s;
}
void print(string *s) {
	int i;
	for(i = 0; i < s->size; i++)
		printf("%c", s->buffer[i]);
	printf("\n");
}
void sWrite(FILE *out, string *s) {
	fwrite(s->buffer, sizeof(tree_key_t), s->size, out);
}
void decompress(FILE *input, FILE *output) {
	string *dictionary[DICT_CAPACITY];
	tree_key_t c = 0;
	do {
		string *sim = n_ewString();
		append(sim, c);
		dictionary[c] = sim;
	} while (++c);
	tree_t dictSize = R;
	tree_t dictIdx;
	string* successful_entry = n_ewString();
	if (!fread(&dictIdx, sizeof(tree_t), 1, input))
		return;
	successful_entry = concatenate(successful_entry, dictionary[dictIdx]);
	sWrite(output, successful_entry);
	while (fread(&dictIdx, sizeof(tree_t), 1, input)) {
		string *new_successful;
		if (dictIdx < dictSize) {
			new_successful = dictionary[dictIdx];
		} else {
			new_successful = concatenate(successful_entry, successful_entry);
		}
		sWrite(output, new_successful);
		if (dictSize != (DICT_CAPACITY - 1)) {
			dictionary[dictSize++] = concatenate(successful_entry, new_successful);
		}
		successful_entry = new_successful;
	};
	do {
		destroyString(dictionary[dictSize - 1]);
	} while(--dictSize);
}
char *unhuff_output_filename(char *filename){
	int i = 0;
	char *newname;
	while(*(filename+i) != '\0')
		i++;
	newname = malloc(sizeof(char) * (i + 7));
	i = 0;
	while(*(filename + i) != '\0'){
		*(newname+i) = *(filename + i);
		i++;
	}
	*(newname + i++) = '.';
	*(newname + i++) = 'u';
	*(newname + i++) = 'n';
	*(newname + i++) = 'l';
	*(newname + i++) = 'z';
	*(newname + i++) = 'w';
	*(newname + i) = '\0';
	return(newname);
}
int main(int argc, char *argv[]) {
	if (argc != 2){
		printf("Usage: ./huff <filename>\n");
		return EINVAL;
	}
	FILE *input, *output;
	char *outname;
	input  = fopen(argv[1], "rb");
	if (input == NULL) {
		perror("cp: can't open file ");
		exit(errno);
	}
	outname = huff_output_filename(argv[1]);
	output = fopen(outname, "wb");
	if (output == NULL) {
		perror("cp: can't open file ");
		exit(errno);
	}
	compress(input, output);
	fclose(input);
	fclose(output);
	input  = fopen(outname, "rb");
	outname =unhuff_output_filename(argv[1]);
	output = fopen(outname, "wb");
	if (output == NULL) {
		perror("cp: can't open file ");
		exit(errno);
	}
	decompress(input, output);
	return 0;
}
