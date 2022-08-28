#ifndef BS_MEM
#define BS_MEM

char* bs_replaceFirstSubstring(char* str, char* old_str, char* new_str);
char* bs_readFileToString(char *path, int *content_len, int *errcode);
void bs_appendToFile(const char *filepath, const char *data);
void bs_writeToFile(const char *filepath, const char *data);

#endif /* BS_MEM */
