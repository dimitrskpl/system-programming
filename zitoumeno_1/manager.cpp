#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <queue>
#include <map>    
#include "worker.h" 
#include "string_process.h"
 
#define READ 0 //for pipe
#define WRITE 1 //for pipe
#define BUFFSIZE 2048 //assume that 2048 bytes is an upper bound of characters read from inotify each time 
using namespace std;

//open file named filename and write msg to it, then close it.
//Return fd of filename on success, else -1.
int open_write_close(const char* filename, const char* msg);

//Send SIGCONT to the first worker in workers_queue, open 
//and write message_to_sent in the fifo which is mapped to 
//In case of success return 0, else -1
int continue_worker(pid_t worker, const char *fifo_name, char *message_to_sent);

//Divide received_message into three tokens
//Combine 1st and 3rd tokens with path_out 
//seperated by space and save to received_message
//Neccessary space for message_to_sent is allocated with malloc.
//Return 0 on success, else -1.
int create_message(char **message_to_sent, char* received_message, char *path_out);

//wait for workers and check if they stopped 
//If a worker stopped push him in workers queue
void handle_stopped_child(int sig);

//manager sends sigterm to listener, sigcont to 
//stopped workers and waits for active workers
//unlink and free fifos
void handle_termination(int sig);

//worker exits
void worker_handle_sigint(int sig);

//send sigint to workers and then to manager
void handle_error();

static pid_t listener_pid;
static queue<pid_t> workers_queue; //stopped workers
static map<pid_t, char*> worker_fifo_map; //map worker to a unique fifo name
//default exit value from manager for handler on success, else 1
static int success = 0; 

int main(){
    char path_in[] = "../input/"; //path where listener looks at for new files 
    char path_out[] = "../output/"; //path where worker creates file.out and writes results 


    static struct sigaction act_sigint;
    act_sigint.sa_handler = handle_termination;
    act_sigint.sa_flags = SA_RESTART; //for read in inotify not to block
    sigfillset(&(act_sigint.sa_mask));
    sigaction(SIGINT, &act_sigint, NULL);
    int pid;
    int p[2];

    if(pipe(p) == -1){ perror("pipe"); exit(1);}
    if((pid = fork ()) == -1){    
        perror("fork"); 
        exit(1);   
    }
    
    if(pid == 0){ //LISTENER: Redirect stdout and exec inotifywait

        close(p[READ]);
        dup2(p[WRITE], 1); 
        close(p[WRITE]);
        char* const  arguments[] = {"inotifywait","-m", path_in, "-e", "create", "-e", "moved_to", NULL}; 
        execvp("inotifywait", arguments);
        perror("execvp");
        success = 1;
        handle_error(); //never should reach this point
    }
    else{ 
        // MANAGER: Read message received by inotifywait. 
        //Assigh jobs to workers. If no worker available, 
        //create one (fork), else continue stopped worker
        close(p[WRITE]);
        listener_pid = pid;
        int nread;
        int cnt_fifos = 0; //counts the # of fifos created
        char inotify_output[BUFFSIZE];

        //Read message received by inotifywait
        while((nread = read(p[READ], inotify_output, BUFFSIZE)) > 0){
            inotify_output[nread] = '\0';
            queue<char*> inotify_msgs; //save all splited by line messages 
            split_str_by_line(&inotify_msgs, inotify_output);
            
            while(!inotify_msgs.empty()){ //for each message received by inotify
                char* received_message = inotify_msgs.front();
                inotify_msgs.pop();
                char* message_to_sent;
                create_message(&message_to_sent, received_message, path_out);
                free(received_message);
                if(!workers_queue.empty()){ //If worker available (stopped) sent signal to continue
                    pid_t worker = workers_queue.front();
                    map<pid_t, char*>::iterator itr;
                    itr = worker_fifo_map.find(worker); //find the fifo mapped to worker
                    const char *fifo_name;
                    if(itr == worker_fifo_map.end()){
                        perror("find in map fifo");
                        success = 1;
                        handle_error();
                    } 
                    
                    fifo_name = itr->second;
                    if(continue_worker(worker, fifo_name, message_to_sent) == -1){
                        perror("continue_worker");
                        success = 1;
                        handle_error();
                    }
                    workers_queue.pop();
                    free(message_to_sent);
                }
                else{ //If no worker available create one (fork)
                    int pid2;
                    char *fifo_name; 
                    char starting_str[] = "/tmp/myfifo"; //new fifo name will start with this str
                    create_nth_str(&fifo_name, starting_str, cnt_fifos++); //create a new fifo name appending cnt_fifo to starting_str
                    if(mkfifo(fifo_name, 0666) == -1){
                        if(errno != EEXIST){ //if it does not exist
                            perror("receiver: mkfifo");
                            success = 1;
                            handle_error();
                        }
                    }

                    if((pid2 = fork ()) == -1){ 
                        perror("fork"); 
                        success = 1;
                        handle_error();
                    }
                    char buff[100];
                    if(pid2 == 0){ //New worker
                        signal(SIGINT, worker_handle_sigint);
                        while(1){  //until exit due to error or sigint           
                            if((worker(fifo_name)) < 0){
                                perror("worker");
                                break;
                            }
                        }
                        success = 2;
                        worker_fifo_map.erase(getpid());
                        exit(1); //in case of error in worker function
                    }
                    else{ //manager continues
                        worker_fifo_map.insert(pair<pid_t, char*>(pid2, fifo_name)); //save new worker and the fifo mapped to him
                        static struct sigaction act_sigchld;
                        act_sigchld.sa_handler = handle_stopped_child;
                        act_sigchld.sa_flags = SA_RESTART; //for read in inotify not to block
                        sigfillset(&(act_sigchld.sa_mask));
                        sigaction(SIGCHLD, &act_sigchld, NULL);

                        int fd_fifo = open_write_close(fifo_name, message_to_sent);
                        free(message_to_sent);

                        if(fd_fifo < 0){
                            perror("open_write_close");
                            success = 1;
                            handle_error();
                        }
                    }
                }
            }  
        }

        //never should reach this point
        close(p[READ]);
        kill(listener_pid, SIGTERM);
        exit(1);
    }
}

int open_write_close(const char* filename, const char* msg){
    int fd_fifo;
    if((fd_fifo = open(filename, O_WRONLY)) < 0){
        perror("fifo open error");
        return -1;
    }
    int nwrite;
    if((nwrite = write(fd_fifo, msg, strlen(msg)+1)) < 0){
        perror("Error in Writing");
        close(fd_fifo);
        return -1;
    }
    close(fd_fifo);
    return fd_fifo;
}

int continue_worker(pid_t worker, const char *fifo_name, char *message_to_sent){
    int ret = kill(worker, SIGCONT); //continue worker
    if(ret==-1){
        perror("SIGCONT");
        return -1;
    }
    if(open_write_close(fifo_name, message_to_sent) == -1){
        perror("open_write_close");
        return -1;
    }
    return 0;
}

int create_message(char **message_to_sent, char* received_message, char* path_out){
    char *create_msg, *path_in, *file, *rest;
    rest = received_message;
    path_in = strtok_r(rest, " ", &rest);
    if(path_in == NULL){
        perror("create_message: too small message to handle");
        return -1;
    }
    create_msg = strtok_r(NULL, " ", &rest); //The word "CREATE" in received message (ignore it)
    if(create_msg == NULL){
        perror("create_message: too small message to handle");
        return -1;
    }
    file = strtok_r(NULL, " ", &rest);
    if(file == NULL){
        perror("create_message: too small message to handle");
        return -1;
    }
    str_append(message_to_sent, file, path_in, path_out);
    return 0;
}

void handle_stopped_child(int sig){
    pid_t worker_pid;
    int status;
 
    while((worker_pid = waitpid(-1, &status, WNOHANG | WUNTRACED))>0){ 
        char buff2[100];
        int exit_status;
        int signal;
        if(WIFSTOPPED(status)){
            workers_queue.push(worker_pid);
        }
    }
}

void handle_termination(int sig){
    int ret, status;
    kill(listener_pid, SIGTERM);
    while(waitpid(listener_pid, &status, WNOHANG) == 0){continue;} //wait for listener

    map<pid_t, char*>::iterator itr;
    for(itr=worker_fifo_map.begin(); itr!=worker_fifo_map.end(); ++itr){
        unlink(itr->second); //unlink fifo
        free(itr->second); //free space allocated for fifo names
    }

    //send SIGCONT to stopped workers and remove them from worker_fifo_map
    while(!workers_queue.empty()){
        pid_t worker = workers_queue.front();
        workers_queue.pop();
        worker_fifo_map.erase(worker); 
        int pid;
        kill(worker, SIGCONT);
        while(waitpid(worker, &status, WNOHANG) == 0){continue;} //wait for stopped worker
    }
       
    //for each active worker
    for(itr=worker_fifo_map.begin(); itr!=worker_fifo_map.end(); ++itr){
        pid_t worker = itr->first;
        while(waitpid(worker, &status, WNOHANG) == 0){continue;} //wait for active workers who have received sigint
    }

    exit(success); //manager exits
}

void worker_handle_sigint(int sig){
    exit(0); //worker exits
}

void handle_error(){
    map<pid_t, char*>::iterator itr;
    //send sigint to all workers
    for(itr=worker_fifo_map.begin(); itr!=worker_fifo_map.end(); ++itr){
        pid_t worker = itr->first;
        int ret = kill(worker, SIGINT);
    }
    kill(getpid(), SIGINT); //send sigint to yourself (manager)
}