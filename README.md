# system-programming
**manager listener**

The function of project:
	For each new file with name <filename> created in a certain folder a new file is created with name <filename>.out whose content are the locations of URLs included in the initial file and their frequency. For example if we have a file test.txt, which includes the string "http://www.di.uoa.gr " repeated 3 times, which is moved to the input folder, then a new file with name test.txt.out is created in the output folder and its content is di.uoa.gr 3.
  
This function is successed through pipes, named pipes, fork, exec, priority queue, low-level I/O and signals .
The main entities of the project are the listener, the manager and the worker.
The Listener receives the name of the file for which it informs the manager via pipe and the Manager notifies or creates a worker process via named pipe to process the specific file. The manager keeps information about the available workers in a queue and creates new ones only if there are no available workers. Each worker processes one file at a time and its job is to open the file and search through low-level I/O URLs that use the http protocol (of the form http://...). Each URL starts with http:// and ends with a blank character. For each URL that is detected, its location is extracted without the www. For example, the location of the URL "http://www.di.uoa.gr/" is "di.uoa.gr".
  
For each file that is processed, the worker creates a new file, in which it records all the locations it detected and their frequency. E.g. if in the added file 3 URLs with location "di.uoa.gr" appear, the output file of the worker will contain the line "di.uoa.gr" 3" and correspondingly a line for every other location. If the the file read by the worker has the name <filename> the file it creates has the name <filename>.out When a worker finishes processing a file, he notifies the manager that he has finished his work and is available by sending himself a SIGSTOP signal so that it enters a stopped state and so the manager receives a SIGCHLD signal and sees which child has changed state using waitpid().
  
Termination of processes:
Processes do not terminate by themselves, the user must stop them. Termination of the manager is done with Control+C (signal SIGINT) and before terminating it kills all other processes (listener and workers).

ADDITIONAL FUNCTION:
After the creation of files in the input folder or the move of files to it there are several new files created in the output folder whose content are the domains found and their frequency. The additional function of this project is to create a shell script finder.sh which receives one or more Top Level Domain (TLD) which we want to search in all the .out files. To be more precise we find the amount of TLDs' appereances in all the .out files. The content of the .out files are of the below form:
location num_of_appearances. 
For instance, giving as argument the TLD "com" the result will be the sum of "num_of_appearances" where locations are ending in com. The execution of the program must be: ./sniffer [-p path] where "-p path" is used to indicate the path of the directory for the monitoring. If no extra argument is provided then the default path of the directory is the current folder.
