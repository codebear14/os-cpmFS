/*
Author: Satyam Todkar
Email: szt0064@auburn.edu

The following program is a creation of my own and I have not reffered to help or been part to any discussion. Most of the concepts were clarified by Dr. Xiao Qin in his presentatio and the codes that he gave were very helpful in understanding the program structure and the task at hand. Besides this I also referred to some of the implementation in my other classes and also the Project 3.
*/

#include <stdio.h> 		
#include <stdint.h> 
#include <string.h>		// String libraray in C to support copy, comparison of two strings
#include "cpmfsys.h"

bool BitMap[NUM_BLOCKS];			// Global variable that keeps track of the free blocks
uint8_t block_zero_buffer[BLOCK_SIZE];	// Global variable that holds the block 0's copy in the program, like a main memory copy

DirStructType *mkDirStruct(int index,uint8_t *e){	//creates a directory entry
	
	DirStructType *dir;
	uint8_t *dirAddr;
	
	dir = malloc(sizeof(DirStructType));	//allocate the memory of size DirStructType
	int i=0;
	int count = 0;
	
	if(dir == NULL){		//memory alloccation failed
		printf("\nStatus: Failed to allocate space");
		exit(-1);
	}
	
	dirAddr = e + index*EXTENT_SIZE;	// this is how you get the address of a directory entry at an index and block referenced by e
	
	dir->status = dirAddr[0];	// store the status that is stored at the location. 0xe5 shows unused directory entry
	
	for(i=0; i<9; i++){		//(initialize) fill in blank spaces, as required in the program
		if(i!=8)
			dir->name[i] = ' ';
		else{
			dir->name[i] = '\0';
		}
	}
	
	for (i=1; i<9; i++){	//store the corresponding file name of length 8 and +1 for delimeter
		//printf("%c",dirAddr[i]);
		if(dirAddr[i] != ' ' && dirAddr[i]){
			dir->name[i-1] = dirAddr[i];
		}
		else{
			dir->name[i-1] = '\0';
			break;
		}
	}

	if(strlen(dir->name)==0){	
		dir->name[0] = '\0';
	}	
	
	//---
	
	for(i=0; i<4; i++){	//initialize using the blank spaces 3 for extension and 1 for delimeter 
		if(i!=3)
			dir->extension[i] = ' ';
		else{
			dir->extension[i] = '\0';
		}
	}
	
	for (i=9; i<12; i++){	//store the values at the corresponding location (9 to 11)
		if(dirAddr[i] != ' ' && dirAddr[i]){
			dir->extension[i-9] = dirAddr[i];
		}
		else{
			dir->extension[i-9] = '\0';
			break;
		}
	}

	if(strlen(dir->extension)==0){
		dir->extension[0] = '\0';
	}	
	
	dir->XL = dirAddr[12];	//store XL
	dir->BC = dirAddr[13];	//store number of bytes occupied in the last unfilled sector
	dir->XH = dirAddr[14];	//store XL
	dir->RC = dirAddr[15]; //store number of sectors occupied in the last sector unfilled block
	
	for (i=16; i<32; i++){	//store the blocks that are assigned to this file name
		dir->blocks[i-16] = dirAddr[i];
	}
	
	//--------------------------------------------------
/*	
	printf("\nDir Status: %2x", dir->status);
	printf("\nFile Name: %s",dir->name);
	printf("\nFile Extension: %s",dir->extension);
	printf("\nXL : %2x", dir->XL);
	printf("\nBC : %2x", dir->BC);
	printf("\nXH : %2x", dir->XH);
	printf("\nRC : %2x", dir->RC);
	
	printf("\nBlocks :");
	for (i=0; i<16; i++){
		printf(" %u",dir->blocks[i]);
	}
	
	//printf("\ni: %d",i);
	
	
*/
	return dir;
}

void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e){ //write the directory entry d, back to a location given by e and index
	
	uint8_t *dirAddr;
	int i;
	dirAddr = e + index*EXTENT_SIZE;	

	dirAddr[0] = d->status; //retrievet the staus of the directory entry
	
	for(i=1; i<9; i++){	//initialize with blank spaces
		dirAddr[i] = ' ';
	}
	
	for(i=1; i<9; i++){	//copy corresponding file name
		//printf("%c",d->name[i-1]);
		dirAddr[i] = d->name[i-1];
	}
	
	for(i=9; i<12; i++){ //initialize with blank spaces
		dirAddr[i] = ' ';
	}
	
	for(i=9; i<12; i++){ //copy corresponding file extension
		//printf("%c",d->extension[i-9]);
		dirAddr[i] = d->extension[i-9];
	}
	
	dirAddr[12] = d->XL; // copy XL
	dirAddr[13] = d->BC; // copy BC
	dirAddr[14] = d->XH; // copy XH
	dirAddr[15] = d->RC; // copy RC 
	
	for (i=16; i<32; i++){ //store the 16 blocks that are assigned to this file name
		//printf(" %u", d->blocks[i-16]);
		dirAddr[i] = d->blocks[i-16];
	}
		
	//--------------------------------------------------
	/*
	printf("\nDir Status: %2x", dirAddr[0]);
	printf("\nStored Name: ");
	for(i=1; i<9; i++){
		printf("%c",dirAddr[i]);
	}
	printf("\nStored Extension: ");
	for(i=9; i<12; i++){
		printf("%c",dirAddr[i]);
	}
	
	printf("\nStored XL : %2x", dirAddr[12]);
	printf("\nStored BC : %2x", dirAddr[13]);
	printf("\nStored XH : %2x", dirAddr[14]);
	printf("\nStored RC : %2x", dirAddr[15]);
	
	printf("\nStored Blocks :");
	for (i=16; i<32; i++){
		printf(" %u",dirAddr[i]);
	}
	*/
	blockWrite(e, 0);
	
}

void cpmDir(){	//funcrion to enlist the contents of a directory in the format file_name.file_extension file_length
	printf("DIRECTORY LISTING");
	uint8_t block_zero = 0;
	DirStructType *cpmDir;
	int extent_index;
	int block_index;
	int num_blocks;
	int file_length=0;

	blockRead(block_zero_buffer, block_zero);	//Read the block 0
	for(extent_index=0; extent_index<32; extent_index++){	//iterate over all the 32 directory entries
		cpmDir = mkDirStruct(extent_index, block_zero_buffer); //get a directory entry
		
		if(cpmDir->status != 0xe5){		//check if it is used or un used
			num_blocks = 0;				//if unused
			file_length = 0;
			for(block_index=0; block_index<16; block_index++){ //find the number of blocks assigned to the file
				if(cpmDir->blocks[block_index] !=0){	//this block is assigned/used by the file with file name cpmDir->name
					num_blocks++;	//increasing the counter
				}
			}
			
			file_length = (num_blocks-1)*1024 + (int)cpmDir->RC*128 + (int)cpmDir->BC; // Translating the formula (NB-1)*1024 + RC*128 + BC
			printf("\n%s.%s %d", cpmDir->name, cpmDir->extension, file_length); //printing the result
		}
	}
	
}


void makeFreeList(){ //function to create a free list that hold all the free blocks in the system
	uint8_t block_zero = 0;
	DirStructType *d;
	int extent_index;
	int block_index;
	int i;
	
	blockRead(block_zero_buffer, block_zero);	//Read the block 0
	
	BitMap[0] = false; //Block 0 is always assigned and can never be used
	for(i=1; i<NUM_BLOCKS; i++){ //Initialising the BitMap i.e. free list
		BitMap[i] = true;  
	}
	
	for(extent_index=0; extent_index<32; extent_index++){
		d = mkDirStruct(extent_index, block_zero_buffer);	//creating a directory entry
		
		if(d->status != 0xe5){		//check for used
			for(block_index=0; block_index<16; block_index++){
				if(d->blocks[block_index] != 0){ //for every block in directory entry mark it as assigned 
					BitMap[(int)d->blocks[block_index]] = false;
				}
			}
		}
	}
}

void printFreeList(){ //printing the free block list i.e. bitmap
	int i=0;
	printf("\nFREE BLOCK LIST: (* means in-use)\n");
	for(i=0; i<NUM_BLOCKS; i++){
		if (i % 16 == 0) { 				 // for dividing into 16X16 grid
		  fprintf(stdout,"%4x: ",i); 
		}
		if(BitMap[i] == true){	//If free mark as .
			printf(". ");
		}
		else{	//If in use mark as *	
			printf("* ");
		}
		if (i % 16 == 15) { 	//next row
		  printf("\n"); 
		}
	}
}

bool checkLegalName(char *name){ //function to check legality of file name
	int i;
	//printf("\n%s : %d",name, strlen(name));
  	for(i=0; i<strlen(name); i++){ //file name should not have punctuation, spaces or control characters
  		if(ispunct(name[i]) || isspace(name[i]) || iscntrl(name[i])){
  			return false;
  		}
  	}
  	return true;
}

bool checkLegalNameExt(char *name){ //function to check legality of file name
	int i;
	//printf("\n%s : %d",name, strlen(name));
  	for(i=0; i<strlen(name); i++){ //file extension should not have punctuation, spaces or control characters
  		if(ispunct(name[i]) || iscntrl(name[i])){
  			return false;
  		}
  	}
  	return true;
}

int findExtentWithName(char *name, uint8_t *block_zero_buffer){ //find an extent with a supplied file name

	int count=0;
	char fileName[9];
	char fileExtension[4];
	
	DirStructType *d;
	int extent_index;
	
    char *token = strtok(name, "."); //using string tokenizer to delemit "."
    while (token != NULL) 
    { 
  		count++;
        switch(count){
        	case 1:
        		if(strlen(token) <=8){
        			strcpy(fileName, token);	//storing file name
        		}
        		else{
        			printf("\nFailed: Filename should be max 8 characters");
        			return -1;
        		}
        		break;
        	
        	case 2:
        		if(strlen(token) <=3){
        			strcpy(fileExtension, token); //storing file extension
        		}
        		else{
        			printf("\nFailed: Extension should be max 3 characters");
        			return -1;
        		}
        		break;
        }
        token = strtok(NULL, ".");
    } 
    //printf("\nExtracted File Name: %s: %d", fileName, strlen(fileName));
    //printf("\nExtracted Extension : %s: %d", fileExtension, strlen(fileExtension));
  	
  	//printf("\n%d",count);
	if(checkLegalName(fileName)){ 
		//printf("\nFine");	
		for(extent_index=0; extent_index<32; extent_index++){
			d = mkDirStruct(extent_index, block_zero_buffer); //read a block

			if((strcmp(fileName, d->name) == 0 && strcmp(fileExtension, d->extension) == 0) || (strcmp(fileName, d->name) == 0 )){ //finding the extent for which file name & file extension is the same as supplieds
				if(d->status == 0xe5){
					//printf("\nFile is deleted");	//File is deleted, but extent still present i.e marked for deletion
					return -1;	
				}
				else{
					//printf("\nFile %s.%s found", d->name, d->extension);
					//printf("\n%s : %d", d->name, strlen(d->name));
					//printf("\n%s : %d", d->extension, strlen(d->extension));
					return extent_index; //File Found
				}
			}
		}
	}
	else{
		printf("\nStatus: Failed due to Illegal filename supplied"); //File check name legality failed
	}
  	
  	//printf("\nFile, just not found");
    
    return -1; //File not found, return no extent index
}


int cpmRename(char *oldName, char *newName){ //Rename an existing file
	
	char oName[20]; //Old File Name
	char nName[20];	//New File Name
	strcpy(oName,oldName); //Copying the files
	strcpy(nName,newName);
	//printf("\n%s",oName);
	//	printf("\n%s",nName);
	
	uint8_t block_zero = 0;
	int extentName;
	DirStructType *dir;
	int count=0;
	
	char fileName[9];
	char fileExtension[4];
	
	blockRead(block_zero_buffer, block_zero); //Read the block 0
	extentName = findExtentWithName(oName, block_zero_buffer); //get an extent i.e. returns extent nos. i.e index if a is file present 
	//printf("Location: %d", extentName);
	if(extentName >= 0){
		char *token = strtok(nName, "."); 
		while (token != NULL) 
		{ 
	  		count++;
		    switch(count){
		    	case 1:
		    		if(strlen(token) <=8){
		    			strcpy(fileName, token); //Extract filename of the new file
		    		}
		    		else{
		    			return -1;
		    		}
		    		break;
		    	
		    	case 2:
		    		if(strlen(token) <=3){
		    			strcpy(fileExtension, token); //Extract fileextension of the new file
		    		}
		    		else{
		    			return -1;
		    		}
		    		break;
		    }
		    token = strtok(NULL, ".");
		}
		//printf("\nExtracted File Name: %s: %d", fileName, strlen(fileName));
		//printf("\nExtracted Extension : %s: %d", fileExtension, strlen(fileExtension));
	
		dir = mkDirStruct(extentName, block_zero_buffer);  //get a directory entry for that index
		strcpy(dir->name,fileName);		//copy the file name
		strcpy(dir->extension, fileExtension); //copy the file extension
		
		writeDirStruct(dir, extentName, block_zero_buffer); //Update the Block 0
	}
	else{
		return -1;
	}
	return 0;
}

int cpmDelete(char * name){ //function to delete a file with given name
	
	char dName[20];		//file name to be deleted
	strcpy(dName,name); //copy the file name

	uint8_t block_zero = 0;
	int extentName;
	DirStructType *dir;
	int count=0;
	int block_index;
	char fileName[9];
	char fileExtension[4];
	
	uint8_t blockMarkDel;
	
	blockRead(block_zero_buffer, block_zero); //Read the block 0
	extentName = findExtentWithName(dName, block_zero_buffer); //get the extent nos. i.e the index of the directory entry that holds the file name
	//printf("Location: %d", extentName);
	if(extentName >= 0){		//found an extent
		char *token = strtok(dName, "."); 
		while (token != NULL) 
		{ 
	  		count++;
		    switch(count){
		    	case 1:
		    		if(strlen(token) <=8){
		    			strcpy(fileName, token); //extract filename
		    		}
		    		else{
		    			return -1;
		    		}
		    		break;
		    	
		    	case 2:
		    		if(strlen(token) <=3){
		    			strcpy(fileExtension, token);	//extract fileextension
		    		}
		    		else{
		    			return -1;
		    		}
		    		break;
		    }
		    token = strtok(NULL, ".");
		}	
	
		dir = mkDirStruct(extentName, block_zero_buffer); // get the dir entry for the specified file name i.e. retrivied from the index
		dir->status = 0xe5;	 //mark as unused
		strcpy(dir->name," "); //empty the file name
		strcpy(dir->extension, " "); //empty the file extension
		dir->XL = 0;	// Clear all the values XL, XH, BC, RC
		dir->BC = 0;	
		dir->XH = 0;	
		dir->RC = 0;	
		for(block_index=0; block_index<16; block_index++){
			
			blockMarkDel = dir->blocks[block_index]; // mark all the blocks in the directory entry referenced by extentName for deletion
			//printf("%u ",blockMarkDel);
			if(blockMarkDel > 0){
				BitMap[blockMarkDel] = true;	//Free the respective bitmap block locations
			}
		}
		writeDirStruct(dir, extentName, block_zero_buffer); //Update in Block 0
	}
	
	return 0;
}












