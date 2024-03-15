#include <fcntl.h>   
#include <unistd.h>
#include <string.h>
#include <map> 
#include <signal.h>
#include "string_process.h"
#include <cstdlib>
#include <cstdio>

#define PERMS 0666 // set access permissions
#define BUFSIZE 512 //for msgbuf

using namespace std;

//open file named fifo_name, read its content, save to msgbuf
//and close. On succesful execution fd of fifo_name is returned > 0
//else -1. msgbuf must be of BUFSIZE
int open_read_close(const char* fifo_name, char msgbuf[][BUFSIZE]);

//write each location and its frecuency contained
//in urls map to opened output file with fd fd_to
//and free space allocated with malloc for strings locations
//On success return 0, else -1
int write_results(int fd_to, map<char*, int, cmp_str> *urls_map);


//Read info from fifo and separate to path_in, path_out, file.
//read file located in path_in and save locations and their frequency. 
//Create file.out located in path_out and write saved info to it.
//Return 0 on succesful execution, else -1
int worker(const char* fifo){
    int from, to;

    char msgbuf[BUFSIZE]; //assume that message has a maximum size of BUFSIZE
    int fd_fifo = open_read_close(fifo, &msgbuf);
    if(fd_fifo < 0){
        perror("open_read_close");
        return -1;
    }
    
    char *file = NULL, *path_in = NULL, *path_out = NULL;
    //Seperate info
    if((get_tokens(&file, &path_in, &path_out, msgbuf)) == -1){
        perror("get_tokens");
        return -1;
    }

    //create string for opening file to read from
    char* file_in = (char*) malloc((strlen(file)+strlen(path_in)+1)*sizeof(char));
    strcpy(file_in, path_in);
    strcat(file_in, file);

    //Open given file
    if ((from = open(file_in,  O_RDONLY)) == -1){
        perror("open");
        free(file_in);
        return -1;
    }
    free(file_in);
    map<char* ,int, cmp_str> urls_map; //map location to its frequency
    char *all_content;
    if(read_all(from, &all_content) == -1){
        perror("read_all");
        close(from);
        return -1;
    }
    close(from);
    get_links(all_content, &urls_map);
    free(all_content);

    //create string for opening file to write in
    char* file_out = (char*) malloc((strlen(file)+strlen(path_out)+5)*sizeof(char));
    strcpy(file_out, path_out);
    strcat(file_out, file);
    strcat(file_out, ".out");

    //Create new file for output
    if ((to = open(file_out, O_CREAT | O_WRONLY , PERMS)) == -1){
        perror("creating");
        free(file_out);
        return -1;
    }
    free(file_out);

    if(write_results(to, &urls_map) == -1){
        perror("write results");
        close(to);
        return -1;
    }
    close(to); 

    if(raise(SIGSTOP)!=0){
        perror("raise");
        return -1;
    }

    return 0;
}


int open_read_close(const char* fifo_name, char msgbuf[][BUFSIZE]){
    int fd_fifo;
    if((fd_fifo = open(fifo_name, O_RDONLY)) < 0){
        perror("fifo open problem");
        return -1;
    }
    //Save info from fifo to msgbuf 
    int nread;
    if((nread = read(fd_fifo, *msgbuf, BUFSIZE)) < 0){
        perror("problem in reading");
        close(fd_fifo);
        return -1;
    }
    (*msgbuf)[nread] = '\0';
    close(fd_fifo);
    return fd_fifo;
}

int write_results(int fd_to, map<char*, int, cmp_str> *urls_map){
        map<char*, int, cmp_str>::iterator itr;
        for (itr = urls_map->begin(); itr != urls_map->end(); ++itr){ 
            char *str_numb;
            int_to_string(itr->second, &str_numb);
            char *line = (char*) malloc((strlen(itr->first) + strlen(str_numb) + 3)*sizeof(char));
            strcpy(line, itr->first);
            free(itr->first);
            strcat(line, " ");
            strcat(line, str_numb);
            free(str_numb);
            strcat(line, "\n");

            if( write(fd_to, line, strlen(line)) == -1){
                perror("write");
                return -1;
            }
            free(line);
        }
        return 0;
}