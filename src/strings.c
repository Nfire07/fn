#include "../include/strings.h"


static String resize(String str){
    char* tmp;
    size_t new_size;
    if(str==NULL) return NULL;
    if(str->lsize>=str->psize){
        new_size=str->psize;
        while(new_size<=str->lsize){
            new_size*=2;
        }
        tmp = realloc(str->cptr,new_size*sizeof(char));
        if(tmp==NULL)
            return NULL;
        str->cptr=tmp;
        str->psize=new_size;
    }
    return str;
}

String init_string(){
    String str;

    if((str = calloc(1,sizeof(String_t)))==NULL){
        return NULL;
    }

    str->psize=INITIAL_PSIZE_STRING;
    if((str->cptr=calloc(str->psize,sizeof(char)))==NULL){
        free(str);
        str=NULL;
        return NULL;
    }

    return str;
}

String init_string_from(const char* cptr){
    if(cptr==NULL) return NULL;

    String str = init_string();

    if(str==NULL) return NULL;

    while(*cptr!='\0'){
        if(push_back(str,*cptr)==0){
            free_string(str);
            return NULL;
        }
        cptr++;
    }

    return str;
}

int equals(String s1,String s2){
    if (s1 == s2) return 1;
    if (s1 == NULL || s2 == NULL) return 0;
    if (s1->lsize != s2->lsize) return 0;
    if (s1->lsize == 0) return 1;
    if (s1->cptr == NULL || s2->cptr == NULL) return 0;

    return memcmp(s1->cptr, s2->cptr, s1->lsize) == 0;
}

int starts_with_character(String s,char ch){
    if(s!=NULL && s->cptr!=NULL && *(s->cptr)==ch){
        return 1;   
    }
    return 0;
}

int ends_with_character(String s,char ch){
    if(s!=NULL && s->cptr!=NULL && *(s->cptr+s->lsize)==ch){
        return 1;   
    }
    return 0;
}

String init_string_of(size_t psize){
    String str;

    if((str = calloc(1,sizeof(String_t)))==NULL)
        return NULL;

    if(psize!=0)
        str->psize=psize;
    else
        str->psize=INITIAL_PSIZE_STRING;

    if((str->cptr=calloc(str->psize,sizeof(char)))==NULL){
        free(str);
        str=NULL;
        return NULL;
    }

    return str;
}

char char_at(String str,size_t index){
    if(str==NULL) return -1;
    if(str->lsize>index){
        return *(str->cptr+index);
    }
    return '\0';
}

int set_char(String str,char c,size_t index){
    if(str==NULL || index>=str->psize) return 0;

    *(str->cptr+index)=c;

    if(index>=str->lsize){
        str->lsize=index+1;
        *(str->cptr+str->lsize)='\0';
    }

    return 1;
}

char* get_cstring(String str){
    if(str==NULL) return NULL;
    return str->cptr;
}

int push_back(String str,char c){
    if(str==NULL) return 0;

    if(str->lsize>=str->psize){
        str=resize(str);
        if(str==NULL) return 0;
    }

    *(str->cptr+str->lsize)=c;
    str->lsize++;
    *(str->cptr+str->lsize)='\0';

    return 1;
}

void free_string(String str){
    if(str==NULL) return;
    if(str->cptr!=NULL){
        free(str->cptr);
        str->cptr=NULL;
    }
    free(str);
    str=NULL;
}

void concat(String dest,String src){
    if(dest==NULL && src==NULL){
        return;
    }
    if(dest==NULL && src!=NULL){
        dest = init_string();

        for(size_t i=0;i<src->lsize;i++){
            if(push_back(dest,char_at(src,i))==0){
                return;
            }
        }

        return;
    }
    if(src==NULL && dest!=NULL){
        return;
    }

    for(size_t i=0;i<src->lsize;i++){
        if(push_back(dest,char_at(src,i))==0){
            return;
        }
    }

}