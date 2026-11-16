#include "includes.h"
#include <ncurses.h>
#include <stdlib.h>

#define CTRL(c) ((c) & 0x1f)

int main(int argc, char** argv) {
    int key;
    int rows, cols;
    Directory dir;
    String path;
    size_t underlined_entry=0,offset=0,visible_items;
    int curses_started = 0;
    int exit_code = EXIT_SUCCESS;

    if(argc < 2) {
        const char* home = getenv("HOME");
        if(home==NULL)
            home=".";
        path = abs_path(home);
    }else{
        path = abs_path(argv[argc - 1]);
    }

    if(path==NULL) return EXIT_FAILURE;
    if(path!=NULL && path->cptr==NULL){
        free_string(path);
        return EXIT_FAILURE;
    }

    dir = directory(path->cptr);
    if(dir == NULL) {
        fprintf(stderr, "Error: directory not available %s\n", path->cptr);
        free_string(path);
        return EXIT_FAILURE;
    }

    if(!install_signal_handlers()){
        return EXIT_FAILURE;
    }

    if(initscr() == NULL) {
        free_directory(dir);
        goto cleanup;
    }

    curses_started = 1;

    getmaxyx(stdscr, rows, cols);

    if(rows<30 || cols<50){
        endwin();
        fprintf(stderr, "Terminal needs to be at least 30 rows height and 50 columns wide\n");
        free_directory(dir);
        free_string(path);
        return EXIT_FAILURE;
    }

    start_color(); 

    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    do {
        clear();
        getmaxyx(stdscr, rows, cols);
        visible_items = (rows > 1) ? (size_t)(rows - 1) : 0;

        ll(dir,underlined_entry,offset);
        refresh();

        key = getch();

        if(get_termination_signal() != 0)
            break;

        if(dir->lsize > 0 && visible_items>0) {
            if(key == KEY_UP) {
                if(underlined_entry > 0)
                    underlined_entry--;
                else
                    underlined_entry = dir->lsize - 1;
                if(underlined_entry < offset)
                    offset = underlined_entry;
                if(underlined_entry >= offset + visible_items)
                    offset = underlined_entry - visible_items + 1;
            }
            else if(key == KEY_DOWN) {
                underlined_entry = (underlined_entry + 1) % dir->lsize;

                if(underlined_entry < offset)
                    offset = underlined_entry;

                if(underlined_entry >= offset + visible_items)
                    offset = underlined_entry - visible_items + 1;
            }
        }
        if(key=='\n' || key == KEY_ENTER){
            if(dir->lsize > 0) {
                DirEntry entry = dir->entries[underlined_entry];

                if(starts_with_character(entry->permission, '-')) {
                    String tmp = init_string_from(path->cptr);

                    if(tmp == NULL) {
                        endwin();
                        free_directory(dir);
                        free_string(path);
                        return EXIT_FAILURE;
                    }

                    if(!ends_with_character(tmp, '/'))
                        push_back(tmp, '/');

                    concat(tmp, entry->name);

                    open_file(tmp->cptr);

                    free_string(tmp);
                }
                else {
                    if(cd(&dir, entry, &path) == 0) {
                        endwin();
                        free_directory(dir);
                        free_string(path);
                        return EXIT_FAILURE;
                    }

                    underlined_entry = 0;
                    offset = 0;
                }
            }
        }
        else if(key==KEY_BACKSPACE){
            String prev = init_string_from("..");
            if(prev==NULL){
                endwin();
                free_directory(dir);
                free_string(path);
                return EXIT_FAILURE;
            }
            if(cd_rel(&dir, &path,prev)==0) {
                endwin();
                free_directory(dir);
                free_string(path);
                free_string(prev);
                return EXIT_FAILURE;
            }
            free_string(prev);
            underlined_entry = 0;
            offset = 0;
        }
        else if(key == 'e'){

        }
        else if(key == 'f'){

        }
    } while(key != 27);

    cleanup:

        if(curses_started)
            endwin();

        if(dir != NULL)
            free_directory(dir);

        if(path != NULL)
            free_string(path);

        if(get_termination_signal() != 0)
            return 128 + get_termination_signal();

        return exit_code;
}
