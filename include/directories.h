#define DIRECTORIES_H
#ifdef DIRECTORIES_H

#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include "strings.h"
#include <string.h>
#include <ncurses.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>


#define INITIAL_PSIZE_DIRECTORY 16

typedef struct {
    String name;
    String date;
    String user;
    String group;
    uint64_t size;
    String permission;
}DirEntry_t;

typedef DirEntry_t* DirEntry;

typedef struct{
    DirEntry* entries;
    size_t lsize;
    size_t psize;
}Directory_t;

typedef Directory_t* Directory;

Directory directory(const char* path);
Directory new_directory();
void print_entry(DirEntry entry,int col_width);
void ll(Directory dir,size_t underlined_entry);
DirEntry direntry();
int add_entry(Directory dir,DirEntry entry);
void free_directory(Directory dir);







#endif