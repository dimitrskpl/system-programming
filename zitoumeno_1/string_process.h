#include <map>
#include <queue>
#include <string.h>

using namespace std;

//Parse string str and save location to loc,
//if its acceptable, else loc=NULL.
//If loc!=NULL, the neccessary allocated space
//for loc is allocated with malloc.
void find_loc(const char* str, char** loc);

//compare strings for map
struct cmp_str{
    bool operator()(char const *str1, char const *str2) const{
        return strcmp(str1, str2) < 0;
    }
};

//Split string str, save links and their frequency to url_map.
void get_links(char* str, map<char*, int, cmp_str> *url_map); 

//read an open file with file descriptor fd per 512 bytes until 
//end of file, save all content to all_content alocating
//memory with malloc. Return number of bytes read on success,
//else -1
int read_all(int fd, char** all_content);

//Convert number to string allocating the neccessary space with malloc. 
//Return # of digits of number
int int_to_string(int number, char **str);

//Append the 3 strings str1, str2, str3 in one final_str divided by space
//Neccessary space for final_str is allocated with malloc
void str_append(char** final_str, const char* str1, const char* str2, const char* str3);

//Divide msg to 3 tokens seperated by " " and save 
//their content to str1, str2 and str3.
//Return -1, if msg contains less than 3 strings.
int get_tokens(char** str1, char** str2, char** str3, char* msg);

//Append numb to the end of str and save to final_str
//allocating the neccessary space with malloc
void create_nth_str(char **final_str, char* str, int numb);

//split string str by line and insert each substring to queue q
//allocating neccessary space for each substring with malloc
void split_str_by_line(queue<char*> *q, char* str);