#Path for input folder, where files will be copied/created/moved
INPUT = ../input/
#Path for output folder, where new files .out will be created
OUTPUT = ../output/
OBJS = manager.o worker.o string_process.o 
SOURCE = manager.cpp worker.cpp string_process.cpp 
HEADER = worker.h string_process.h
EXEC = manager
CC = g++
FLAGS = -c -Wno-write-strings  
#-c for objects file creation
#Wno-write-strings for ignoring the warning of converting 
#a string constant to ‘char* in manager.cpp initilizing
#arguments for execvp

all: $(EXEC)

$(EXEC) : $(OBJS)
	${CC} -o $@ ${OBJS}

manager.o: manager.cpp worker.cpp string_process.cpp
	$(CC) $(FLAGS) manager.cpp 
	$(CC) $(FLAGS) worker.cpp 
	$(CC) $(FLAGS) string_process.cpp 

worker.o: worker.cpp string_process.cpp
	$(CC) $(FLAGS) worker.cpp 
	$(CC) $(FLAGS) string_process.cpp

string_process.o: string_process.cpp
	$(CC) $(FLAGS) string_process.cpp

#clean objects and executable
clean:
	rm -f $(OBJS) $(EXEC)

# run: $(EXEC)
# 	./$(EXEC)

#clean file *.txt inserted in INPUT folder
clean_input_txt:
	rm -f $(INPUT)*.txt

#clean file *.out inserted in OUTPUT folder
clean_output_txt: 
	rm -f $(OUTPUT)*.out