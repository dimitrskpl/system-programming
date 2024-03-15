#include <unistd.h>
#include <string.h>
#include "string_process.h"
#include <cstdio>
#include <cstdlib>

using namespace std;

void find_loc(const char* str, char** loc){
    if(strlen(str) < 8){ 
        *loc=NULL; //it doesnt contain http://
        return;
    }
    const char start_with[8] = "http://";
    const char *sub_str = strstr(str, start_with); //check if substr contains "http://"
    
    char *str_copy, *orig_str_copy;
    if(sub_str == NULL){
        *loc = NULL;
        return;
    }
    else{
        str_copy = (char*) malloc((strlen(sub_str)+1)*sizeof(char));
        orig_str_copy = str_copy; //copy original pointer for free later
        strcpy(str_copy, sub_str);      
        str_copy+=7;  //ignore http://
    }

    if(strlen(str_copy) > 4){ 
        if(str_copy[0] == 'w' && str_copy[1] == 'w' && str_copy[2] == 'w' && str_copy[3] == '.')
            str_copy+=4; //ignore "www."
    }

    char* rest = str_copy;
    char* token = strtok_r(rest, "/", &rest); //save the rest string before '/', if '/' exists
    if(token != NULL){
        *loc = (char*) malloc((strlen(token)+1)*sizeof(char));
        strcpy(*loc, token);
    }
    else{
        *loc = (char*) malloc((strlen(str_copy)+1)*sizeof(char));
        strcpy(*loc, str_copy);
    }
    free(orig_str_copy);
}


void get_links(char* buffer, map<char*, int, cmp_str> *url_map){ 
    char* rest = buffer;
    char *token = strtok_r(rest, " \n", &rest); //divide buffer by space or '\n'
    while(token != NULL){
        char* loc;
        find_loc(token, &loc);
        if(loc != NULL){
            map<char*, int, cmp_str>::iterator iter;
            iter = url_map->find(loc);
            if(iter == url_map->end()){ //if loc not in map
                url_map->insert(pair<char*, int>(loc, 1)); //insert it
            }
            else{
                free(loc);
                iter->second++; //increase its frequency
            }
        }
        token = strtok_r(NULL, " \n", &rest);
    }
}

int read_all(int fd, char** all_content){
    int nread;
    char buffer[512];
    int total_bytes = 0;
    while((nread = read(fd, buffer, 512)) > 0){
        total_bytes += nread;
    }
    if(nread == -1){
        perror("read");
        return -1;
    }

    *all_content = (char*) malloc((total_bytes+1)*sizeof(char));
    off_t newposition;
    newposition = lseek( fd, -total_bytes, SEEK_END); //poinet in beginning of initial read
    nread = read(fd, *all_content, total_bytes);
    if(nread != total_bytes)
        return -1;

    (*all_content)[total_bytes] = '\0';
    if(nread == -1){
        perror("read");
        return -1;
    }
    return total_bytes;
}

int int_to_string(int number, char **str){
    if(number == 0){
        *str = (char*) malloc(2*sizeof(char));
        (*str)[0] = '0';
        return 1;
    }

    int digits = 0;
    int copy_number = number;
    while(copy_number != 0){
        copy_number /= 10;
        digits++;
    }
    int digits_cp = digits;

    int rem;
    *str = (char*) malloc((digits+1)*sizeof(char));
    (*str)[digits] = '\0';

    while(digits--){
        rem = number % 10;
        number /= 10;
        (*str)[digits] = rem + '0';
    }
    return digits_cp;
}

void str_append(char** final_str, const char* str1, const char* str2, const char* str3){
    *final_str = (char*) malloc((strlen(str1) + strlen(str2) + strlen(str3) + 3)*sizeof(char));
    strcpy(*final_str, str1);
    strcat(*final_str, " ");
    strcat(*final_str, str2);
    strcat(*final_str, " ");
    strcat(*final_str, str3);
}

int get_tokens(char** str1, char** str2, char** str3, char* msg){
    char* rest = msg;
    /* get the first token */
    *str1 = strtok_r(rest, " ", &rest);
    if(*str1==NULL)
        return -1;
    
    *str2 = strtok_r(NULL, " ", &rest);
    if(*str2==NULL){
        return -1;
    }

    *str3 = strtok_r(NULL, " ", &rest);
    if(*str3==NULL){
        return -1;
    }
    return 0;
}
void create_nth_str(char **final_str, char* str, int numb){
    char* str_numb;
    int digits = int_to_string(numb, &str_numb);
    *final_str = (char*) malloc((strlen(str_numb)+strlen(str)+1)*sizeof(char));
    strcat(*final_str, str);
    strcat(*final_str, str_numb);
    free(str_numb);
}

void split_str_by_line(queue<char*> *q, char* str){
    char* rec = str;
    char* token = strtok_r(rec, "\n", &rec);

    while(token != NULL){
        char *cur_str = (char*) malloc((strlen(token)+1)*sizeof(char));
        strcpy(cur_str, token);
        q->push(cur_str);
        token = strtok_r(NULL, "\n", &rec);
    }
}