#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include "userCommand.h"
#define CMDLINE_MAX 512

int mySystem(struct command input);
struct command split(char* str,char* character);
int redirection(struct command input);
bool existence(char* str,char searchingCharacter);//searches for the existence of the character
void returnString(char* str,char* character);//add spaces before and after 'character' in the string
int pipingFunction(struct command pipingInput,char* originalStr,int numberOfArguments);
int environmentVariables(struct command *input,char arrayOfStrings[26][CMDLINE_MAX]);
void initializeArrayOfStrings(char* arrayOfStrings[]);
bool pipingMissingCommand(char * str,int numberOfArguments);
int main(void)
{
        char cmd[CMDLINE_MAX];
	char arrayOfStrings[26][CMDLINE_MAX];
	
	
        while (1) {
                char *nl;
                int retval;

                /* Print prompt */
                printf("sshell$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';
		if(cmd[0]=='\0'){
			continue;
		}	
		//Storing the original entered input in order to modify cmd
		char originalCmd[CMDLINE_MAX];
		strcpy(originalCmd,cmd);

		//checking for the existence of '|' or '>' characters
		bool pipingExist=existence(cmd,'|');
	
	
		bool redirectionCharacter=existence(cmd,'>');
     
	     //adding voluntary spaces before and after '>' and '|' symbols
	     
		
		if(redirectionCharacter){
			returnString(cmd,">");
		}			
		if(pipingExist){
			returnString(cmd,"|");	
			struct command pipingInput=split(cmd,"|");
			pipingInput.mislocatedOutputRedirection=false;
			int numberOfArguments=0;
			while(pipingInput.args[numberOfArguments]!=NULL){
			if((pipingInput.args[numberOfArguments+1]!=NULL) && (existence(pipingInput.args[numberOfArguments],'>'))){
				pipingInput.mislocatedOutputRedirection=true;
			}
			numberOfArguments++;
		}
			if(pipingInput.mislocatedOutputRedirection){
				fprintf(stderr,"Error: mislocated output redirection\n");
				continue;
			}
			if(pipingMissingCommand(originalCmd,numberOfArguments)){
				fprintf(stderr,"Error: missing command\n");
				continue;
				}

			int x=pipingFunction(pipingInput,originalCmd,numberOfArguments);
		
			
			fprintf(stderr," [%d]\n",x);
			continue;

		}
		
	      
	       struct command input=split(cmd," ");
	
	       if(input.tooManyArguments){
			fprintf(stderr,"Error: too many arguments\n");
			continue;
		}else if(input.missingCommand){
			fprintf(stderr,"Error: missing command\n");
			continue;
		}else if(input.noOutputFile){
			fprintf(stderr,"Error: no output file\n");
			continue;
		}else if(redirectionCharacter){
			int locationOfRedirection=redirection(input);
			if(locationOfRedirection<0){
				fprintf(stderr,"Error: no output file\n");
				continue;
			}else if(input.args[locationOfRedirection+1]==NULL){
				fprintf(stderr,"Error: no output file\n");
				continue;
			}
		}
		//replace the arguments with $alphabet with strings from the arrayOfStrings
		int properEnvironmentVariables=environmentVariables(&input,arrayOfStrings);
		if(properEnvironmentVariables==1){
			continue;
		}
	
	         
	       	/* Builtin command */

                if (!strcmp(input.fileName, "exit")) {
                        fprintf(stderr, "Bye...\n");
			fprintf(stderr,"+ completed 'exit' [0]\n");
                        exit(0);
                }else if(!strcmp(input.fileName,"pwd")){
			char cwd[PATH_MAX];
			getcwd(cwd,sizeof(cwd));
			printf("%s\n",cwd);
			fprintf(stderr,"+ completed 'pwd' [0]\n");
			continue;
			
		
		}else if(!strcmp(input.fileName,"cd")){
			int chdirStatus=chdir(input.args[1]);
			if(chdirStatus==-1){
				fprintf(stderr,"Error: cannot cd into directory\n");
				fprintf(stderr,"+ completed '%s' [1]\n",originalCmd);
				continue;
			}
			fprintf(stderr,"+ completed '%s' [%d]\n",originalCmd,chdirStatus);
			continue;

			//for set
		}else if(!strcmp(input.fileName,"set")){
			char setValue[CMDLINE_MAX];
			strcpy(setValue,input.args[2]);
			if(input.args[1]==NULL){
				fprintf(stderr,"Error: invalid variable name\n");
				fprintf(stderr,"+ completed '%s' [1]\n",originalCmd);
				continue;
			}
			if(strlen(input.args[1])>1){
				fprintf(stderr,"Error: invalid variable name\n");
				fprintf(stderr,"+ completed '%s' [1]\n",originalCmd);
				continue;
			}	
			if((input.args[1][0]-97)<0 || (input.args[1][0]-97)>25){
				fprintf(stderr,"Error: invalid variable name\n");
				fprintf(stderr,"+ completed '%s' [1]\n",originalCmd);
				continue;
			}
			if(setValue==NULL){
				strcpy(arrayOfStrings[input.args[1][0]-97],"");
				continue;
			}
			strcpy(arrayOfStrings[input.args[1][0]-97],setValue);
			fprintf(stderr,"+ completed '%s' [0]\n",originalCmd);
			continue;
		}
				

	
                // Regular command 
                retval = mySystem(input);
		
		if(retval==-3){
			continue;
		}
                fprintf(stderr, "+ completed '%s' [%d]\n",
		originalCmd, retval);

		}

        return EXIT_SUCCESS;
}


int environmentVariables(struct command *input,char arrayOfStrings[26][CMDLINE_MAX]){
	int i=0;
	while(input->args[i]!=NULL){
		
		if(existence(input->args[i],'$')){
			if(strlen(input->args[i])>2){
				fprintf(stderr,"Error: invalid variable name\n");
				return 1;
			}
			if((input->args[i][1]-97)<0 || (input->args[i][1]-97)>25){
				fprintf(stderr,"Error: invalid variable name\n");
				return 1;
			}
			strcpy(input->args[i],arrayOfStrings[input->args[i][1]-97]);
		
		}
		i++;
	
	}
	
	return 0;
}
int mySystem(struct command input){
	pid_t pid;
	
	pid = fork();
	if(pid==0){
		//child
		
		int redirectionCharacter=redirection(input);
		if(redirectionCharacter>-1){
			int fd;
			fd=open(input.args[redirectionCharacter+1],O_WRONLY | O_CREAT | O_TRUNC,0644);
			
			if(fd==-1){
				fprintf(stderr,"Error: cannot open output file\n");
				return -3;
			}
			dup2(fd,STDOUT_FILENO);
			close(fd);	
			input.args[redirectionCharacter]=NULL;
		}
		
		
		execvp(input.fileName,input.args);
		fprintf(stderr,"Error: command not found\n");
		
		exit(1);
	}else if(pid>0){
		//parent
		int status;
		waitpid(pid,&status,0);
		return(WEXITSTATUS(status));
	}else{
		perror("fork");
		exit(1);
	}


}

bool pipingMissingCommand(char* str,int numberOfArguments){
	int i=0;
	int numberOfPipes=0;
	while(str[i]!='\0'){
		if(str[i]=='|'){
			numberOfPipes++;
		}
		i++;
	}
	if(numberOfArguments-1<numberOfPipes){
		return true;
	}else{
		return false;
}
}
struct command split(char* str,char* character){

struct command splitStr;


	char* token=strtok(str,character);
	splitStr.fileName=token;
	
	int i=0;	
	splitStr.args[i]=token;
	
	splitStr.tooManyArguments=false;
	splitStr.missingCommand=false;	
	splitStr.noOutputFile=false;
	while(token != NULL){
		
		i++;
		if(i>16){
			
			splitStr.tooManyArguments=true;
			break;
		}
			
		token=strtok(NULL,character);
		splitStr.args[i]=token;
			
	}
	

	
	if(!strcmp(splitStr.fileName,">")){
			splitStr.missingCommand=true;
			}	
	
	
	
	return splitStr;
	
	}
int redirection(struct command input){
	
	for(int i=0; i<20;i++){
		if(input.args[i]==NULL){
			return -1;
		}else if(!strcmp(input.args[i],">")){
			return i;
		}
	}
	return -1;

}
bool  existence(char* str,char searchingCharacter){
	char* str_pointer=strchr(str,searchingCharacter);
	int position=str_pointer - str;
	char myStr[CMDLINE_MAX];
	strcpy(myStr,str);
	if(position<100 && position>-1){
		return true;
	}
	return false;



}

void returnString(char* str,char* character){
	

 	char s[2];
 	strcpy(s,character);
	char* token;

	token=strtok(str,s);
	char* redirectionMessage=token;


	char fileName[3][20];
	int i=0;
	while(token!=NULL){
	

		if(i==1){
			strcpy(fileName[0],token);
		}
		if(i==2){
			strcpy(fileName[1],token);
		}
		if(i==3){
			strcpy(fileName[2],token);
		}

		token=strtok(NULL,s);
		i++;
	}





	for(int x=0;x<(i-1);x++){
		strcat(redirectionMessage," ");
		strcat(redirectionMessage,character);
		strcat(redirectionMessage," ");
		strcat(redirectionMessage,fileName[x]);
	}

}





int pipingFunction(struct command pipingInput,char* originalStr,int numberOfArguments){
	
	
pid_t firstPid;
firstPid=fork();
if(firstPid==0){
	if(numberOfArguments==3){

		struct command input=split(pipingInput.args[0]," ");
		struct command inputTwo=split(pipingInput.args[1]," ");
		struct command inputThree=split(pipingInput.args[2]," ");

		int fdOne[2];
		pipe(fdOne);
		int fdTwo[2];
		pipe(fdTwo);
		



		pid_t pidOne= fork();
		if(pidOne!=0){
	
			

				
				int statusTwo;
				close(fdOne[0]);
				close(fdOne[1]);
				close(fdTwo[1]);
				dup2(fdTwo[0],STDIN_FILENO);
				close(fdTwo[0]);
				waitpid(pidOne,&statusTwo,0);
						
				int statusThree=mySystem(inputThree);
				exit(statusThree);
	
		}else{
			pid_t pidTwo=fork();
			if(pidTwo!=0){
			
			
		        int status;	
			close(fdOne[1]);
			dup2(fdOne[0],STDIN_FILENO);
			close(fdOne[0]);

			close(fdTwo[0]);
			dup2(fdTwo[1],STDOUT_FILENO);
			close(fdTwo[1]);
			waitpid(pidTwo,&status,0);
			int statusTwo=mySystem(inputTwo);
	
			exit(statusTwo);
			
			}else{
				
				
					
				
			close(fdTwo[0]);
			close(fdTwo[1]);
			close(fdOne[0]);
			dup2(fdOne[1],STDOUT_FILENO);
			close(fdOne[1]);
			int status=mySystem(input);
	
			exit(status);
		
			}
		
		}
	}else if(numberOfArguments==2){
		struct command input=split(pipingInput.args[0]," ");
		struct command inputTwo=split(pipingInput.args[1]," ");

		int fdOne[2];
		pipe(fdOne);
		pid_t pid= fork();
		if(pid!=0){

			

					
			int status;
	
			close(fdOne[1]);
			dup2(fdOne[0],STDIN_FILENO);
			close(fdOne[0]);
			waitpid(pid,&status,0);
			int statusTwo=mySystem(inputTwo);
	
			
			exit(statusTwo);	
		}else{
		
		
			close(fdOne[0]);			
			dup2(fdOne[1],STDOUT_FILENO);
			
			close(fdOne[1]);
			int status=mySystem(input);
	
			
			exit(status);
			
			

		}
	}else if(numberOfArguments==4){
		struct command input=split(pipingInput.args[0]," ");
		struct command inputTwo=split(pipingInput.args[1]," ");
		struct command inputThree =split(pipingInput.args[2]," ");
		struct command inputFour=split(pipingInput.args[3]," ");

		int fdOne[2];
		pipe(fdOne);
		int fdTwo[2];
		pipe(fdTwo);
		int fdThree[2];
		pipe(fdThree);

		pid_t pid =fork();
		if(pid!=0){

		
			close(fdOne[0]);
					close(fdOne[1]);
					close(fdTwo[0]);
					close(fdTwo[1]);

					close(fdThree[1]);
					dup2(fdThree[0],STDIN_FILENO);
					close(fdThree[0]);


											
					int status;
					wait(&status);
					int ownStatus=mySystem(inputFour);
					exit(ownStatus);

		}else{
			if(fork()!=0){

					close(fdOne[0]);
					close(fdOne[1]);

					close(fdTwo[1]);
					dup2(fdTwo[0],STDIN_FILENO);
					close(fdTwo[0]);

					close(fdThree[0]);
					dup2(fdThree[1],STDOUT_FILENO);
					close(fdThree[1]);
				       int status;
					wait(&status);
					int ownStatus=mySystem(inputThree);	
					exit(ownStatus);
			}else{
				if(fork()!=0){

				close(fdThree[0]);
				close(fdThree[1]);
				
				close(fdOne[1]);
				dup2(fdOne[0],STDIN_FILENO);
				close(fdOne[0]);

				close(fdTwo[0]);
				dup2(fdTwo[1],STDOUT_FILENO);
				close(fdTwo[1]);
				int status;
				wait(&status);
				int ownStatus=mySystem(inputTwo);
				exit(ownStatus);
				}else{


close(fdTwo[0]);
			close(fdTwo[1]);
			close(fdThree[0]);
			close(fdThree[1]);
			
			close(fdOne[0]);
			dup2(fdOne[1],STDOUT_FILENO);
			close(fdOne[1]);

			int ownStatus=mySystem(input);
			exit(ownStatus);

				}
			}
		}

	}
}else if(firstPid>0){
	
	int status;
	
	waitpid(firstPid,&status,0);
	fprintf(stderr,"+ completed '%s'",originalStr);
	for(int i=1;i<numberOfArguments;i++){
		fprintf(stderr," [0]");
	}
	
	
	return(WEXITSTATUS(status));
}else{
	perror("fork");
	exit(1);
	
}
return 1;
}
