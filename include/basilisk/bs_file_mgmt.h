#ifndef BS_FILE_MGMT
#define BS_FILE_MGMT

char* bs_readFileToString(char *path, int *errcode);
void bs_appendToFile(const char *filepath, const char *data);

#endif /* BS_FILE_MGMT */