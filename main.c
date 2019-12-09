//
//  main.c
//  FAT32
//
//  Created by Samuel Parmer on 11/26/19.
//  Copyright © 2019 Samuel Parmer. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

// decls
void ls(char * DIRNAME);

#define BYTES_PER_SEC 512 //offset 11, 2 bytes
#define RSVD_SEC_CNT 32 // offset 14, 2 bytes
#define NUM_FATS 2 //offset 16, 1 byte
#define FATSZ_32 1009 //offset 36, 4 bytes <- # OF SECTORS IN A FAT
#define SEC_PER_CLUS 1 //offset 13, 1 byte

#define FATStart 16384
#define MaxItems 32

// BPB_RootClus: offset 44, 4 bytes

//Region offsets//
// offset to FAT: 16384
// offset to data: 1049600

// Total FAT region size: 1033216


//--EQUATIONS--//

// RSVD_SEC_CNT * BYTES_PER_SEC = RESERVED_REGION_SIZE_BYTES
// FATSZ_32 * NUM_FATS * BYTES_PER_SEC = FAT_REGION_SIZE_BYTES
// FirstSectorOfACluster = FirstDataSector + ((N-2)*SEC_PER_CLUS

// MARK: GLOBALS

FILE * __fat_fp;
uint32_t currentDirectory = 2;  // FAT slot

typedef struct metaData{
    int BtyesPerSec;
    int RSVDSecCount;
    int FATS;
    int FATSize;
    int secsPerCluster;
    int BPB_RootEntCnt;
    int jmpBoot;
    int OEMName;
    int BPB_TotSec32;
    int BPBRootClus;
    
}metaData;

struct DirEntry{
    uint8_t fileName[11]; // bytes 0-10
    uint8_t attributes; //byte 11
    uint8_t reserved; //byte 12
    uint8_t timeStampTenth; // file creation in tenths of second, byte 13
    uint16_t timeStampHourMinSec; // file creation in hours, mins, secs bytes 14 & 15
    uint16_t creationDate; //bytes 16 & 17
    uint16_t accessDate; // bytes 18 & 19
    uint16_t Hi; // high order 2 bytes of address of first cluster; bytes 20 & 21
    uint16_t modifiedTime; // bytes 22 & 23
    uint16_t modifiedDate; //bytes 24 & 25
    uint16_t Lo; // bytes 26 & 27
    uint32_t fileSize; //0 for directories; bytes 28-31
    
    
}__attribute__((packed)) DirEntry;


struct LList {
    struct LList *next;
    void *data;
};
typedef struct LList *node; //

struct LLHead {
    node start;
    node end;
};
typedef struct LLHead *head;



node newNode(void *data){
    node temp; // declare a node
    temp = (node)malloc(sizeof(struct LList)); // allocate memory using malloc()
    temp->next = NULL;// make next point to NULL
    temp->data = data;
    return temp;
}

node addNode(head head, void *data){
    node last_end = head->end;
    head->end = newNode(data);
    
    head->start   = (head->start == NULL) ? head->end : head->start;
    if (last_end != NULL) { last_end->next = head->end; }
    return head->end;
    
}

head newHead(){
    return (head) calloc(sizeof(struct LLHead), 1);
}

node pushNode(head head, void *data){
    node temp = newNode(data);
    
    temp->next = head->start;
    head->start=temp;
    
    if(head->end == NULL){
        head->end = temp;
    }
    
    return head->start;
}

//mode 1 = read, 2 = write, 3 = read and write
enum FileMode{
    mode_read = 1,
    mode_write = 2,
    mode_read_write = 3
} ;

struct FileEntry {
    uint32_t cluster;
    enum FileMode mode;
};
typedef struct FileEntry *fatfile;





head FATHead;
metaData BootSector;
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
//char *FileName;

FILE * _FATOpen(char *FileName) {
    FILE *fp;
#ifdef __APPLE__
    
    fp = fopen ("/Users/samuelparmer/Desktop/C++ Projects/C Projects/FAT32/FAT32/fat32.img","r+");
    return fp;
    
#endif

    // fp = fopen ("/home/majors/parmer/OS_P3/fat32.img","r+");
    fp = fopen(FileName,"r+");
    if(!fp)
    {
        printf("Error: File name %s does not exist.\n", FileName);
        exit(0);
    }
    else
    {
        printf("File transfer: Successful\n");
        return fp;
    }
    
    
    //implemet with argv[0] param on linprog
    // return 0;
}

int cluster(int n){
    //hard coded data region value, fix later
    return 1049600 +((n-2) * SEC_PER_CLUS) * BYTES_PER_SEC;
}

uint16_t getFirstCluster(uint16_t hi, uint16_t lo) {
    return (hi<<8) | lo;
}


void *get(u_int64_t offset, int length){
    
    void *data = (void *)malloc(length);
    fseek(__fat_fp, offset, SEEK_SET);
    fread(data, length, 1, __fat_fp);
    
    return data;
}

uint32_t getUInt32(u_int64_t offset) {
    uint32_t result;
    
    fseek(__fat_fp, offset, SEEK_SET);
    fread(&result, sizeof(uint32_t), 1, __fat_fp);
    return result;
    
}

uint16_t getUInt16(u_int64_t offset) {
    uint16_t result;
    
    fseek(__fat_fp, offset, SEEK_SET);
    fread(&result, sizeof(uint16_t), 1, __fat_fp);
    return result;
    
}

uint8_t getUInt8(u_int64_t offset) {
    uint8_t result;
    
    fseek(__fat_fp, offset, SEEK_SET);
    fread(&result, sizeof(uint8_t), 1, __fat_fp);
    return result;
    
}

size_t putUint32(uint32_t offset, uint32_t value){
    size_t result;
    
    fseek(__fat_fp, offset, SEEK_SET);
    result = fwrite(&value, sizeof(uint32_t), 1, __fat_fp);
    return result;
}


size_t putUint8(uint8_t offset, uint8_t value){
    size_t result;
    
    fseek(__fat_fp, offset, SEEK_SET);
    result = fwrite(&value, sizeof(uint8_t), 1, __fat_fp);
    return result;
}


void memcpyX(void *target, const void *source, size_t size) {
    
    int* result = (int *)target;
    memcpy(target,source,size);
    //printing 4 bytes of buffer in HEX
    printf("%04X \n",*result);
    
}


void **getFatData(uint32_t at) {
    
    uint32_t locations[MaxItems];
    void **data = malloc(MaxItems * sizeof(void *));
    int location_index = 0;
    int data_index = 0;
    uint32_t currentLocation = at;
    
    locations[location_index] = currentLocation;
    int nextValue = (currentLocation * 4) + FATStart;
    
    
    while ( (currentLocation != 0xFFFFFF0F  && currentLocation != 0xFFFFFFFF && currentLocation != 0x0FFFFFF8) &&
           data_index < MaxItems - 1 ) {
        void *dataPart = get(cluster(currentLocation), 512);
        data[data_index] = dataPart;
        
        // next data location
        currentLocation = getUInt32(nextValue);
        locations[location_index] = currentLocation;
        nextValue = (currentLocation * 4) + FATStart;
        
        // advance
        data_index++;
        location_index++;
    }
    
    
    data[data_index] = NULL;
    return data;
    
    
}


head getClusterList(uint32_t at) {
    
    //uint32_t locations[MaxItems];
    //void **data = malloc(MaxItems * sizeof(void *));
    //int location_index = 0;
    int data_index = 0;
    uint32_t currentLocation = at;
    head clusterHead = newHead();
    
    
    
    //locations[location_index] = currentLocation;
    int nextValue = (currentLocation * 4) + FATStart;
    
    
    while ( (currentLocation != 0xFFFFFF0F&& currentLocation != 0xFFFFFFFF && currentLocation != 0x0FFFFFF8) ) {
        //void *dataPart = get(cluster(currentLocation), 512);
        //data[data_index] = dataPart;
        uint32_t *value = malloc(sizeof(uint32_t ));
        *value = currentLocation;
        pushNode(clusterHead, value);
        
        
        // next data location
        currentLocation = getUInt32(nextValue);
        //locations[location_index] = currentLocation;
        nextValue = (currentLocation * 4) + FATStart;
        
        // advance
        data_index++;
        //location_index++;
        }
    
    
    //data[data_index] = NULL;
    //node result = clusterHead->start;
    //free(clusterHead);
    return clusterHead;
    
    
}



typedef int (*dir_op_t)(struct DirEntry *entry, void *param);

int match_FAT_spec(struct DirEntry *entry,  void *name, uint8_t type){
    // match exact spec;  0xFF for any type
    if ( ((type == 0xFF) || (entry->attributes == type)) &&
        (strncmp(name, (const char *)entry->fileName, 11) == 0) ) {
        return 1;}
    
    return 0;
    
}

int directory_do(dir_op_t item_op, uint32_t dir, void *param){
    
    int i,j,exiting = 0;
    int dotdirs = -1;
    void**data = getFatData(currentDirectory);
    
    for(i = 0; i < MaxItems; i++){
        if(data[i] == NULL) { break; }
        struct DirEntry *entries = data[i];
        //        printf("DirEntry size: %lu", sizeof(DirEntry));
        for(j = 0; j < 16; j++) {
            if (entries[j].attributes == 0x0f) { continue; }
            if (entries[j].fileName[0] ==  0 ) { exiting = -1; break; }
            if (entries[j].fileName[0] == '.') {
                if (dotdirs < 1) {
                    dotdirs += 1;
                } else {
                    exiting = -1;
                    break;
                }
            }
            
            // operated...
            if ((exiting = item_op(&entries[j], param))) { break; }
            
        }
        if (exiting) { break; }
    }
    return exiting;
}


//struct DirEntry *getLastDirEntry(dir_op_t item_op){
//
//    int i,j,exiting = 0;
//    int dotdirs = -1;
//    void**data = getFatData(currentDirectory);
//
//    for(i = 0; i < MaxItems; i++){
//        if(data[i] == NULL) { break; }
//        struct DirEntry *entries = data[i];
//        //        printf("DirEntry size: %lu", sizeof(DirEntry));
//        for(j = 0; j < 16; j++) {
//            if (entries[j].attributes == 0x0f) { continue; }
//            if (entries[j].fileName[0] ==  0 ) { exiting = -1; break; }
//            if (entries[j].fileName[0] == '.') {
//                if (dotdirs < 1) {
//                    dotdirs += 1;
//                } else {
//                    exiting = j-1;
//                    break;
//                }
//            }
//
//            // operated...
//            //if ((exiting = item_op(&entries[j], param))) { break; }
//
//        }
//        if (exiting) { break; }
//    }
//
//    if(exiting > 0){
//        return entries[j];
//    }
//
//}

int dir_print (struct DirEntry *entry, void *param)  {
    printf("%.11s\n", entry->fileName); return 0;
}

void *dir_entry (struct DirEntry *entry, void *param)  {
    
    if ((strncmp(param, (const char *)entry->fileName, 11) == 0) ) {
        return entry;}
    
    return 0;
}


int isOpen(char * fileName){
    //check if file is in the open list
    node p = FATHead->start;
    struct DirEntry *entry = directory_do(dir_entry,currentDirectory, fileName);
    
    while (p) {
        if ((uint32_t)p->data == getFirstCluster(entry->Hi, entry->Lo)) {
            printf("file was open");
            return 1; //found cluster in list of open files
        }
    }
    return 0; //not found
}


int dir_change(struct DirEntry *entry, void *param)  {
    // directories matching name with blanks padded...
    printf("%.11s\n", entry->fileName);
    uint32_t newdir = 0;
    
    //    if ( (entry->attributes == 0x10) &&
    //         (strncmp(param, (const char *)entry->fileName, 11) == 0) ) {
    if (match_FAT_spec(entry, param, 0x10)) {
        // traverse to directory
        // TODO: Setup the directory properly...
        newdir =  getFirstCluster(entry->Hi, entry->Lo);
        printf("*** New Cluster Number: %d\n", newdir);
        //        newdir = 3;
        
        
        // ...
        currentDirectory = newdir;
        
    }
    return newdir;   // done & done
}

int dir_print_sub(struct DirEntry *entry, void *param)  {
    // directories matching name with blanks padded...
    
    uint32_t newdir = 0;
    uint32_t saved = currentDirectory;
    if ((newdir = dir_change(entry, param))) {
        ls(NULL);
        currentDirectory = saved;
    }
    return newdir;
}

int dir_get_size(struct DirEntry *entry, void *param)  {
    // directories matching name with blanks padded...
    
    if (match_FAT_spec(entry, param, 0xFF)) {
        return entry->fileSize ? entry->fileSize : -1;
    }
    
    return 0;
    
}

int dir_get_cluster(struct DirEntry *entry, void *param)  {
    // directories matching name with blanks padded...
    
    if (match_FAT_spec(entry, param, 0xFF)) {
        return getFirstCluster(entry->Hi, entry->Lo);
    }
    
    return 0;
    
}



// MARK: Required

void FATexit(){
    
    //free all resources
    
    fclose(__fat_fp);
    exit(0);
}

void info(){
    //parse boot sector, print info for each entry
    // come back and add all other fat fields!
    BootSector.BtyesPerSec = getUInt16(11);
    BootSector.secsPerCluster = getUInt8(13);
    BootSector.FATS = getUInt8(16);
    BootSector.RSVDSecCount = getUInt16(14);
    BootSector.FATSize = getUInt32(36);
    BootSector.BPB_RootEntCnt = getUInt16(17);
    BootSector.BPB_TotSec32 = getUInt32(32);
    BootSector.BPBRootClus = getUInt32(44);
    
    printf("Bytes per sector: %d\n",BootSector.BtyesPerSec);
    printf("Sectors per cluster: %d\n",BootSector.secsPerCluster);
    printf("Reserved sector count: %d\n",BootSector.RSVDSecCount);
    printf("Number of FATS: %d\n",BootSector.FATS);
    printf("Total sectors: %d\n",BootSector.BPB_TotSec32);
    printf("Size of FATS: %d\n",BootSector.FATSize);
    printf("Root cluster: %d\n",BootSector.BPBRootClus); //use to calc how big root dir is, each entry is 32 bytes
    
    
    
    
}




void size(char * FILENAME){
    //print size of file in the current working dir in bytes, error if not found.
    
    int sz = directory_do(dir_get_size,currentDirectory, FILENAME);
    //TODO: error if not found
    if (sz == 0) {
        // file was not found, handle error here:
        printf("ERROR: %.11s - file not found.\n", FILENAME);
    } else {
        printf("%.11s %d\n", FILENAME, (sz == -1) ? 0 : sz);
    }
}

void ls(char * DIRNAME){
    //list name of all directories in the current dir including . and ..
    
    if (DIRNAME == NULL) {
        directory_do(dir_print, currentDirectory, NULL);
    } else {
        directory_do(dir_print_sub, currentDirectory, DIRNAME);
    }
    
}


void cd(char * DIRNAME){
    directory_do(dir_change, currentDirectory, DIRNAME);
}

int create(char * FILENAME){
    //create a file of size 0 in cwd
    
    // 1) guard - filename not in directory
    
    
    // 2)
    
    return 0;
    
}

void  FATmkdir(char * DIRNAME){
    
}

fatfile FATOpen(char * FILENAME, enum FileMode mode){
    //open file for reading or writing. Need to maintain a table of files that are open
    
    // 1) guard - filename not in directory
    uint32_t cluster = directory_do(dir_get_cluster,currentDirectory, FILENAME);
    //TODO: error if not found
    if (cluster == 0) {
        // file was found, handle error here:
        printf("ERROR: %.11s - file not found.\n", FILENAME);
        return NULL;
    }
    
    fatfile file = malloc(sizeof(struct FileEntry));
    file->cluster = cluster;
    file->mode = mode;
    addNode(FATHead, file);
    return file;
}

void FATClose(char * FILENAME){
    //close file for reading or writing. Need to maintain a table of files that are open
    
    // is it in the current directory?
    uint32_t cluster = directory_do(dir_get_cluster,currentDirectory, FILENAME);
    //TODO: error if not found
    if (cluster == 0) {
        // file was found, handle error here:
        printf("ERROR: %.11s - file not found.\n", FILENAME);
        return;
    }
    
    
    // traverse
    node p  = FATHead->start;
    node last = (node)FATHead;
    int8_t found = 0;
    while(p != NULL){
        
        // find...; remove link.
        if (((fatfile)(p->data))->cluster == cluster) {
            found = 1;
            last->next = p->next;
            free(p->data);
            free(p);
            
            if(FATHead->start == NULL) {FATHead->end = NULL;}
            
            break;
        }
        
        last = p; p = p->next;
        
    }
    
    
    
}

void FATRead(char * FILENAME, int OFFSET, int SIZE){  //THIS IS THE read() FUNCTION

    struct DirEntry *entry = directory_do(dir_entry,currentDirectory, FILENAME);


    if(entry != 0 && entry->attributes != 0x0f && isOpen(getFirstCluster(entry->Hi, entry->Lo)) && OFFSET < entry->fileSize){
        //file is open


    }

}

void FATWrite(char * FILENAME, int OFFSET){
    
}

void rm(char * FILENAME){
    struct DirEntry *entry = directory_do(dir_entry,currentDirectory, FILENAME);
    head list;
    node p;
    
    list = getClusterList(getFirstCluster(entry->Hi, entry->Lo));
    
    p = list->start;
    
    while(p){
        putUint32(*(uint32_t *)p->data,0x00000000); //unlink list; put 0's in FAT entries
        p = p->next;
        }
    
    //entry->fileName
    
    
    printf("about to remove file %s", entry->fileName);
}

void FATrmdir(char * DIRNAME){
    
}

char *padName(char *destination,char *src){
    
    sprintf(destination, "%s           ", src);
    return destination;
}



int main(int argc, const char * argv[]) {
    
    // init globals
    FATHead = newHead();
    __fat_fp = _FATOpen("");

    char padded[23];

    // test operations


    fatfile ff = FATOpen(padName(padded, "HELLO"), mode_read);
    ////isOpen(padName(padded, "HELLO"));

    FATClose(padName(padded, "HELLO"));
    //FATRead(padName(padded, "HELLO"));

    info();
    size("GLADDIO     ");
    size(padName(padded, "LONGFILE"));
    cd("RED          ");
    ls(NULL);
    ls("RED001       ");


    FATexit();
    return 0;
//    -----parser-----
//    if(!argv[1] || strlen(argv[1]) < 1)
//    {
//        printf("Error:Couldn't import file\n");
//        printf("Using default parameters.\n");
//        __fat_fp = _FATOpen("fat32.img");
//    }
//    else
//    {
//        __fat_fp = _FATOpen(argv[1]);
//        printf("File was imported\n");
//    }
//    FATHead = newHead();
//    //__fat_fp = _FATOpen();
//    char* token = NULL;
//    char* temp = NULL;
//    instruction instr;
//    instr.tokens = NULL;
//    instr.numTokens = 0;
//    while (1) {
//        printf("Enter Command:");
//        do {
//            scanf("%ms",&token);
//            temp = (char*)malloc((strlen(token) + 1) * sizeof(char));
//            int i;
//            int start = 0;
//            addToken(&instr,token);
//            start = i + 1;
//
//            if (start < strlen(token)) {
//                memcpy(temp, token + start, strlen(token) - start);
//                temp[i-start] = '\0';
//                addToken(&instr, temp);
//            }
//            free(token);
//            free(temp);
//
//            token = NULL;
//            temp = NULL;
//
//
//        } while ('\n' != getchar());    //until end of line is reached
//        addNull(&instr);
//        //  printTokens(&instr);
//        run(&instr);
//
//        clearInstruction(&instr);
//    }
//
//    return 0;
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
    char padded[23];
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
                info(padName(padded,instr_ptr->tokens[i+1]));
                printf("Error:too many arguments\n");
            }
        }
        else if (strcmp(instr_ptr->tokens[i], "size")==0)
        {
            if(instr_ptr->tokens[i+1]!=NULL)
            {
                size(padName(padded,instr_ptr->tokens[i+1]));
            }//
            else
                printf("Error:not enough arguments\n");
            
        }
        else if (strcmp(instr_ptr->tokens[i], "ls")==0)
        {
            
                ls(padName(padded,instr_ptr->tokens[i+1]));
        }
        else if (strcmp(instr_ptr->tokens[i], "cd")==0)
        {
            if(instr_ptr->tokens[i+1]!=NULL)
            {
                cd(padName(padded,instr_ptr->tokens[i+1]));
            }
            else
                printf("Error:not enough arguments\n");
        }
        else if (strcmp(instr_ptr->tokens[i], "creat")==0)
        {
            if(instr_ptr->tokens[i+1]!=NULL)
            {
                //creat(padded(padded,instr_ptr->tokens[i+1])_;
            }
            else
                printf("Error:not enough arguments\n");
        }
        else if (strcmp(instr_ptr->tokens[i], "mkdir")==0)
        {
            
            if(instr_ptr->tokens[i+1]!=NULL)
            {
                //mkdir(padded(padded,instr_ptr->tokens[i+1]));
            }
            else
                printf("Error:not enough arguments\n");
        }
        else if (strcmp(instr_ptr->tokens[i], "open")==0)
        {
            if(instr_ptr->tokens[i+1]!=NULL&&instr_ptr->tokens[i+2]!=NULL)
            {
                //FATOpen((instr_ptr->tokens[i+1],instr_ptr->tokens[i+2]));
            }
            else
                printf("Error:not enough arguments\n ");
        }
        else if (strcmp(instr_ptr->tokens[i], "close")==0)
        {
            if(instr_ptr->tokens[i+1]!=NULL)
            {
                //FATClose(padName(padded,instr_ptr->tokens[i+1]));
            }
            else
                printf("Error:not enough arguments\n");
        }
        else if (strcmp(instr_ptr->tokens[i], "read")==0)
        {
            if(instr_ptr->tokens[i+1]!=NULL&&instr_ptr->tokens[i+2]!=NULL&&instr_ptr->tokens[i+3]!=NULL)
            {
                //FATRead(instr_ptr->tokens[i+1],instr_ptr->tokens[i+2],instr_ptr->tokens[i+3]);
            }
            else
                printf("Error:not enough arguments\n");
        }
        else if (strcmp(instr_ptr->tokens[i], "write")==0)
        {
            if(instr_ptr->tokens[i+1]!=NULL&&instr_ptr->tokens[i+2]!=NULL&&instr_ptr->tokens[i+3]!=NULL)
            {
                //FATWrite(instr_ptr->tokens[i+1],instr_ptr->tokens[i+2],instr_ptr->tokens[i+3]);
            }
            else
                printf("Error:not enough arguments\n");
        }
        else if (strcmp(instr_ptr->tokens[i], "rm")==0)
        {
            if(instr_ptr->tokens[i+1]!=NULL)
            {
                //rm(padName(padded,instr_ptr->tokens[i+1]));
            }
            else
                printf("Error:not enough arguments\n");
        }
        else if (strcmp(instr_ptr->tokens[i], "rmdir")==0)
        {
            if(instr_ptr->tokens[i+1]!=NULL)
            {
                // FATrmdir(padName(padded,instr_ptr->tokens[i+1]));
            }
            else
                printf("Error:not enough arguments\n");
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
