#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "header.h"
#define TRUE 1
#define FALSE 0
#define MAX_INPUT_LENGTH 1025
#define MAX_COUNTER 20
#define SUB_COMMAND_LEN 50
#define PROMPT "KaimingCui>"

void printPrompt();

/*
 *Define the struct that store input and split input
 *cmds represents commands that are seperated by pipes in input
 *input file means the name of input file, the input behind '<' 
 *output file means the name of output file, the input behind '>' or '>>'
 *isBackground means if it is running background, to see if the last char of inp
 *ut is '&' or not.
 * */
typedef struct command{
	char cmds[MAX_COUNTER][SUB_COMMAND_LEN];  //2D char array to store the command, cmds[i] will store ith sub command
	char *input_file;
	char *output_file;
	int isBackground;
	int isAppend;
}shell_cmd;


void splitByIORedirection(char *,shell_cmd *,int,int *,const int);
void ifBackground(shell_cmd *,char *,int);
void runProcess(shell_cmd *,int);

/*main entrance*/
int main(){
	int exit = FALSE;
	int invalid_input = 0;
	//the infinite loop, dealing with the input
  	while(!exit){
  		invalid_input = 0; //reset the invalid mark.
  		char cmd_in[MAX_INPUT_LENGTH];   //a char arrary stores the input from users.
  		printPrompt();    // print out the prompt.
  		fgets(cmd_in,MAX_INPUT_LENGTH,stdin);  //get the input from keyboard.  

  		char * find = strchr(cmd_in,'\n');   //replace the last char of input that would be '\n' to '\0'.
  		if(find){
    			*find = '\0';
  		}


  		if(strcmp(cmd_in,"exit") == 0||strcmp(cmd_in,"exist")==0){  //if the input is exit or exist, then end the loop and exist the shell.
     			exit = TRUE;
     			break;
  		}else if(*cmd_in == '\0'){   //if user does not enter anything, just entering a '\n', then start the loop again.
    			continue;
  		}

  
  		//check if the first char is '|' which means no command before this pipe.
  		if(cmd_in[0] == '|'){
			invalid_input = 1;
			fprintf(stderr,"#error:pipe with no proceeding command. invalid input.\n");
			continue;
  		}

  		int input_length = strlen(cmd_in);   //stores the length of input
  
  		shell_cmd cmd;  //the command struct
  		cmd.output_file = "NONE";  //innitialize the input output char pointer.
  		cmd.input_file = "NONE";
		cmd.isBackground = 0;
		cmd.isAppend = 0;	//initialize
		int init = 0; 		
		for(init = 0;init<MAX_COUNTER;init++){
			strcpy(cmd.cmds[init],"");
		}
 
  		ifBackground(&cmd,cmd_in,input_length);  //see if it is running background.

  		input_length = strlen(cmd_in); //reset the length because after background check, it may be changed.

  		//check if the last char if '|' which means no succeeding command behind that pipe.
  		// 'cat|&' is also invalid
  		if((cmd_in[input_length-1]) == '|' ){
			invalid_input = 1;
			fprintf(stderr,"#error:a pipe with no succeeding command. invalid input.\n");
			continue;
  		}
 

  		//starting seperate the input by pipes
  		char * save_pointer;
  		char * pipe_token = strtok_r(cmd_in,"|",&save_pointer); //using strtok_r because there will be another strtok, I need to save the pointer.
							  //seperate the input by pipes
  		int counter = 0;

  		while(pipe_token){
    			//check if there is "| |" like command, the pipe has no succeeding command, error ocurrs.
    			if(strcmp(pipe_token," ") == 0){
    				invalid_input = 1;
				fprintf(stderr,"#error:a pipe with no succeeding command. ");
				break;
    			}

			char *pipe_token_saved = pipe_token;  //save the token
			int if_last_pipe_token = 0; //boolean to see  if this pipe_token_saved is the last one
    			pipe_token = strtok_r(NULL,"|",&save_pointer); //next sub command behaind a pipe

			if(pipe_token == NULL){
				if_last_pipe_token = 1;  //check if last pipe token I just saved is the last one
			}
    			splitByIORedirection(pipe_token_saved,&cmd,counter,&invalid_input,if_last_pipe_token); //seperate the input by ' '
   	 		if(invalid_input == 1){
    				break;
    			}
    			counter++;
  		}
  

  		//if the command is invalid
  		if(invalid_input == 1){
			fprintf(stderr,"invalid input.\n");
			continue;
  		}
 

 		//output the result
 		int i = 0;
  		//printf("commands:");
  		//for(i = 0; i < counter; i++){    //print out the command

    			//printf("%s",cmd.cmds[i]);
			//printf("%d",strlen(cmd.cmds[i]));
    			//if(i != counter-1){ 
         			//printf(",");
     			//}

  		//}
  		//printf("\n");
  		//printf("Input file:%s\n",cmd.input_file); 
  		//printf("Output file:%s\n",cmd.output_file);
 	 	//if(cmd.isBackground){
   			// printf("Background or not:Yes\n");
  		//}
 	 	//else{
   			//printf("Background or not:No\n");
  		//}
		int numOf_subcmd = counter;
		runProcess(&cmd,numOf_subcmd);
 	}

	return 0;
}

/*
 * used to seperate the sub command by ' '
 * get the input and output file name
 * stores the sub command into the char array
 */
void splitByIORedirection(char *line,shell_cmd *cmd,int counter,int *invalid_input,const int if_last_pipe_token){
	char * inner_pointer;
 	char * token = strtok_r(line," ",&inner_pointer); //seperate the line by ' '
  	char cmd_array[SUB_COMMAND_LEN]="";  //a temp char array used to stores the sub command and its arguments
  while(token){
    	//check if there is a '&' in the command line
    	if(*token == '&'||*(token+strlen(token)-1) == '&'){
		*invalid_input = 1;
		fprintf(stderr,"#error:\"&\" must appear at the end of input. ");
		break;
    	}

    	if(*token == '<'){
		//if counter > 0, that means before this token, there is at least one pipe.
		//then there should not be a input redirection mark.
		if(counter > 0){
			*invalid_input = 1;		
			fprintf(stderr,"#error:input redirection after a pipe, make sure only the first subcommand have \"<\". ");
			break;
		}

      		//to see if the char behind '<','>' or '>>' is ' ', here ' ' should be '\0'
      		//I mean user may input the command in two ways. '<filename' or '< filename', we should recogonise both way.
      		if(*(token+1)){
        
        		cmd->input_file = token+1; //if it is not ' ',the file name is the array with starting address token+1
      		}
      		else{
			char *temp = strtok_r(NULL," ",&inner_pointer);   //if there is a  ' ', then the file name will be next token
			if(temp == NULL){
				*invalid_input = 1;
				fprintf(stderr,"#error:\"<\" is not followed by a filename. ");
				break;
			}
			else{
				cmd->input_file = temp;
			}
      		}	
    	}
    	else if(*token == '>'){
		//if it is not the last pipe token and we find a output redirection, this should be a error
     		if(if_last_pipe_token != 1){
			*invalid_input = 1;
			fprintf(stderr,"#error:output redirect before a pipe, make sure only the last subcommand has \">\". ");
			break;
		}

     		if(*(token+1)){     //check if there is a space behind '>'
        		if(*(token + 1) == '>'){          //to see if it is '>>'
				cmd->isAppend = 1;
				if(*(token+2)){		//check if there is  a space behind '>>'
					cmd->output_file = token+2;
				}
				else{
					char *temp = strtok_r(NULL," ",&inner_pointer);
					if(temp == NULL){
					*invalid_input = 1;
					fprintf(stderr,"#error:\">>\" is not followed by a filename. ");
					break;
					}
					else{
						cmd->output_file = temp;
					}
				}
        		}
       			 else{
				cmd->isAppend = 0;
         			cmd->output_file = token+1;  
        		}
		}
     		else{
			char *temp = strtok_r(NULL," ",&inner_pointer);
			if(temp == NULL){
				*invalid_input = 1;
				fprintf(stderr,"#error:\">\" is not followed by a filename. ");
				break;
			}
			else{
        			cmd->output_file = temp;
				//cmd->output_file = strtok_r(NULL," ",&inner_pointer);
			}
     		}
     
	}
   	else{
     		//link the sub command with its argument, using ' ' to seperate them.
     		strncat(cmd_array,token,strlen(token));
     		strcat(cmd_array," ");     
   	}

    	token = strtok_r(NULL," ",&inner_pointer);
    
  }  
  	//store the subcommand into the struct's 2D char array.
  	if(strcmp(cmd_array,"") == 0){
		//means the token is null, the input may be "|      |"
		*invalid_input = 1;
		fprintf(stderr,"#error:a piep with no succeeding command. ");
  	}
  	else if(*invalid_input == 0){
 		 strncpy(cmd->cmds[counter],cmd_array,strlen(cmd_array));
 		 cmd->cmds[counter][strlen(cmd_array)-1]='\0'; //fill in a end char '\0' behind the sub command
  	}
}


void printPrompt(){
	printf(PROMPT);
}

void ifBackground(shell_cmd *cmd,char *cmd_in,int input_length){
	char isBack = *(cmd_in + input_length -1);
	if(isBack == '&'){

    		cmd->isBackground = 1;
    		*(cmd_in + input_length - 1) = '\0';  //if there is a &, I also need to replace it by '\0'.
	}
  	else{
    		cmd->isBackground = 0;
  	}
}

void runProcess(shell_cmd *cmd,int numOf_subcmd){
	int i = 0;
	int dummy[numOf_subcmd];
	int pipefd[2];
	int pi = pipe(pipefd);   //pipe
	if(pi<0){
		perror("pipe");
		exit(1);
	}


	for(i = 0; i < numOf_subcmd;i++){
		//deal with each sub command as a child
		if(strcmp(cmd->cmds[i],"") != 0){
			int j = 0;
			int num_of_parameter = 0;
			for(j=0;j<strlen(cmd->cmds[i]);j++ ){
				if(cmd->cmds[i][j] == ' '){
					num_of_parameter++;
				}	
			}
			//make a array for command, the last element of this array is NULL
			char *cmd_argv[num_of_parameter+2];			
			char *para;
			int argv_counter = 0;
			para = strtok(cmd->cmds[i]," ");
			while(para){
			//	char *temp = (char *)malloc(30*sizeof(char));
			//	strncpy(temp,para,strlen(para));
			//	temp[strlen(para)] = '\0';
				cmd_argv[argv_counter] = para;
				para = strtok(NULL," ");
				argv_counter++;
			}
			
			cmd_argv[num_of_parameter+1] = NULL;
			//create child process
			if(fork() == 0){
				//if i == 0, this means it is the first subcomand, see if there is a input file redirection
				if(i == 0){
					if(strcmp(cmd->input_file,"NONE")!=0){
						int fdin;
						fdin = open(cmd->input_file,O_RDONLY|O_CREAT,0777);
						if(fdin<0){
							perror("open input file failed");
							exit(1);
						}
						if(dup2(fdin,0)<0){
							perror("dup2 input file and stdin failed.");
							exit(1);
						}
						close(fdin);
					}
				}
				//the last subcommand, see if there is a output file redirection
				if(i == numOf_subcmd -1){
					if(strcmp(cmd->output_file,"NONE")!=0){
						int fdout;
						if(cmd->isAppend != 1){
							fdout = open(cmd->output_file,O_RDWR|O_CREAT|O_TRUNC,0777);
							if(fdout < 0){
								perror("open output file failed.");
								exit(1);
							}
							if(dup2(fdout,1)<0){
								perror("dup2 output file and stdout failed.");
								exit(1);
							}
							close(fdout);	
						}else{
							fdout = open(cmd->output_file,O_RDWR|O_CREAT|O_APPEND,0777);
							if(fdout < 0){
								perror("open output file append failed.");
								exit(1);
							}
							if(dup2(fdout,1)<0){
								perror("dup2 output file append and stdout fialed.");
								exit(1);
							}
							close(fdout);							
						}
					}
				}		

				if(i == 0 && numOf_subcmd > 1){
					//printf("This %d time.",i);
					if(dup2(pipefd[1],1)!=1){
						perror("dup2");
						exit(1);
					}									
				}
				else if(i > 0){
					//printf("This %d time.",i);
					if(dup2(pipefd[0],0)!=0){
						perror("dup2");
						exit(1);
					}
				}
				//close the pipes of childs
				close(pipefd[1]);
				close(pipefd[0]);
				//execute the subcommand
				if(execvp(cmd_argv[0],cmd_argv)<0){
					perror("execvp");
					exit(1);
				}	
			}else{
				continue;
			}
			
			
		}else{
			break;
		}
	}

	close(pipefd[1]);
	close(pipefd[0]);
	int n = 0;
	//if there is a &, need to call wait.
	if(cmd->isBackground != 1){

		for(n = 0;n < numOf_subcmd;n++){
			wait(&dummy[n]);
			if(WEXITSTATUS(dummy[n])){
				fprintf(stderr,"Abnormal exit.\n");
			}
		}
	}

	
}

