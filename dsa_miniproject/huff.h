//huffman compression and decompression
#ifndef __HUFFH__
#define __HUFFH__
typedef struct bn{
	unsigned long int count;
	unsigned int ch;
	int height;
	struct bn *left;
	struct bn *right;
	struct bn *next;
}Node; 
Node *node_construct(int count, unsigned int ch) {
	Node *n = malloc(sizeof(Node));
	n->count = count;
	n->ch = ch;
	n->height = 0;
	n->left = NULL;
	n->right = NULL;
	n->next = NULL;
	return n;
}
void node_destruct(Node *n) {
	free(n);
}
Node *insert_node(Node *head, Node *n) {
	if (head == NULL) {
		return n;
	}
	Node *prev = NULL;
	Node *curr = head;
	while ((curr != NULL) && (curr->count < n->count)){
		prev = curr;
		curr = curr->next;
	}
	if (curr == head){
		n->next = curr;
		return n;
	}
	prev->next = n;
	n->next = curr;
	return head;
}
void free_tree(Node *root) {
	if (root == NULL) {
		return;
	}
	free_tree(root->right);
	free_tree(root->left);
	node_destruct(root);
}
void print_binary(int o, unsigned long d, unsigned long l) {
	unsigned long i = 0;
	unsigned long temp = d;
	temp = temp << (sizeof(unsigned long) * 8 - l);
	if(o){
		while(i < (sizeof(unsigned long) * 8 - l))
			printf(" ");
			i++;
	}
	i = 0;
	while(i < l) {
		if(temp & 0x8000000000000000){
			printf("1");
		}
		else{
			printf("0");
		}
		temp = temp << 1;
		i++;
	}
}
void pack(FILE *wfp, int *bits_remaining, unsigned long int *output, unsigned long int input, unsigned long length){
	if(length < *bits_remaining){
		*output = (*output << length)|input;
		*bits_remaining -= length;            
	}
	else{
		*output = (*output << *bits_remaining) | (input>>(length - *bits_remaining));
		fwrite(output,sizeof(char), 8, wfp);
		*output = input;
		*bits_remaining = sizeof(unsigned long) * 8 - (length - *bits_remaining);
	}
}
void get_codes(Node *root, unsigned long int codetable[257][2], unsigned long int *length, unsigned long int *code){
	if(root == NULL){
		return;
	}
	if(root->height == 0){
		codetable[root->ch][0] = *code;
		codetable[root->ch][1] = *length;
		return;	
	}
	(*length)++;
	*code = (*code) << 1;
	get_codes(root->left, codetable, length, code);
	(*code)++;
	get_codes(root->right, codetable, length, code);
	(*length)--;
	*code = (*code) >> 1; 
}
void print_header(FILE *wfp, Node *root, int *bits_remaining, unsigned long int *output){
	if (root == NULL){
		return;
	}
	if ((root->left == NULL) && (root->right == NULL)) {
		pack(wfp, bits_remaining, output, 1, 1);
		if (root->ch != 256){
			pack(wfp, bits_remaining, output, root->ch, 8);
		}
		else {
			pack(wfp, bits_remaining, output, 255, 8);
			pack(wfp, bits_remaining, output, 0, 1);
		}
	}
	else{
	pack(wfp, bits_remaining, output, 0, 1);
	}
	print_header(wfp, root->left, bits_remaining, output);
	print_header(wfp, root->right, bits_remaining, output);
	node_destruct(root);
	root = NULL;
}
void print_tree(Node *root) {
	if (root == NULL) {
		return;
	}
	print_tree(root->left);
	if ((root->left == NULL) && (root->right == NULL)) {
		unsigned long int count = 0;
		if (root->ch == 256){
			printf("%lu, <EOF>, h:%d\n", count, root->height);
		}
		else {
			printf("%lu, <(%d)%c>, h:%d\n", count, root->ch, (char) root->ch, root->height);
		}
	}
	print_tree(root->right);
}
void print_list(Node * head) {
	if (head == NULL) {
		return;
	}
	if (head->ch == 256) {
		printf("%lu, <EOF>\n", head->count);
	}
	else {
		printf("%lu, <%c>\n", head->count, (char) head->ch);
	}
	print_list(head->next);
}
void print_two_lists(Node *list, Node *other) {
	if (list != NULL) {
		if (list->ch == 256) {
			printf("%3lu, <EOF>    ", list->count);
		}
		else {
			printf("%3lu, <%c>      ", list->count, (char) list->ch);
		}
		list = list->next;
	}
	else {
		printf("              ");
	}
	if (other != NULL) {
		if (other->ch == 256) {
			printf("%3lu, <EOF>    ", other->count);
		}
		else {
			printf("%3lu, <%c>      ", other->count, (char) other->ch);
		}
		other = other->next;
	}
	printf("\n");
	if ((other != NULL) || (list != NULL)) {
		print_two_lists(list, other);
	}
} 

char *huff_output_filename(char *filename){
	int i = 0;
	char *newname;
	while(*(filename+i) != '\0') 
			i++;
	newname = malloc(sizeof(char)*(i+6));
	i = 0;
	while(*(filename + i) != '\0'){
		*(newname+i) = *(filename + i);
		i++;
	}
	*(newname + i++) = '.';
	*(newname + i++) = 'h';
	*(newname + i++) = 'u';
	*(newname + i++) = 'f';
	*(newname + i++) = 'f';
	*(newname + i) = '\0';
	return(newname);
}
Node *head;
FILE *ofhd;
Node *tree(unsigned char bit, unsigned char byte, Node *current_position){
	if (head == NULL){
		if (bit == 0){
			head = node_construct(0, 0);
			return head;
		}
		else {
			head = node_construct(0, byte);
			return  NULL;
		}
	}
	if (current_position == NULL){
		Node *n = head;
		while (n->right != NULL) {
			n = n->right;
		}
		n->ch = 256;
		return NULL;
	}
	if (bit == 0){
		if (current_position->left == NULL){
			Node *n = node_construct(0,0);
			current_position->left = n;
			n->next = current_position;
			return n;
		}
		else if (current_position->right == NULL){
			Node *n = node_construct(0,0);
			current_position->right = n;
			n->next = current_position;
			return n;
		}
		else {
			return tree(bit, byte, current_position->next);
		}
	}
	if (bit == 1){
		if (current_position->left == NULL){
			Node *n = node_construct(0, byte);
			current_position->left = n;
			n->next = current_position;
			return current_position;
		}
		else if (current_position->right == NULL){
			Node * n = node_construct(0, byte);
			current_position->right = n;
			n->next = current_position;
			return current_position;
		}
		else {
			return tree(bit, byte, current_position->next);
		}
	}
	return NULL;
}
Node *tree2(unsigned char bit, Node * current_position){
	if (current_position == NULL){
		return NULL;
	}
	if (bit == 1){
		current_position = current_position->right;
	}
	else if (bit == 0){
		current_position = current_position->left;
	}
	if ((current_position->right == NULL) && (current_position->left == NULL)){
		if (current_position->ch == 256){
			return NULL;
		}
		unsigned char temp = (unsigned char) current_position->ch;
		fwrite(&temp, sizeof(unsigned char), 1, ofhd);
		return head;
	}
	return current_position;
}
char *unhuff_output_filename(char *filename){
	int i = 0;
	char *newname;
	while(*(filename+i) != '\0')
		i++;
	newname = malloc(sizeof(char)*(i+8));
	i = 0;
	while(*(filename + i) != '\0'){
		*(newname+i) = *(filename + i);
		i++;
	}
	*(newname + i++) = '.';
	*(newname + i++) = 'u';
	*(newname + i++) = 'n';
	*(newname + i++) = 'h';
	*(newname + i++) = 'u';
	*(newname + i++) = 'f';
	*(newname + i++) = 'f';
	*(newname + i) = '\0';
	return(newname);
}
#endif
