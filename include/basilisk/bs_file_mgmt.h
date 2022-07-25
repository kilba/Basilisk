#ifndef BS_FILE_MGMT
#define BS_FILE_MGMT

char* bs_readFileToString(char *path, int *content_len, int *errcode);
void bs_appendToFile(const char *filepath, const char *data);
void bs_writeToFile(const char *filepath, const char *data);

#endif /* BS_FILE_MGMT */