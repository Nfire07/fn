#define STRINGS_H
#ifdef STRINGS_H
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_PSIZE_STRING 128

typedef struct{
    char* cptr;
    size_t lsize;
    size_t psize;
}String_t;

typedef String_t* String;

String init_string();
String init_string_of(size_t psize);
String init_string_from(const char* cptr);
int equals(String s1,String s2);
int starts_with_character(String s,char ch);
int ends_with_character(String s,char ch);
char char_at(String str,size_t index);
int set_char(String str,char c,size_t index);
char* get_cstring(String str);
int push_back(String str,char c);
void concat(String dest,String src);
void free_string(String str);



#endif