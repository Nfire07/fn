
#include "includes.h"

int main(int argc, char** argv) {
    int key;
    Directory dir;
    const char* path;
    size_t underlined_entry=0;

    if(argc < 2) {
        fprintf(stderr, "Use: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    path = argv[argc - 1];

    dir = directory(path);
    if(dir == NULL) {
        fprintf(stderr, "Error: directory not available %s\n", path);
        return EXIT_FAILURE;
    }

    if(initscr() == NULL) {
        free_directory(dir);
        return EXIT_FAILURE;
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    do {
        clear();

        ll(dir,underlined_entry);

        printw("\n[ESC] - Exit");
        refresh();

        key = getch();

        if(key==KEY_UP){
            if(underlined_entry==0) underlined_entry=dir->lsize;
            underlined_entry=(underlined_entry-1)%dir->lsize;
        }
        else if(key==KEY_DOWN){
            underlined_entry=(underlined_entry+1)%dir->lsize;
        }

    } while(key != 27);

    endwin();

    free_directory(dir);

    return EXIT_SUCCESS;
}
