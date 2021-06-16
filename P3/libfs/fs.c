#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "disk.h"
#include "fs.h"

#define FAT_EOC 65535		// last block of data for a file
#define FS_NAME "ECS150FS"	// name of file system

struct superblock {
	uint8_t sig[8];			// file signature
	uint16_t blockTotal;	// total number of blocks
	uint16_t rootIndex;		// root directory block index
	uint16_t dataIndex;		// start of data blocks index
	uint16_t dataTotal;		// total number of data blocks
	uint8_t fatTotal;		// total number of FAT blocks
	uint8_t padding[4079];	// padding
};
typedef struct superblock superblock;

struct fileinfo {
	uint8_t name[16];		// file name
	uint32_t size;			// file size
	uint16_t index;			// index of beginning data block
	uint8_t padding[10];	// padding
};
typedef struct fileinfo fileinfo;

struct rootblock {
	uint8_t count;						// count of files
	fileinfo files[FS_FILE_MAX_COUNT];	// array of files
};


typedef struct rootblock rootblock;

struct openInfo{
	bool isOccupied;
	size_t fileOffset; //the offset for open file(initially 0)
	fileinfo individualFile;	//obtained from the root block
};
typedef struct openInfo openInfo;

struct openFileTable{
	uint8_t count;//Number of files open
	openInfo files[FS_OPEN_MAX_COUNT];//the array of open files
};
typedef struct openFileTable openFileTable;


openFileTable OFT; //the open file table
static int fd;		// file system
superblock sb;		// super block
uint16_t *fatblock;	// FAT block array
rootblock rb;		// root directory

/**
 * find_file - Check for file in root directory
 *
 * filename: Name of the file being located
 *
 * Return: -1 if no match was found, otherwise return the root directory index
 * of the file.
 */
int find_file(const char *filename)
{
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
		if (strncmp((char*)rb.files[i].name, filename, FS_FILENAME_LEN) == 0)
			return i;
	}

	return -1;
}

/**
 * count_files - Count number of files in root directory
 */
void count_files(void)
{
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
		if (rb.files[i].index != 0)
			rb.count++;
	}
}

//find the first free data block by looking in FAT 
int freeDataBlock(void){
	
	for (int i = 0; i < sb.dataTotal; i++) {
		if (fatblock[i] == 0)
			return i;
	}
	return -1; //no free data block available
}

//initializing the open file table
void OpenFileTableInitializer(void){
	for(int i=0; i < FS_OPEN_MAX_COUNT;i++){
		OFT.files[i].isOccupied=false;
	}
}

int find_openFD(void){
	for(int i=0;i<FS_OPEN_MAX_COUNT;i++){
		if(!OFT.files[i].isOccupied){
			return i;
		}
	}
	return -1; //all FDs are occupied in open file table
}

/**
 * file_open - Check is file is currently open
 *
 * filename: Name of the file being checked
 *
 * Return: -1 if no match was found, otherwise return the open file table
 * index.
 */
int file_open(const char *filename)
{
	for (int i = 0; i < FS_OPEN_MAX_COUNT; i++) {
		if (OFT.files[i].isOccupied == true) {
			if (strncmp((char*)OFT.files[i].individualFile.name,
					filename,
					FS_FILENAME_LEN) == 0)
			return i;
		}
	}

	return -1;
}

/**
 * free_block - Zero out all data in specified data block
 *
 * block: Index of the data block to be cleared
 */
void free_block(uint16_t block)
{
	uint8_t emptyBuf[BLOCK_SIZE];
	block_write(block, emptyBuf);
}

/**
 * print_fat - Print the FAT table to console
 */
void print_fat(void)
{
	for (int i = 1; i < sb.dataTotal; i++) {
		if (fatblock[i] > 0)
			printf("[%d]: %d\n", i, fatblock[i]);
	}
}

int fs_mount(const char *diskname)
{
	
	OpenFileTableInitializer(); //initialize the open file table
	// open file system
	fd = block_disk_open(diskname);

	// return -1 if file open failed
	if (fd == -1)
		return -1;

	// read super block
	block_read(0, &sb);

	// return -1 if FS signature is incorrect
	if (strncmp((char*)sb.sig, FS_NAME, 8) != 0)
		return -1;

	// allocate enough memory for the FAT block array and read Fat info
	fatblock = (uint16_t*)malloc(2048 * sb.fatTotal * sizeof(uint16_t));
	for (int i = 0; i < sb.fatTotal; i++) {
		block_read(1 + i, &fatblock[i * 2048]);
	}

	/*
	for (int j = 0; j < sb.fatTotal * 2048; j++) {
		if (fatblock[j] != 0)
			printf("fatblock[%d] = %d\n", j, fatblock[j]);
	}
	*/

	// read root directory info
	block_read(sb.rootIndex, &rb.files);
	count_files();

	return 0;
}

int fs_umount(void)
{
	// write out data to file system
	block_write(sb.rootIndex, &rb.files);

	for (int i = 0; i < sb.fatTotal; i++) {
		block_write(1 + i, &fatblock[i * 2048]);
	}

	block_write(0, &sb);

	// cleanup
	free(fatblock);
	fatblock = NULL;

	if (OFT.count > 0)
		return -1;

	if (block_disk_close() == 0)
		return 0;
	else
		return -1;
}

int fs_info(void)
{
	int fatFree = sb.dataTotal;
	int fileFree = FS_FILE_MAX_COUNT;

	// Calculate free data blocks
	for (int i = 0; i < sb.dataTotal; i++) {
		if (fatblock[i] != 0)
			fatFree--;
	}

	// Calculate available number of files
	for (int j = 0; j < 128; j++) {
		if (rb.files[j].index != 0)
			fileFree--;
	}

	printf("FS Info:\n");
	printf("total_blk_count=%d\n", sb.blockTotal);
	printf("fat_blk_count=%d\n", sb.fatTotal);
	printf("rdir_blk=%d\n", sb.rootIndex);
	printf("data_blk=%d\n", sb.dataIndex);
	printf("data_blk_count=%d\n", sb.dataTotal);
	printf("fat_free_ratio=%d/%d\n", fatFree, sb.dataTotal);
	printf("rdir_free_ratio=%d/%d\n", fileFree, FS_FILE_MAX_COUNT);
	return 0;
}

int fs_create(const char *filename)
{
	int nLen = strlen(filename);
	//if(filename[15]!='\000'){
	//	printf("filename[15]:%c\n",filename[15]);
	//}
	//printf("%d\n",nLen);
	if (filename == NULL){
		//return -1;
		//printf("we are in filename==NULL\n");
		return -1;
	}
	if (rb.count == FS_FILE_MAX_COUNT){
		//return -1;
		//printf("we are in max count\n");
		return -1;
	}
	if (nLen > FS_FILENAME_LEN){// || filename[15] != '\000'){
		//printf("we are in nLen\n");
		return -1;
	}
	if (strncmp((char*)sb.sig, FS_NAME, 8) != 0){
		return -1;
	}
	if (find_file(filename) > -1){
		return -1;
	}
	strncpy((char*)rb.files[rb.count].name, filename, FS_FILENAME_LEN);
	//printf("%s\n",rb.files[rb.count].name);
	rb.files[rb.count].index =  FAT_EOC;
	rb.count++;

	return 0;
}

int fs_delete(const char *filename)
{
	//printf("where at?\n");
	uint16_t dataIndex = 0;
	uint16_t nextDataIndex = 0;
	int nLen = strlen(filename);
	int fileIndex;
	int openIndex;

	//printf("**FAT before delete**\n");
	//print_fat();

	if (strncmp((char*)sb.sig, FS_NAME, 8) != 0)
		return -1;

	if (filename == NULL)
		return -1;

	if (rb.count == 0)
		return -1;

	if (nLen > FS_FILENAME_LEN)// || filename[15] != '\000')
		return -1;

	fileIndex = find_file(filename);
	if (fileIndex == -1)
		return -1;

	openIndex = file_open(filename);
	if (openIndex > -1)
		return -1;

	nextDataIndex = rb.files[fileIndex].index;
	while (nextDataIndex != FAT_EOC) {
		dataIndex = nextDataIndex;
		nextDataIndex = fatblock[dataIndex];
		free_block(dataIndex);
		fatblock[dataIndex] = 0;
	}

	rb.files[fileIndex].index = 0;
	rb.files[fileIndex].size = 0;
	for (int i = 0; i < FS_FILENAME_LEN; i++)
		rb.files[fileIndex].name[i] = '\000';
	rb.count--;

	//printf("**FAT after delete**\n");
	//print_fat();

	return 0;
}

int fs_ls(void)
{
	printf("FS Ls:\n");
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
		if (rb.files[i].index != 0) {
			printf("file: %s", rb.files[i].name);
			printf(", size: %d", rb.files[i].size);
			printf(", data_blk: %d\n", rb.files[i].index);
		}
	}
	return 0;
}

int fs_open(const char *filename)
{

	/* TODO: Phase 3 */
	
		
	int nLen = strlen(filename);
	int rootBlockIndex;

	if (strncmp((char*)sb.sig, FS_NAME, 8) != 0)
		return -1;

	if (filename == NULL)
		return -1;

	if (rb.count == 0)
		return -1;

	if (nLen > FS_FILENAME_LEN)// || filename[15] != '\000')
		return -1;

	rootBlockIndex = find_file(filename);
	if (rootBlockIndex == -1)
		return -1;
	
	if(OFT.count==32){
		return -1;
	}
	
	//int rootBlockIndex=find_file(filename);
	int openFD=find_openFD();
	//printf("%d\n",openFD);
	OFT.files[openFD].individualFile=rb.files[rootBlockIndex];
	OFT.files[openFD].isOccupied=true;
	//printf("%s\n",OFT.files[OFT.entry].individualFile.name);
	OFT.count++;
	return openFD;//returns the file discriptor 
}

int fs_close(int fd)
{
	/* TODO: Phase 3 */

	if (strncmp((char*)sb.sig, FS_NAME, 8) != 0)
		return -1;

	if(fd<0 || fd>31){
		return -1;
	}
	if(!OFT.files[fd].isOccupied){
		return -1;
	}
	OFT.files[fd].isOccupied=false;
	OFT.files[fd].fileOffset=0;
	OFT.count--;
	return 0;
}

int fs_stat(int fd)
{
	
	if (strncmp((char*)sb.sig, FS_NAME, 8) != 0)
		return -1;

	if(fd<0 || fd>31){
		return -1;
	}
	if(!OFT.files[fd].isOccupied){
		return -1;
	}
	
	int rootBlockIndex=find_file((char*)OFT.files[fd].individualFile.name);
	return rb.files[rootBlockIndex].size;
	return 0;
}

int fs_lseek(int fd, size_t offset)
{
	
	if (strncmp((char*)sb.sig, FS_NAME, 8) != 0)
		return -1;

	if(fd<0 || fd>31){
		return -1;
	}
	if(!OFT.files[fd].isOccupied){
		return -1;
	}

	int rootBlockIndex=find_file((char*)OFT.files[fd].individualFile.name);
	if(offset>rb.files[rootBlockIndex].size){
		return -1;	
	}
	OFT.files[fd].fileOffset=offset;
	return 0;
}
int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
	
	//To do:
		//next:incorporating FAT
		//after that: write a file over multiple data blocks
		//now: when writing a huge data, find the number of data blocks required and write it(also changing the fat in the process)
		
	if (strncmp((char*)sb.sig, FS_NAME, 8) != 0)
		return -1;

	if(fd<0 || fd>31){
		return -1;
	}
	if(!OFT.files[fd].isOccupied){
		return -1;
	}
	if(buf==NULL){
		return -1;
	}
	
	
	
	if(count==0){
		return count;
	}
	
	int rootBlockIndex=find_file((char*)OFT.files[fd].individualFile.name);
	
		//printf("%zu\n",count);
		//first we take the case where we write something into the first block, and then try and extend it to the next one

	int dataBlockFromTheRoot=OFT.files[fd].fileOffset/4096;//this gives us the number of we have to iterate through in FAT in order to get the data block we need to write in
								//Ex: 0 means write in the first block shown by root block, 1 means find the next block from the first block from FAT
		//printf("offset: %zu",OFT.files[fd].fileOffset);
	int offsetWithinDataBlock=OFT.files[fd].fileOffset - (dataBlockFromTheRoot*4096);//this is the offset within the data block we need to write	
		//printf("offset in new dataBlock: %d\n",offsetWithinDataBlock);
		
		
		//first we find the data block
	int dataBlockIndex=rb.files[rootBlockIndex].index; //stores the index from the root block
	if(dataBlockIndex==FAT_EOC){
			//printf("root block index is FAT_EOC\n");						
		int firstFreeDataBlockIndex=freeDataBlock();//this obtains index from the fat block
						
		rb.files[rootBlockIndex].index=firstFreeDataBlockIndex; //this has to equal the first free data block
					
		fatblock[firstFreeDataBlockIndex]=FAT_EOC;
	
		dataBlockIndex=rb.files[rootBlockIndex].index;
			
	}else{
		
		for(int i=0;i<dataBlockFromTheRoot;i++){
			if(fatblock[dataBlockIndex]==FAT_EOC){
				int newDataBlockIndex=freeDataBlock();
				fatblock[dataBlockIndex]=newDataBlockIndex;
				fatblock[newDataBlockIndex]=FAT_EOC;
				//printf("we are in for\n");	
			}
			dataBlockIndex=fatblock[dataBlockIndex];
		}
	}
		
	uint8_t bounce[4096];// bounce contains all the past information of the data block that we are writing to
	uint8_t finalBuf[4096];//This is what we write to the data block
	int bytesStillToWrite=count;
	int writtenBytes=0;//this keeps track of the number of bytes written
	while(bytesStillToWrite>0){
			
		int trueDataBlockIndex=dataBlockIndex+sb.dataIndex;
		int space = 4096-offsetWithinDataBlock;
		block_read(trueDataBlockIndex,bounce);

	//	memcpy(finalBuf,bounce,offsetWithinDataBlock);	
		
		memcpy(finalBuf,bounce,offsetWithinDataBlock); 
		//next I need to copy the newly written part
		if(bytesStillToWrite<=space){
			memcpy(finalBuf+offsetWithinDataBlock,buf+writtenBytes,bytesStillToWrite);
			OFT.files[fd].fileOffset+=count;//new file offset
			offsetWithinDataBlock+=bytesStillToWrite;
			bytesStillToWrite=0;
		//next I need to copy the non-modified part after the new
			memcpy(finalBuf+offsetWithinDataBlock,bounce+offsetWithinDataBlock,4096-offsetWithinDataBlock);
			block_write(trueDataBlockIndex,finalBuf);

		}else{
			memcpy(finalBuf+offsetWithinDataBlock,buf+writtenBytes,space);
			offsetWithinDataBlock=0;
			writtenBytes+=space;
			bytesStillToWrite-=space;
			if(fatblock[dataBlockIndex]==FAT_EOC){
					
				int newDataBlockIndex=freeDataBlock();
				fatblock[dataBlockIndex]=newDataBlockIndex;
				fatblock[newDataBlockIndex]=FAT_EOC;
				dataBlockIndex=newDataBlockIndex;
			}else{
				dataBlockIndex=fatblock[dataBlockIndex];
			}
			block_write(trueDataBlockIndex,finalBuf);
				
		}
	}

		//have to change root block size 
	if(OFT.files[fd].fileOffset>rb.files[rootBlockIndex].size){
		rb.files[rootBlockIndex].size=OFT.files[fd].fileOffset;
	}

		
	return count;
	
		
			

 
	
}

int fs_read(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
	

	
	if (strncmp((char*)sb.sig, FS_NAME, 8) != 0)
		return -1;

	if(fd<0 || fd>31){
		return -1;
	}
	if(!OFT.files[fd].isOccupied){
		return -1;
	}
	if(buf==NULL){
		return -1;
	}

	int rootBlockIndex=find_file((char*)OFT.files[fd].individualFile.name);
	
	int dataBlockFromTheRoot=OFT.files[fd].fileOffset/4096;//this gives us the number of we have to iterate through in FAT in order to get the data block we need to write in
								//Ex: 0 means write in the first block shown by root block, 1 means find the next block from the first block from FAT
	//printf("offset: %zu",OFT.files[fd].fileOffset);
	int offsetWithinDataBlock=OFT.files[fd].fileOffset - (dataBlockFromTheRoot*4096);//this is the offset within the data block we need to write	
		
	int dataBlockIndex=rb.files[rootBlockIndex].index; //stores the index from the root block
	
	if((rb.files[rootBlockIndex].size-OFT.files[fd].fileOffset)<count){
		count=rb.files[rootBlockIndex].size-OFT.files[fd].fileOffset;
	}
	for(int i=0;i<dataBlockFromTheRoot;i++){
		
		dataBlockIndex=fatblock[dataBlockIndex];
	}


	uint8_t bounce[4096];// bounce contains all the past information of the data block that we are writing to
	//uint8_t finalBuf[count];//This is what we write to the data block
	int bytesStillToRead=count;
	int readBytes=0;//this keeps track of the number of bytes written
	while(bytesStillToRead>0){

		int trueDataBlockIndex=dataBlockIndex+sb.dataIndex;
		int space = 4096-offsetWithinDataBlock;
		block_read(trueDataBlockIndex,bounce);
		
		

		if(bytesStillToRead<=space){
			memcpy(buf+readBytes,bounce+offsetWithinDataBlock,bytesStillToRead);
			OFT.files[fd].fileOffset+=count;//new file offset
				//offsetWithinDataBlock+=bytesStillToWrite;
			bytesStillToRead=0;

		}else{
		
			memcpy(buf+readBytes,bounce+offsetWithinDataBlock,space);
			offsetWithinDataBlock=0;
			readBytes+=space;
			bytesStillToRead-=space;
			dataBlockIndex=fatblock[dataBlockIndex];
				//block_write(trueDataBlockIndex,finalBuf);
		}
	}
	return count;
}
