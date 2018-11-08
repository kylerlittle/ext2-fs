#ifndef TOKENIZE
#define TOKENIZE

/* Standard Libraries and Header Files */
#include <string.h>
#include <stdlib.h>

/* Function Signatures */
int tokenize(char *tok_list[], char *input_line, char *delimiters);
int clear_tok_list(char *tok_list[]);
int clear_toks_after_i(char *tok_list[], int i);

#endif