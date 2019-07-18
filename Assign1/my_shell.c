#include  <stdio.h>
#include  <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>


#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

int a[100], i=0;

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

//Function for foreground execution
void foreground(char ** tokens){
	int pid;
		if(strcmp(tokens[0], "cd")==0){
				chdir(tokens[1]);}

		pid=fork();
		if(pid==-1){
			printf("Error in forking child\n");
		}
		else if(pid==0){
			if(execvp(tokens[0], tokens)<0 && strcmp(tokens[0], "cd")!=0){
				printf("Shell: Incorrect command\n");
			}
			exit(0);
		}
		else{
			wait(NULL); 
		}
}

void serial_foreground(char ** tokens){
	int i,j,k;
	int prev_i=0;
  	for(i=0;tokens[i]!=NULL;i++){
  		if(strcmp(tokens[i] ,"&&")==0 ){
  			char **tokens1 = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  			for(k=0,j=prev_i;j<i;j++){
  				tokens1[k++]=tokens[j];}
  			foreground(tokens1);
  			prev_i=i+1;	
  			free(tokens1);
		}
		else if(tokens[i+1]==NULL){
			char **tokens1 = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  			for(k=0,j=prev_i;j<=i;j++){
  				tokens1[k++]=tokens[j];}
  				foreground(tokens1);
  				free(tokens1);
		}

	}
}

void background(char ** tokens){
	int pid,i;
	for(i=0;tokens[i]!=NULL;i++){
  		if(strcmp(tokens[i] , "&")==0){tokens[i]=NULL;}
  	}	
	if(strcmp(tokens[0], "cd")==0){
		chdir(tokens[1]);
		return;
	}
	pid=fork();	
	if(pid==0){
	setpgid(0,0);
	if(execvp(tokens[0], tokens)<0 && strcmp(tokens[0], "cd")!=0){
	printf("Shell: Incorrect command\n");
	}
	exit(0);
	}
	else{
		a[i++]=pid;
		return; 
	}
	printf("\n");

}

void parallel_background(char ** tokens){
	int i,j,k;
	int prev_i=0;
  	for(i=0;tokens[i]!=NULL;i++){
  		if(strcmp(tokens[i] ,"&&&")==0 ){
  			char **tokens1 = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  			for(k=0,j=prev_i;j<i;j++){
  				tokens1[k++]=tokens[j];}
  			background(tokens1);
  			prev_i=i+1;	
  			free(tokens1);
		}
		else if(tokens[i+1]==NULL){
			char **tokens1 = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  			for(k=0,j=prev_i;j<=i;j++){
  				tokens1[k++]=tokens[j];}
  				background(tokens1);
  				free(tokens1);
		}

	}
}

void handle_sigint(int sig){
	printf("%s\n","child killed" );
}

void exitprocess(){
	for(int l=0; l<i; ++l){
		kill(a[l],SIGKILL);
	}
	exit(0);
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;
	int pid;
	int no_of_ands=0;
	int flag=0;
	int status;

	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r");
		if(fp < 0) {
			printf("File doesn't exists.");
			return -1;
		}
	}

	while(1) {
		signal(SIGINT, handle_sigint);			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}
			line[strlen(line) - 1] = '\0';
		} else { // interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}
		/* END: TAKING INPUT */
		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);


		for(i=0;tokens[i]!=NULL;i++){
			if(strcmp(tokens[i], "&")==0){no_of_ands=1; break;}
			if(strcmp(tokens[i], "&&")==0){no_of_ands=2; break;}
			if(strcmp(tokens[i], "&&&")==0){no_of_ands=3; break;}
			if(strcmp(tokens[i], "exit")==0){exitprocess();}
		}

		pid = waitpid(-1, &status, WNOHANG);
		//pid=wait(NULL);
		if(pid>0){printf("%s\n", "Shell: Background process finished");}

		switch(no_of_ands){
			case 1:	background(tokens);
					break;

			case 2:	serial_foreground(tokens);
					break;

			case 3:	parallel_background(tokens);
					break;

			default:foreground(tokens);
					break;
		}
		
		//Executing the command;
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
		no_of_ands=0;
		int a= waitpid(-1,&status,WNOHANG);
	}
	return 0;
}