#ifndef FILES_H
#define FILES_H


#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ncurses.h>
#include <errno.h>

typedef enum {
    VIEW_TEXT,
    VIEW_BINARY
} ViewMode;

int view_file(const char *path, ViewMode mode);
int open_file(const char *path);

ViewMode detect_file_type(const char *path);



#endif