// STD
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- STRING FUNCTIONS --- */
char* 
bs_replaceFirstSubstring(char* str, char* old_str, char* new_str) {
    int new_len = strlen(new_str);
    int old_len = strlen(old_str);
    int str_len = strlen(str);

    // Counting the number of times old word
    // occur in the string
    char *start = strstr(str, old_str);
    int new_size = str_len - old_len + new_len;

    if(start == NULL) 
	return NULL;

    int replace_offset = start-str;

    // Making new string of enough length
    char *result = malloc(new_size);
    char *offset = result;

    strncpy(offset, str, replace_offset); offset += replace_offset;
    strncpy(offset, new_str, new_len);    offset += new_len;
    strncpy(offset, start + old_len, str_len - (offset - result));
    
    result[new_size-1] = '\0';

    return result;
}

char* 
bs_readFileToString(char *path, int *content_len, int *errcode) {
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
