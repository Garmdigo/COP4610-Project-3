//
//  main.c
//  FAT32
//
//  Created by Samuel Parmer on 11/26/19.
//  Copyright © 2019 Samuel Parmer. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
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
