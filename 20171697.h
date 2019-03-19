#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define TRUE 1
#define FALSE 0

#define HELP 0
#define DiR 1
#define QUIT 2
#define HISTORY 3
#define DUMP 4
#define EDIT 5
#define FILL 6
#define RESET 7
#define OPCODE 8
#define OPCODELIST 9

#define ENTER 1
#define COMMA 2
#define CHAR 3
#define BLANK 4

#define command_num 10
#define MAX_PATH 260
#define MAX_COMMAND 500
#define MEM_SIZE 1048576
#define MEM_LIMIT 1048575
#define HASH_TABLE_SIZE 20


typedef struct input{
	char  str[MAX_COMMAND];
	struct input *next;
}input;

typedef struct opcode_info{
	char mnemonics[7];
	unsigned char opcode;
	struct opcode_info *next;
}opcode_info;

input *History_Head;
input *History_Tail;
input *new_input;
int rd_pt; // index that indicates where I start to read input text.
char* command_list[command_num][2];
unsigned char *Memory;
int last_mem_idx;
opcode_info* Hash_Table[HASH_TABLE_SIZE];
int Exit_flag;
int Success;

void Init();
int Hash_func(char mnemonics[]);
void Make_hash_table();
int Handling_Input(int mode, char input_str[], char str[], int len);
int Get_Command();
void help(); 
void show_files(); //dir
void show_history(); //history
int Str_convert_into_Hex(char str[], int len_limit, unsigned int *num);
void Hex_convert_into_Str(unsigned int num, int len);
int Get_argument(unsigned int *arg1, unsigned int *arg2, unsigned int *arg3, int arg_len[]);
void mem_dump();
void mem_edit();
void mem_fill();
void mem_reset();
void opcode();
void opcodelist();
void store_input();
void end_program();
