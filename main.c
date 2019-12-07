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

metaData BootSector;

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

void memcpyX(void *target, const void *source, size_t size) {
    
    int* result = (int *)target;
    memcpy(target,source,size);
    //printing 4 bytes of buffer in HEX
    printf("%04X \n",*result);
    
}

void getDirEntries(void *data){
    //    uint32_t leader = 0;
    //    uint8_t attributes = 0xFF;
    //    const uint8_t DirStructSz = 32;
    struct DirEntry *result = data;
    
    // end of list for root is 0x00000000
    // end of list for other directories may be 0xFFFFFFFF
    //    memcpyX(&leader, source, 1);
    //
    //
    //    memcpyX(&attributes, source + 11, 1);
    //
    //    source += DirStructSz - 11;
    
    
    
    
    
}

void **getFatData(uint32_t at) {
    
    uint32_t locations[MaxItems];
    void **data = malloc(MaxItems * sizeof(void *));
    int location_index = 0;
    int data_index = 0;
    uint32_t currentLocation = at;
    
    locations[location_index] = currentLocation;
    int nextValue = (currentLocation * 4) + FATStart;
    
    
    while ( (currentLocation != 0x0FFFFFF8 && currentLocation != 0xFFFFFFFF) &&
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


FILE * _FATOpen() {
#ifdef __APPLE__
    FILE *fp;
    fp = fopen ("/Users/samuelparmer/Desktop/C++ Projects/C Projects/FAT32/FAT32/fat32.img","r+");
    return fp;
    
#endif
    
    //implemet with argv[0] param on linprog
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
    BootSector.BPB_TotSec32 = getUInt16(32);
    
    printf("Bytes per sector: %d\n",BootSector.BtyesPerSec);
    printf("Sectors per cluster: %d\n",BootSector.secsPerCluster);
    printf("Reserved sector count: %d\n",BootSector.RSVDSecCount);
    printf("Number of FATS: %d\n",BootSector.FATS);
    printf("Size of FATS: %d\n",BootSector.FATSize);
    printf("Root entries count: %d\n",BootSector.BPB_RootEntCnt); //use to calc how big root dir is, each entry is 32 bytes
    printf("TotSec32: %d\n",BootSector.BPB_TotSec32);
    
    
    
}

void size(char * FILENAME){
    //print size of file in the current working dir in bytes, error if not found.
    
}

typedef int (*dir_op_t)(struct DirEntry *entry, void *param);

int directory_do(dir_op_t item_op, void **data, void *param){
    
    int i,j,exiting = 0;
    
    for(i = 0; i < MaxItems; i++){
        if(data[i] == NULL) { break; }
        struct DirEntry *entries = data[i];
        //        printf("DirEntry size: %lu", sizeof(DirEntry));
        for(j = 0; j < 16; j++) {
            if (entries[j].attributes == 0x0f) { continue; }
            if (entries[j].fileName[0] == 0) { break; }
            
            // operated...
            if ((exiting = item_op(&entries[j], param))) { break; }
            
        }
        if (exiting) { break; }
    }
    return exiting;
}

int dir_print (struct DirEntry *entry, void *param)  { printf("%.11s\n", entry->fileName); return 0; }
int dir_change(struct DirEntry *entry, void *param)  {
    // directories matching name with blanks padded...
    printf("%.11s\n", entry->fileName);
    uint32_t newdir = 0;
    if ( (entry->attributes == 0x10) &&
        (strncmp(param, (const char *)entry->fileName, 11) == 0) ) {
        // traverse to directory
        // TODO: Setup the directory properly...
        newdir =  cluster(getFirstCluster(entry->Hi, entry->Lo));
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

void ls(char * DIRNAME){
    //list name of all directories in the current dir including . and ..
    void**data = getFatData(currentDirectory);
    if (DIRNAME == NULL) {
        directory_do(dir_print,  data, NULL);
    } else {
        directory_do(dir_print_sub, data, DIRNAME);
    }
    
}


void cd(char * DIRNAME){
    void**data = getFatData(currentDirectory);
    directory_do(dir_change, data, DIRNAME);
}

int create(char * FILENAME){
    //create a file of size 0 in cwd
    
    // 1) guard - filename not in directory
    
    
    // 2)
    
    return 0;
    
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



int main(int argc, const char * argv[]) {
    
    __fat_fp = _FATOpen();
    
    ls("GREEN      ");
    
    
    FATexit();
    return 0;
}
