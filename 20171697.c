#include "20171697.h"

int main (void){
	Init();
	while(!Exit_flag){
		switch(Get_Command()){
			case HELP: help(); break;
			case DiR: show_files(); break;
			case QUIT: Exit_flag = TRUE; break;
			case HISTORY: show_history(); break;
			case DUMP: mem_dump(); break;
			case EDIT: mem_edit(); break;
			case FILL: mem_fill(); break;
			case RESET: mem_reset(); break;
			case OPCODE: opcode(); break;
			case OPCODELIST: opcodelist(); break;
			default: Success = FALSE;break;
		}
		if(Success)
			store_input();
		else{
			free(new_input);
			new_input = NULL;
			printf("Error: invalid input\n");
		}
	}
	end_program();
}

void Init(){
	Exit_flag = FALSE;
	History_Head = NULL;
	Memory = (unsigned char *)calloc(MEM_SIZE,sizeof(unsigned char));
	last_mem_idx = -1;

	command_list[0][0] = "h";
	command_list[0][1] = "help";
	command_list[1][0] = "d";
	command_list[1][1] = "dir";
	command_list[2][0] = "q";
	command_list[2][1] = "quit";
	command_list[3][0] = "hi";
	command_list[3][1] = "history";
	command_list[4][0] = "du";
	command_list[4][1] = "dump";
	command_list[5][0] = "e";
	command_list[5][1] = "edit";
	command_list[6][0] = "f";
	command_list[6][1] = "fill";
	command_list[7][0] = "reset";
	command_list[7][1] = "\0";
	command_list[8][0] = "opcode";
	command_list[8][1] = "\0";
	command_list[9][0] = "opcodelist";
	command_list[9][1] = "\0";

	Make_hash_table();
}

int Hash_func(char mnemonics[]){
	int idx = 0;
	int total = 0;
	int ret;

	while(1){
		if(mnemonics[idx] == '\0')
			break;
		total += (mnemonics[idx]-'A');
		idx++;
	}

	ret = (total % HASH_TABLE_SIZE);
	return ret;
}

void Make_hash_table(){
	char line[500];
	char hex_str[3];
	FILE *fp;
	int i;
	int store_adr;
	unsigned int opc;
	opcode_info *new_info;

	for(i = 0; i < HASH_TABLE_SIZE ; i++)
		Hash_Table[i] = NULL;

	fp = fopen("opcode.txt","r");
	if(fp == NULL){
		printf("Error: cannot read opcode file\n");
		Exit_flag = TRUE;
		return;
	}
	while(!feof(fp)){
		fgets(line,500,fp);
		rd_pt = 0;
		new_info = (opcode_info *)malloc(sizeof(opcode_info));
		Handling_Input(0,line,hex_str,2);
		Str_convert_into_Hex(hex_str, 2, &opc);
		new_info->opcode = (unsigned char)opc;
		Handling_Input(1,line,NULL,0);
		Handling_Input(0,line,new_info->mnemonics,6);
		store_adr = Hash_func(new_info->mnemonics);
		new_info->next = Hash_Table[store_adr];
		Hash_Table[store_adr] = new_info;
	}
	fclose(fp);
}
int Handling_Input(int mode,char input_str[], char str[], int len){
	/*mode=0: store until meeting blank
	  mode=1: erase blanks
	  return TRUE: meet '\n'
	  return FALSE: not meet '\n'
	  */
	int idx = 0;
	int ret;
	int sig_exist = FALSE;
	char ch;
	while(1){
		ch = input_str[rd_pt];
		if(ch == '\n'){
			ret = ENTER;
			break;
		}
		else if(ch == ','){
			ret = COMMA;
			rd_pt++;
			break;
		}
		if((ch == ' ' || ch == '\t' || ch == '\r' || ch == '\v') != mode){
			if( mode == 0 )
				ret = BLANK;
			else
				ret = CHAR;
			break;
		}
		if(mode == 0){
			if(idx >= len){
				ret = FALSE;
				break;
			}
			else if( str[0] == '0' ){
				if((ch == '0') && (idx == 1))
					idx = 1;
				else if( ( ch == 'X' || ch == 'x') && (sig_exist == FALSE) ){
					sig_exist = TRUE;
					idx = 0;
				}
				else{
					str[idx] = ch;
					idx++;
				}
			}
			else{
				str[idx] = ch;
				idx++;
			}
		}
		rd_pt++;
	}
	if(mode == 0)
		str[idx]='\0';
	return ret;
}
int Get_Command(){
	int idx = command_num;
	int ret;
	char command[11];

	Success = TRUE;
	new_input = (input*)malloc(sizeof(input));
	new_input->next = NULL;

	printf("sicsim> ");
	fgets(new_input->str,MAX_COMMAND,stdin);
	rd_pt = 0;

	ret = Handling_Input(1,new_input->str,NULL,0); //erase blank before command
	if(ret != CHAR){
		Success = FALSE;
		return idx;
	}
	ret =  Handling_Input(0,new_input->str,command,10);// store command
	if( ret == COMMA || ret == FALSE){
		Success = FALSE;
		return idx;
	}

	for(idx = 0 ; idx < command_num ; idx++){
		if(!(strcmp(command_list[idx][0],command)&&strcmp(command_list[idx][1],command))){
			return idx;
		}
	}
	return idx;
}

void help(){
	int idx;
	int len;

	if( Handling_Input(1,new_input->str,NULL,0) != ENTER ){
		Success = FALSE;
		return;
	}

	for( idx = 0 ; idx < command_num ; idx++ ){
		printf("%s", command_list[idx][0]);
		len = strlen(command_list[idx][0]);
		if( command_list[idx][1] != "\0" ){
			printf("[");
			while( command_list[idx][1][len] != '\0' ){
				printf("%c", command_list[idx][1][len]);
				len++;
			}
			printf("]");
		}
		if( idx == DUMP ){
			printf(" [start, end]");
		}
		else if( idx == EDIT ){
			printf(" address, value");
		}
		else if( idx == FILL ){
			printf(" start, end, value");
		}
		else if ( idx == OPCODE ){
			printf(" mnemonic");
		}
		printf("\n");
	}
}
void show_files(){
	DIR *current_dir; // directory stream
	struct dirent *dr_st; //dirent structure
	struct stat stat_st; //stat structure
	int file_type;
	int file_permission;

	if( Handling_Input(1,new_input->str,NULL,0) != ENTER ){
		Success = FALSE;
		return;
	}

	if ((current_dir = opendir(".")) == NULL){
		printf("Error: cannot open current directory\n");
		return;
	}

	while(1){
		if((dr_st = readdir(current_dir)) == NULL)
			break;
		stat(dr_st->d_name,&stat_st);
		file_type = stat_st.st_mode & S_IFMT;
		printf("%s",dr_st->d_name);
		if(file_type == S_IFDIR){
			printf("/");
		}
		else if((stat_st.st_mode & S_IXUSR) != 0){
			printf("*");
		}
		else if((stat_st.st_mode & S_IXGRP) != 0){
			printf("*");
		}
		else if((stat_st.st_mode & S_IXOTH) != 0){
			printf("*");
		}
		printf("\n");
	}
	closedir(current_dir);
}
void show_history(){
	input *cur;
	int num=1;

	if( Handling_Input(1,new_input->str,NULL,0) != ENTER ){
		Success = FALSE;
		return;
	}

	cur = History_Head;
	while(cur != NULL){
		printf("%5d     %s",num,cur->str);
		cur = cur->next;
		num++;
	}
	printf("%5d     %s",num,new_input->str);
}
int Str_convert_into_Hex(char str[], int len_limit, unsigned int *num){
	int hex_len = 0;
	int idx = 0;
	int ret = TRUE;
	unsigned int number = 0;
	
	if(str[0] == '\0')
		ret = FALSE;
	while(ret){
		if(str[idx] == '\0')
			break;
		else if(hex_len >= len_limit){
			ret = FALSE;
			break;
		}
		else if( str[idx] >= '0' && str[idx] <= '9' ){
			number *= 16;
			number += (str[idx] - '0');
		}
		else if ( str[idx] >= 'a' && str[idx] <= 'f' ){
			number *= 16;
			number += (str[idx] - 'a' + 10);
		}
		else if ( str[idx] >= 'A' && str[idx] <= 'F' ){
			number *= 16;
			number += (str[idx] - 'A' + 10);
		}
		else
			ret = FALSE;
		idx++;
		hex_len++;
	}
	*num = number;
	return ret;
}
void Hex_convert_into_Str(unsigned int num, int len){
	char str[6];
	int idx;
	int remain;

	str[len] = '\0';
	for(idx = (len-1) ; idx >= 0 ; idx-- ){
		remain = num % 16;
		if( remain < 10 )
			str[idx] = remain + '0';
		else
			str[idx] = (remain - 10) + 'A';
		num /= 16;
	}
	printf("%s",str);
}
int Get_argument(unsigned int *arg1, unsigned int *arg2, unsigned int *arg3, int arg_len[]){
	int arg_num = 0;
	int Max_arg_num = 3;
	int i;
	int ret;
	int ret3, ret2, ret1;
	char argument[3][8];
	ret3 = ret2 = ret1 = TRUE;

	for( i = 0 ; i < Max_arg_num ; i++ ){
		ret = Handling_Input(1,new_input->str,NULL,0);
		if(ret == ENTER){
			if(i != 0)
				Success = FALSE;
			return arg_num;
		}
		else if(ret == COMMA){
			Success = FALSE;
			return arg_num;
		}
		ret = Handling_Input(0,new_input->str,argument[i],arg_len[i]);
		arg_num++;
		if(ret == BLANK){
			ret = Handling_Input(1,new_input->str,NULL,0);
		}
		if(ret == ENTER){
			switch(arg_num){
				case 3: ret3  = Str_convert_into_Hex( argument[2], arg_len[2], arg3);
				case 2: ret2  = Str_convert_into_Hex( argument[1], arg_len[1], arg2);
				case 1: ret1 = Str_convert_into_Hex( argument[0], arg_len[0], arg1);
			}
			Success = ( ret3 && ret2 && ret1 );
			return arg_num;
		}
		else if(ret == CHAR || ret == FALSE){
			Success = FALSE;
			return arg_num;
		}
	}
	if(ret != ENTER)
		Success = FALSE;
	return arg_num;
}
void mem_dump(){
	int ret;
	unsigned int start_m;
	unsigned int end_m;
	unsigned int tmp;
	unsigned int start_p;
	unsigned int end_p;
	int print_size = 160;
	int dump_case = 0;
	int idx;
	int r;
	int arg_len[3];
	arg_len[0] = arg_len[1] = 5;
	arg_len[2] = 2;

	dump_case = Get_argument(&start_m, &end_m, &tmp, arg_len);
	if( !Success )
		return;

	switch(dump_case){
		case 0:{
				   start_m = last_mem_idx + 1;
				   if( start_m > MEM_LIMIT)
					   start_m = 0;
				   start_p = 16 * (start_m/16);
				   end_m = 160 + start_m - 1;
				   if( end_m > MEM_LIMIT )
					   end_m = MEM_LIMIT;
				   end_p = 16 * (end_m/16);
				   break;
			   }
		case 1:{
				   start_p = 16 * (start_m/16);
				   end_m = 160 + start_m - 1;
				   if( end_m > MEM_LIMIT )
					   end_m = MEM_LIMIT;
				   end_p = 16 * (end_m/16);
				   break;

			   }
		case 2:{
				   if( start_m > end_m ){
					   Success = FALSE;
					   return;
				   }
				   start_p = 16 * (start_m/16);
				   end_p = 16 * (end_m/16); 
			   }
	}

	for( idx = start_p ; idx <= end_p ; idx += 16){
		Hex_convert_into_Str((unsigned int)idx,5);
		printf(" ");
		for( r = idx ; r < (idx + 16) ; r++ ){
			if( (r > end_m) || (r < start_m) )  
				printf("   ");
			else{
				Hex_convert_into_Str( (unsigned int)Memory[r], 2);
				printf(" ");
			}
		}
		printf("; ");
		for( r = idx ; r < (idx + 16) ; r++ ){
			if( (r > end_m) || (r < start_m) )  
				printf(".");
			else if( (Memory[r] >= 32) && (Memory[r] <= 126) )
				printf("%c",Memory[r]);
			else
				printf(".");
		}
		printf("\n");
	}
	last_mem_idx = end_m;
}

void mem_edit(){
	unsigned int address;
	unsigned int value;
	unsigned int tmp;
	int arg_num;
	int arg_len[3];
	arg_len[0] = 5;
	arg_len[1] = 2;
	arg_len[2] = 0;
	
	arg_num = Get_argument(&address, &value, &tmp, arg_len);
	if( arg_num != 2)
		Success = FALSE;
	else if(Success)
		Memory[address] = (unsigned char)value;
}

void mem_fill(){
	unsigned int start_m;
	unsigned int end_m;
	unsigned int value;
	int arg_num;
	int idx;
	int arg_len[3];
	arg_len[0] = arg_len[1] = 7;
	arg_len[2] = 4;

	arg_num = Get_argument(&start_m, &end_m, &value, arg_len);
	if( arg_num != 3)
		Success = FALSE;
	else if(Success){
		if(start_m > end_m)
			Success = FALSE;
		else{
			for( idx = start_m ; idx<= end_m ; idx++ )
				Memory[idx] = (unsigned char)value;
		}
	}
}

void mem_reset(){
	int idx;
	for( idx = 0; idx <= MEM_LIMIT ; idx++)
		Memory[idx] = 0;
}
void opcode(){
	int ret;
	int adr;
	opcode_info *cur;
	char mnem[7];

	ret = Handling_Input(1,new_input->str,NULL,0);
	if(ret != CHAR){
		Success = FALSE;
		return;
	}
	ret = Handling_Input(0,new_input->str, mnem,6);
	if(ret == FALSE){
		Success = FALSE;
		return;
	}
	else if(ret == BLANK)
		ret = Handling_Input(1,new_input->str,NULL,0);
	if(ret != ENTER){
		Success = FALSE;
		return;
	}
	adr = Hash_func(mnem);
	if( adr < 0 ){
		Success = FALSE;
		return;
	}
	cur = Hash_Table[adr];
	while(cur != NULL){
		if( strcmp(cur->mnemonics, mnem) == 0)
			break;
		cur = cur->next;
	}
	if( cur == NULL){
		Success = FALSE;
		return;
	}
	printf("opcode is ");
	Hex_convert_into_Str((unsigned int)cur->opcode,2);
	printf(".\n");
}
void opcodelist(){
	int ret;
	int idx;
	opcode_info *cur;
	ret = Handling_Input(1,new_input->str,NULL,0);
	for( idx = 0 ; idx < HASH_TABLE_SIZE ; idx++){
		printf("%d : ",idx);
		cur = Hash_Table[idx];
		while(1){
			printf("[%s, ",cur->mnemonics);
			Hex_convert_into_Str((unsigned int)cur->opcode,2);
			printf("]");
			cur = cur->next;
			if(cur == NULL){
				printf("\n");
				break;
			}
			printf(" -> ");
		}
	}
}

void store_input(){
	if(History_Head == NULL){
		History_Head = new_input;
		History_Tail = History_Head;
	}
	else{
		History_Tail->next = new_input;
		History_Tail = new_input;
	}
}
void end_program(){
	int idx;
	opcode_info *prev;
	opcode_info *current;
	input *cur;
	input *after;
	
	after = cur = History_Head;
	while(after != NULL){
		cur = after;
		after = cur->next;
		free(cur);
		cur = NULL;
	}

	free(Memory);
	Memory = NULL;

	for( idx = 0 ; idx < HASH_TABLE_SIZE ; idx++ ){
		prev = current = Hash_Table[idx];
		while(current != NULL){
			prev = current;
			current = current->next;
			free(prev);
			prev = NULL;
		}
	}

}
