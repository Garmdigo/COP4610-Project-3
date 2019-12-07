//
//  main.c
//  FAT32
//
//  Created by Samuel Parmer on 11/26/19.
//  Copyright © 2019 Samuel Parmer. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct
{
	char** tokens;
	int numTokens;
} instruction;

void addToken(instruction* instr_ptr, char* tok);
void printTokens(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);
void run(instruction * instr_ptr);

int main(int argc, const char * argv[])
{

	char* token = NULL;
	char* temp = NULL;
	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;
	while (1) {
		printf("Enter Command:");
		do {
			scanf("%ms",&token);
			temp = (char*)malloc((strlen(token) + 1) * sizeof(char));
			int i;
			int start = 0;
        addToken(&instr,token);
          start = i + 1;

		if (start < strlen(token)) {
				memcpy(temp, token + start, strlen(token) - start);
				temp[i-start] = '\0';
				addToken(&instr, temp);
			}
			free(token);
			free(temp);

			token = NULL;
			temp = NULL;


		} while ('\n' != getchar());    //until end of line is reached
		addNull(&instr);
      //  printTokens(&instr);
		run(&instr);

		clearInstruction(&instr);
	}

	return 0;
}

void FATexit(){

    //free all resources
    exit(0);
}

void info(){
    //parse boot sector, print info for each entry
}

void size(char * FILENAME){
    //print size of file in the current working dir in bytes, error if not found.

}

void ls(char * DIRNAME){
    //list name of all directories in the current dir including . and ..
}

void cd(char * DIRNAME){

}

void create(char * FILENAME){
    //create a file of size 0 in cwd
}

void  FATmkdir(char * DIRNAME){

}

void FATOpen(char * FILENAME, char * MODE){
    //open file for reading or writing. Need to maintain a table of files that are open

}

void FATClose(char * FILENAME){
    //open file for reading or writing. Need to maintain a table of files that are open

}

void FATRead(char * FILENAME, int OFFSET){

}

void FATWrite(char * FILENAME, int OFFSET){

}

void rm(char * FILENAME){

}

void FATrmdir(char * DIRNAME){

}
void addToken(instruction* instr_ptr, char* tok)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**) malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**) realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	//allocate char array for new token in new slot
	instr_ptr->tokens[instr_ptr->numTokens] = (char *)malloc((strlen(tok)+1) * sizeof(char));
	strcpy(instr_ptr->tokens[instr_ptr->numTokens], tok);

	instr_ptr->numTokens++;
}

void addNull(instruction* instr_ptr)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**)malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**)realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	instr_ptr->tokens[instr_ptr->numTokens] = (char*) NULL;
	instr_ptr->numTokens++;
}

void printTokens(instruction* instr_ptr)
{
	int i;
	printf("Tokens:\n");
	for (i = 0; i < instr_ptr->numTokens; i++) {
		if ((instr_ptr->tokens)[i] != NULL)
			printf("%s\n", (instr_ptr->tokens)[i]);
	}
}
void run(instruction * instr_ptr)
{
	int i=0;
		if ((instr_ptr->tokens)[i] != NULL)
			{
				if(strcmp(instr_ptr->tokens[i], "exit")==0)
				{
                    FATexit();

                }
					else if (strcmp(instr_ptr->tokens[i], "info")==0)
					{

                       if(instr_ptr->tokens[i+1]!=NULL)
                    {
                        printf("Error:too many commands\n");
                    }
                    }
					else if (strcmp(instr_ptr->tokens[i], "size")==0)
					{
                    if(instr_ptr->tokens[i+1]!=NULL)
                    {

                    }
                    else
                        printf("Error:not enough commands\n");

					}
					else if (strcmp(instr_ptr->tokens[i], "ls")==0)
					{
                      if(instr_ptr->tokens[i+1]!=NULL)
                        {

                        }

                        else
                        printf("Error:not enough commands\n");
					}
					else if (strcmp(instr_ptr->tokens[i], "cd")==0)
					{
                    if(instr_ptr->tokens[i+1]!=NULL)
                    {

                    }
                    else
                        printf("Error:not enough commands\n");
					}
					else if (strcmp(instr_ptr->tokens[i], "creat")==0)
				{
                    if(instr_ptr->tokens[i+1]!=NULL)
                    {

                    }
                     else
                        printf("Error:not enough commands\n");
				}
				else if (strcmp(instr_ptr->tokens[i], "mkdir")==0)
					{

					 if(instr_ptr->tokens[i+1]!=NULL)
                    {

                    }
                    else
                        printf("Error:not enough commands\n");
                    }
					else if (strcmp(instr_ptr->tokens[i], "open")==0)
					{
					 if(instr_ptr->tokens[i+1]!=NULL&&instr_ptr->tokens[i+2]!=NULL)
                    {

                    }
                      else
                        printf("Error:not enough commands");
					}
					else if (strcmp(instr_ptr->tokens[i], "close")==0)
					{
                    if(instr_ptr->tokens[i+1]!=NULL)
                    {

                    }
                     else
                        printf("Error:not enough commands\n");
					}
					else if (strcmp(instr_ptr->tokens[i], "read")==0)
					{
                    if(instr_ptr->tokens[i+1]!=NULL&&instr_ptr->tokens[i+2]!=NULL&&instr_ptr->tokens[i+3]!=NULL)
                    {

                    }
                    else
                        printf("Error:not enough commands");
						}
					else if (strcmp(instr_ptr->tokens[i], "write")==0)
					{
					 if(instr_ptr->tokens[i+1]!=NULL&&instr_ptr->tokens[i+2]!=NULL&&instr_ptr->tokens[i+3]!=NULL)
                    {

                    }
                     else
                        printf("Error:not enough commands\n");
					}
					else if (strcmp(instr_ptr->tokens[i], "rm")==0)
					{
                    if(instr_ptr->tokens[i+1]!=NULL)
                    {

                    }
                    else
                        printf("Error:not enough commands\n");
					}
                    else if (strcmp(instr_ptr->tokens[i], "rmdir")==0)
					{
                    if(instr_ptr->tokens[i+1]!=NULL)
                    {

                    }
                    else
                        printf("Error:not enough commands\n");
					}
					else
                        printf("No valid commmand given. Try again.\n");
			}

}
void clearInstruction(instruction* instr_ptr)
{
	int i;
	for (i = 0; i < instr_ptr->numTokens; i++)
		free(instr_ptr->tokens[i]);

	free(instr_ptr->tokens);

	instr_ptr->tokens = NULL;
	instr_ptr->numTokens = 0;
}

