#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

uint_least8_t
bs_memU8(void *data, uint_fast32_t offset) {
    return *((uint_least8_t*)data + offset);
}

uint_least16_t
bs_memU16(void *data, int offset) {
    const uint8_t *base = data;
    base += offset;

    uint_least16_t b1 = base[0], b0 = base[1]; 
    return (uint_least16_t) (b1 << 8 | b0);
}

uint32_t
bs_memU32(void *data, uint_fast32_t offset) {
    const uint8_t *base = data;
    base += offset;

    uint_least32_t b3 = base[0], b2 = base[1], b1 = base[2], b0 = base[3]; 
    return (uint_least32_t) (b3 << 24 | b2 << 16 | b1 << 8 | b0);
}

int bs_memcmpU32(const void *a, const void *b) {
    return memcmp(a, b, 4);
}

/* --- STRING FUNCTIONS --- */
char* 
bs_replaceFirstSubstring(const char* str, const char* old_str, const char* new_str) {
    if(str == NULL || old_str == NULL || new_str == NULL) return NULL;
    int new_len = strlen(new_str);
    int old_len = strlen(old_str);
    int str_len = strlen(str);

    // Counting the number of times old word
    // occur in the string
    char *start = strstr(str, old_str);

    if(start == NULL) 
	return NULL;

    int replace_offset = start - str;
    int last_size = str_len - replace_offset - old_len;
    
    int new_size = replace_offset + new_len + last_size + 1;

    // Making new string of enough length
    char *result = malloc(new_size);
    char *offset = result;

    memcpy(offset, str, replace_offset); offset += replace_offset;
    memcpy(offset, new_str, new_len); offset += new_len;
    memcpy(offset, str + replace_offset + old_len, last_size);
    result[new_size-1] = '\0';

    return result;
}

char* 
bs_fileContents(const char *path, int *content_len, int *errcode) {
    if(path == 0) {
        *errcode = 1;
        return NULL;
    }

    char *buffer = 0;
    long length;
    FILE * f = fopen (path, "rb");

    if (f)
    {
      fseek (f, 0, SEEK_END);
      length = ftell (f) + 1;
      fseek (f, 0, SEEK_SET);
      buffer = malloc (length);
      if (buffer)
      {
        fread (buffer, 1, length, f);
      }
      fclose (f);
    } else {
        *errcode = 2;
        return NULL;
    }

    *errcode = 0;
    *content_len = length;
    buffer[length - 1] = '\0';
    return buffer;
}

void 
bs_appendToFile(const char *filepath, const char *data) {
    FILE *fp = fopen(filepath, "ab");
    if (fp != NULL) {
        fputs(data, fp);
        fclose(fp);
    }
}

void 
bs_writeToFile(const char *filepath, const char *data) {
    FILE *fp = fopen(filepath, "w");
    if (fp != NULL) {
        fputs(data, fp);
        fclose(fp);
    }  
}
