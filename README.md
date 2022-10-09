# system-programming
A project about observing a folder for newly created or moved files and searching-extracting locations from URLs, while meassuring their frequency. Additionally, there is a shell script whose function is to search for the number of appearances of certain TLDs in the output files of the first part.

**search-extract locations**

The files, which are useful for the first function (search-extract locations from URLs and meassuring their frequency in newly created and moved files in a certain folder), are included in the folder "extract_locations". The name of the folder which is observed for newly created and moved files is "input/", while the name of the folder where new files are created with the extract information is "output/".

**function** 
For each new file with name <filename> created in or moved to the "input/" folder a new file is created with name <filename>.out in the "output/" folder., whose content are the locations of URLs included in the initial file and their frequency. The input files are text files, which contain text and URLs. The goal is to search for URLs and extract their location and the frecuency of each location and write this information to the .out file. The search is limited to URLs using the http protocol that are of the form http://... More specifically, each URL starts with http:// and ends with a blank character. For each URL, the location information is extracted without the www. For example, if the file test.txt, which includes the string "http://www.di.uoa.gr " repeated 3 times, the string "http://www.google.com" repeated 2 times and the string "http://www.github.com" repeated 5 times, is moved to the input folder, then a new file with name test.txt.out is created in the output folder and its content is:
di.uoa.gr 3
google.com 2
github.com 5
	
compilation: make all
execution: ./manager 
compilation and execution: make run
termination: Termination of the program is done with Control+C 
	
The main entities of this part are the listener, the manager and the worker.
	
**Listener**
The Listener observes the input/ folder for newly created or moved files and receives the name of each file for which it informs the manager via pipe.
	
**Manager**	
The Manager process notifies or creates a worker process via named pipe to process the specific file. It keeps information about the available workers in a queue and creates new ones only if there are no available workers. If there is available (stopped) worker, manager starts him by sending SIGCONT.
	
**Worker**
Each worker processes one file at a time and its job is to open the file and search through low-level I/O URLs that use the http protocol. For each URL that is detected, worker extracts its location without the www and counts how many times it is repeated in the file. For each file that is processed, the worker creates a new file, in which it records all the locations it detected and their frequency. E.g. if in the added file 3 URLs with location "di.uoa.gr" appear, the output file of the worker will contain the line "di.uoa.gr 3" and correspondingly a line for every other location. If the file read by the worker has the name <filename>, then the file it creates has the name <filename>.out. When a worker finishes processing a file, he notifies the manager that he has finished his work and is available by sending himself a SIGSTOP signal so that it enters a stopped state and so the manager receives a SIGCHLD signal and sees which child has changed state using waitpid().
  
Termination of processes:
Processes do not terminate by themselves, the user must stop them. Termination of the manager is done with Control+C (signal SIGINT) and before terminating it kills all other processes (listener and workers).

For this part pipes, named pipes, fork, exec, priority queue, low-level I/O and signals are used.
	
**Shell script for searching TLDs**
The files, which are useful for the second function (shell script for searching for TLDs), are included in the folder "sniffer".
After the creation of files in the input folder or the move of files to it there are several new files created in the output folder, whose content are the domains found and their frequency. The shell script finder.sh receives as arguments one or more Top Level Domain (TLD) which we want to search in all the .out files. The goal is to find the amount of TLDs' appereances in all the .out files. The content of the .out files are of the below form:
location num_of_appearances. For each TLD given as argument, a line is printed, which is of the form "Number of appearance of tld t: X", where X is the amount of appearance of tld t.
For instance, giving as argument the TLDs "com gr" in the previous example where there is only one .out file with content:
di.uoa.gr 3
google.com 2
github.com 5	
	
the result will be:
Number of appearance of tld com: 7
Number of appearance of tld gr: 3
	
Execution: ./sniffer [-p path] args where "-p path" is used to indicate the path of the directory for the monitoring and args the TLDs, which are searched. If the parameter -p path is not provided then the default path of the directory is the current folder.
