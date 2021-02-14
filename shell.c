#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <setjmp.h>
#include <signal.h>


static sigjmp_buf jmpCode;

void sigHandler(int signo) {
    siglongjmp(jmpCode, 42);
}

char command_buf[500] , *parsedArgs[100];
char* parsedArgsPiped[100];
int flag=0;
char dir[100];

void welcome(){

	printf("********************************\n\n\n\n");
	printf("Welcome to ArashShell!\n\n\n\n");
	printf("********************************\n");	
	
}
 void system_cmd(char** parsed) 
{ 

	pid_t pid = fork(); 

	if (pid == -1) { 
		printf("\nFailed forking child.."); 
		return; 
	} else if (pid == 0) { 
		signal(SIGINT, SIG_DFL);
		if (execvp(parsed[0], parsed) < 0) { 
			printf("Invalid command!\n"); 
		} 
		exit(0); 
	} else { 
		
		wait(NULL); 
		return; 
	} 
} 


void pipe_cmd(char** parsed, char** parsedpipe) 
{ 

	int pipefd[2]; 
	pid_t p1, p2; 

	if (pipe(pipefd) < 0) { 
		printf("\nPipe could not be initialized"); 
		return; 
	} 
	p1 = fork(); 
	if (p1 < 0) { 
		printf("\nCould not fork"); 
		return; 
	} 

	if (p1 == 0) { 
		signal(SIGINT, SIG_DFL);
		close(pipefd[0]); 
		dup2(pipefd[1], STDOUT_FILENO); 
		close(pipefd[1]); 

		if (execvp(parsed[0], parsed) < 0) { 
			printf("Command Not Found\n"); 
			exit(0); 
		} 
	} else { 
		 
		p2 = fork(); 

		if (p2 < 0) { 
			printf("\nCould not fork"); 
			return; 
		} 


		if (p2 == 0) { 
			signal(SIGINT, SIG_DFL);
			close(pipefd[1]); 
			dup2(pipefd[0], STDIN_FILENO); 
			close(pipefd[0]); 
			if (execvp(parsedpipe[0], parsedpipe) < 0) { 
				printf("Could not execute command 2..\n"); 
				exit(0); 
			} 
		} else { 
		
			wait(NULL); 
			wait(NULL); 
		} 
	} 
} 
int parsePipe(char* str, char** strpiped) 
{ 
	int i; 
	for (i = 0; i < 2; i++) { 
		strpiped[i] = strsep(&str, "|"); 
		if (strpiped[i] == NULL) 
			break; 
	} 

	if (strpiped[1] == NULL) 
		return 0; 
	else { 
		return 1; 
	} 
} 


void parseSpace(char* str, char** parsed) 
{ 
	int i; 

	for (i = 0; i < 100; i++) { 
		parsed[i] = strsep(&str, " "); 

		if (parsed[i] == NULL) 
			break; 
		if (strlen(parsed[i]) == 0) 
			i--; 
	} 
} 

int processString(char* str, char** parsed, char** parsedpipe) 
{ 

	char* strpiped[2]; 
	int piped = 0; 

	piped = parsePipe(str, strpiped); 

	if (piped) { 
		parseSpace(strpiped[0], parsed); 
		parseSpace(strpiped[1], parsedpipe); 

	} else { 

		parseSpace(str, parsed); 
	} 


		return 1 + piped; 
} 


 
int main_loop() {

	
	signal(SIGINT, sigHandler);
	getcwd(dir, 100);
	while(1){
		if (sigsetjmp(jmpCode,1) == 42) {
			printf("\n");
		}
		printf("ArashPrompt:%s> ",dir);
		gets(command_buf);
		flag = processString(command_buf,parsedArgs, parsedArgsPiped);
	
		if(!strcmp(command_buf,"quit")){
			exit(0);
		}
		else if(!strcmp(command_buf,"cd")){
			char* cdPointer = command_buf;
			cdPointer += 3;
			if (!chdir(cdPointer)){
				getcwd(dir, 100);
				continue;
			} else {
				printf("path not found\n");
				continue;
			}
		}
		
		if(!strcmp(command_buf,"sendmsg")){
			writer();
			
		}else if(!strcmp(command_buf,"seemsg")){
			reader();
						
		}else{
				
			if(flag == 1){
				system_cmd(parsedArgs);
			}
		
			if (flag == 2){		
				pipe_cmd(parsedArgs,parsedArgsPiped);
			}			
		}					
	}
}	


void reader() 
{ 
	int fd1; 

	char * myfifo = "/tmp/myfifo"; 

	 
	mkfifo(myfifo, 0666); 

	char str1[80], str2[80]; 
	while (1) 
	{ 
	
		fd1 = open(myfifo,O_RDONLY); 
		read(fd1, str1, 80); 

		
		printf("User1: %s\n", str1); 
		close(fd1); 

		fd1 = open(myfifo,O_WRONLY); 
		fgets(str2, 80, stdin); 
		write(fd1, str2, strlen(str2)+1); 
		close(fd1); 
	} 
	
} 
void writer() 
{ 
	int fd; 


	char * myfifo = "/tmp/myfifo"; 

 
	mkfifo(myfifo, 0666); 

	char arr1[80], arr2[80], quit[80]; 
	while (1) 
	{ 
		 
		fd = open(myfifo, O_WRONLY); 

		fgets(arr2, 80, stdin);
		
		if( !strcmp(arr2,"quit")){
			exit(0);
		}
		
	

		write(fd, arr2, strlen(arr2)+1); 
		close(fd); 

		 
		fd = open(myfifo, O_RDONLY); 

	 
		read(fd, arr1, sizeof(arr1)); 

	 
		printf("User2: %s\n", arr1); 
		close(fd); 
	} 
	
} 

int main(int argc, char* argv[]) {
	
	welcome();
	if (argc == 1) {
		main_loop();
	} else if (argc == 2) {
		FILE *file = fopen(argv[1], "r");
		char line[512];
		while (fgets(line,512,file))
		{
			printf("%s",line);
			line[strlen(line) - 1] = '\0';
			
			flag = processString(line,parsedArgs, parsedArgsPiped);	
		
			if(!strcmp(line,"quit")){
				exit(0);
			}
			
			if(!strcmp(line,"sendmsg")){
				writer();
				
			}else if(!strcmp(line,"seemsg")){
				reader();
							
			}else{
					
				if(flag == 1){
					system_cmd(parsedArgs);
				}
			
				if (flag == 2){		
					pipe_cmd(parsedArgs,parsedArgsPiped);
				}			
			}					
		}
		
	} else {
		fprintf(stderr, "too much args!!\n");
		exit(1);
	}
	  
   return 0;
}

