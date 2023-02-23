#ifndef BS_MEM
#define BS_MEM

#include <stdint.h>
#define BS_FLAGSET(flag, cmp) ((flag >> cmp) & 0x01)

uint_least8_t	bs_memU8 (void *data, uint_fast32_t offset);
uint_least16_t	bs_memU16(void *data, int offset);
uint32_t	bs_memU32(void *data, uint_fast32_t offset);

int bs_memcmpU32(const void *a, const void *b);

char* bs_replaceFirstSubstring(const char *str, const char *old_str, const char *new_str);
char* bs_fileContents(const char *path, int *content_len, int *errcode);
void bs_appendToFile(const char *filepath, const char *data);
void bs_writeToFile(const char *filepath, const char *data);

void bs_pushText();

#endif /* BS_MEM */
