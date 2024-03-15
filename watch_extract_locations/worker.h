//Reads info for input and output file through named pipe
//Saves locations from input file to a map writes them 
//to output file and sends sigstop to himself 
int worker(const char* fifo);
