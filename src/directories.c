#include "../include/directories.h"



static String get_permissions(mode_t mode) {
    String perm = init_string_of(11);

    if(perm==NULL) return NULL;

    if(push_back(perm,S_ISDIR(mode)  ? 'd' :
                        S_ISLNK(mode)  ? 'l' :
                        S_ISCHR(mode)  ? 'c' :
                        S_ISBLK(mode)  ? 'b' :
                        S_ISFIFO(mode) ? 'p' :
                        S_ISSOCK(mode) ? 's' : '-')==0) {
        goto error;
    }

    if(push_back(perm,(mode & S_IRUSR) ? 'r' : '-')==0) goto error;
    if(push_back(perm,(mode & S_IWUSR) ? 'w' : '-')==0) goto error;
    if(push_back(perm,(mode & S_ISUID) ?
                        ((mode & S_IXUSR) ? 's' : 'S') :
                        ((mode & S_IXUSR) ? 'x' : '-'))==0) goto error;

    if(push_back(perm,(mode & S_IRGRP) ? 'r' : '-')==0) goto error;
    if(push_back(perm,(mode & S_IWGRP) ? 'w' : '-')==0) goto error;
    if(push_back(perm,(mode & S_ISGID) ?
                        ((mode & S_IXGRP) ? 's' : 'S') :
                        ((mode & S_IXGRP) ? 'x' : '-'))==0) goto error;

    if(push_back(perm,(mode & S_IROTH) ? 'r' : '-')==0) goto error;
    if(push_back(perm,(mode & S_IWOTH) ? 'w' : '-')==0) goto error;
    if(push_back(perm,(mode & S_ISVTX) ?
                        ((mode & S_IXOTH) ? 't' : 'T') :
                        ((mode & S_IXOTH) ? 'x' : '-'))==0) goto error;

    return perm;

error:
    free_string(perm);
    return NULL;
}

static String get_mtime(time_t* time){
    String date_buffer = init_string_of(32);
    if(date_buffer==NULL){
        return NULL;
    }
    struct tm* time_info = localtime(time);

    if (time_info != NULL) {
        if(strftime(date_buffer->cptr,date_buffer->psize,"%b %d %H:%M",time_info) == 0){
                free_string(date_buffer);
                return NULL;
        }
        date_buffer->lsize = strlen(date_buffer->cptr);
    }
    else{
        free_string(date_buffer);
        return NULL;
    }
    return date_buffer;
}

static Directory resize_directory(Directory dir){
    DirEntry* tmp;
    size_t new_size;
    if(dir==NULL) return NULL;
    if(dir->lsize>=dir->psize){
        new_size=dir->psize*2;
        
        tmp = realloc(dir->entries,new_size*sizeof(DirEntry));
    
        if(tmp==NULL)
            return NULL;
        
        dir->entries=tmp;
        dir->psize=new_size;
    }
    return dir;
}

Directory new_directory(){
    Directory dir;
    if((dir = calloc(1,sizeof(Directory_t)))==NULL){
        return NULL;
    }
    dir->psize=INITIAL_PSIZE_DIRECTORY;
    if((dir->entries = calloc(dir->psize,sizeof(DirEntry)))==NULL){
        free(dir);
        return NULL;
    }
    return dir;
}

static DirEntry direntry(){
    return calloc(1,sizeof(DirEntry_t));
}

int add_entry(Directory dir,DirEntry entry){
    if(dir==NULL || entry==NULL) return 0;

    if(dir->lsize>=dir->psize){
        if(resize_directory(dir)==NULL)
            return 0;
    }

    *(dir->entries+dir->lsize)=entry;
    dir->lsize++;

    return 1;
}

void free_directory(Directory dir){
    if(dir != NULL){
        for(size_t i=0;i<dir->lsize;i++){
            if(*(dir->entries+i)!=NULL){
                if((*(dir->entries+i))->name!=NULL)free_string((*(dir->entries+i))->name);
                if((*(dir->entries+i))->date!=NULL)free_string((*(dir->entries+i))->date);
                if((*(dir->entries+i))->user!=NULL)free_string((*(dir->entries+i))->user);
                if((*(dir->entries+i))->group!=NULL)free_string((*(dir->entries+i))->group);
                if((*(dir->entries+i))->permission!=NULL)free_string((*(dir->entries+i))->permission);
                free(*(dir->entries+i));
                *((dir->entries+i))=NULL;
            }
        }
        free(dir->entries);
        dir->entries=NULL;
        free(dir);
    }
}

static void print_entry(DirEntry entry, int col_width) {
    int w = (col_width > 1) ? col_width - 1 : 1;

    printw("%-*.*s %-*lu %-*.*s %-*.*s %-*.*s %-*.*s\n",
           w, w, entry->permission->cptr,
           w, entry->size,
           w, w, entry->user->cptr,
           w, w, entry->group->cptr,
           w, w, entry->date->cptr,
           w, w, entry->name->cptr);
}

void ll(Directory dir, size_t underlined_entry, size_t offset) {
    if(dir == NULL) return;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int col_width = cols / 6;
    int w = (col_width > 1) ? col_width - 1 : 1;

    attron(A_BOLD | A_UNDERLINE);

    printw("%-*.*s %-*.*s %-*.*s %-*.*s %-*.*s %-*.*s\n",
           w, w, "permission",
           w, w, "size",
           w, w, "user",
           w, w, "group",
           w, w, "date",
           w, w, "name");

    attroff(A_BOLD | A_UNDERLINE);

    size_t max_items = (rows > 1) ? (size_t)(rows - 1) : 0;

    size_t remaining = dir->lsize - offset;
    size_t limit = remaining < max_items ? remaining : max_items;

    for(size_t i = 0; i < limit; i++) {

        size_t index = offset + i;

        if(index == underlined_entry) {
            attron(A_STANDOUT);
            print_entry(dir->entries[index], col_width);
            attroff(A_STANDOUT);
        }
        else {
            print_entry(dir->entries[index], col_width);
        }
    }
}

Directory directory(const char* path){
    Directory dir = new_directory();
    if(dir==NULL) return NULL;
    DIR* dp;
    struct dirent* dir_entries;
    struct stat properties;
    struct passwd* pw;
    struct group* gr;

    if((dp=opendir(path))==NULL){
        free_directory(dir);
        dir=NULL;
        return NULL;
    }

    while((dir_entries=readdir(dp))!=NULL){
        String name = init_string_from(dir_entries->d_name);
        if(name==NULL){
            closedir(dp);
            free_directory(dir);dir=NULL;
            return NULL;
        }

        String dpath = init_string_from(path);
        if(dpath==NULL){
            closedir(dp);
            free_directory(dir);dir=NULL;
            free_string(name);name=NULL;
            return NULL;
        }
        if(char_at(dpath,dpath->lsize-1)!='/'){
            if(push_back(dpath,'/')==0){
                free_string(dpath);
                closedir(dp);
                free_directory(dir);dir=NULL;
                free_string(name);name=NULL;
                return NULL;
            }
        }
        
        concat(dpath,name);

        if (lstat(dpath->cptr, &properties) == -1) {
            free_string(name);name=NULL;
            free_string(dpath);dpath=NULL;
            continue;
        }
        

        String mode = get_permissions(properties.st_mode);
        if(mode==NULL) {
            closedir(dp);
            free_string(dpath);dpath=NULL;
            free_string(name);name=NULL;
            free_directory(dir);dir=NULL;
            return NULL;
        }

        String time = get_mtime(&properties.st_mtime);
        if(time==NULL){ 
            closedir(dp);
            free_string(dpath);dpath=NULL;
            free_string(name);name=NULL;
            free_directory(dir);dir=NULL;
            free_string(mode);mode=NULL;
            return NULL;
        }
        pw = getpwuid(properties.st_uid);
        if(pw==NULL){
            closedir(dp);
            free_string(dpath);dpath=NULL;
            free_string(name);name=NULL;
            free_directory(dir);dir=NULL;
            free_string(mode);mode=NULL;
            free_string(time);time=NULL;
            return NULL;
        }
        String user_info_pwname = init_string_from(pw->pw_name);
        if(user_info_pwname==NULL){
            closedir(dp);
            free_string(dpath);dpath=NULL;
            free_string(name);name=NULL;
            free_directory(dir);dir=NULL;
            free_string(mode);mode=NULL;
            free_string(time);time=NULL;
            return NULL;
        }
        gr=getgrgid(properties.st_gid);
        if(gr==NULL){
            closedir(dp);
            free_string(dpath);dpath=NULL;
            free_string(name);name=NULL;
            free_directory(dir);dir=NULL;
            free_string(mode);mode=NULL;
            free_string(time);time=NULL;
            free_string(user_info_pwname);user_info_pwname=NULL;
            return NULL;
        }
        String group_info_grname = init_string_from(gr->gr_name);
        if(group_info_grname==NULL){
            closedir(dp);
            free_string(dpath);dpath=NULL;
            free_string(name);name=NULL;
            free_directory(dir);dir=NULL;
            free_string(mode);mode=NULL;
            free_string(time);time=NULL;
            free_string(user_info_pwname);user_info_pwname=NULL;
            return NULL;
        }
        DirEntry dir_entry =  direntry();
        if(dir_entry==NULL){
            closedir(dp);
            free_string(dpath);dpath=NULL;
            free_string(name);name=NULL;
            free_directory(dir);dir=NULL;
            free_string(mode);mode=NULL;
            free_string(time);time=NULL;
            free_string(user_info_pwname);user_info_pwname=NULL;
            free_string(group_info_grname);
            return NULL;
        }
        
        dir_entry->name=name;
        dir_entry->date=time;
        dir_entry->permission=mode;
        dir_entry->user=user_info_pwname;
        dir_entry->group=group_info_grname;
        dir_entry->size=properties.st_size;

        if(!add_entry(dir,dir_entry)){
            free_string(dpath);dpath=NULL;
            free_string(dir_entry->name);dir_entry->name=NULL;
            free_string(dir_entry->date);dir_entry->date=NULL;
            free_string(dir_entry->user);dir_entry->user=NULL;
            free_string(dir_entry->group);dir_entry->group=NULL;
            free_string(dir_entry->permission);dir_entry->permission=NULL;
            free(dir_entry);dir_entry=NULL;

            closedir(dp);
            free_directory(dir);dir=NULL;
            return NULL;
        }

        free_string(dpath);dpath=NULL;
    }

    closedir(dp);

    return dir;
}

String abs_path(const char *path){
    if(path==NULL) return NULL;
    String apath = init_string_of(PATH_MAX);
    if(apath==NULL){
        return NULL;
    }
    if(apath->cptr==NULL){
        free_string(apath);
        return NULL;
    }
    if (realpath(path, apath->cptr) != NULL) {
        apath->lsize = strlen(apath->cptr);
        return apath;
    }   
    free_string(apath);
    return NULL;
}

int cd(Directory* dir,DirEntry entry,String* path){
    if(starts_with_character(entry->permission, 'd') ){
        if(ends_with_character(*path,'/')){
            concat(*path,entry->name);
            String apath = abs_path((*path)->cptr);
            if(apath==NULL){
                return 0;
            }
            free_string((*path));
            *path=apath;
            
            free_directory(*dir);
            *dir=directory((*path)->cptr);
            if(dir==NULL){
                return 0;
            }
        }
        else{
            if(push_back(*path, '/')==0){
                return 0;
            } 
            concat(*path,entry->name);
            String apath = abs_path((*path)->cptr);
            if(apath==NULL){
                return 0;
            }
            free_string((*path));
            *path=apath;

            free_directory((*dir));
            *dir=directory((*path)->cptr);
            if(*dir==NULL){
                return 0;
            }
        }
    }
    return 1;
}


int cd_rel(Directory* dir,String* path,String movement){
    if(ends_with_character(*path,'/')){
        concat(*path,movement);
        String apath = abs_path((*path)->cptr);
        if(apath==NULL){
            return 0;
        }
        free_string((*path));
        *path=apath;
        
        free_directory(*dir);
        *dir=directory((*path)->cptr);
        if(dir==NULL){
            return 0;
        }
    }
    else{
        if(push_back(*path, '/')==0){
            return 0;
        } 
        concat(*path,movement);
        String apath = abs_path((*path)->cptr);
        if(apath==NULL){
            return 0;
        }
        free_string((*path));
        *path=apath;

        free_directory((*dir));
        *dir=directory((*path)->cptr);
        if(*dir==NULL){
            return 0;
        }
    }
    return 1;
}
