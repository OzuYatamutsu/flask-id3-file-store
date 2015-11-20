#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "cache_manager.h"

#define SERVER_ROOT "/mnt/yourtuneslib/"
#define GET_META_COMMAND "curl -H \"Accept: application/json\" -H \"Content-Type: application/json\" http://localhost:9880/ls > ~/.cache/ytl_rawmeta.txt"

static meta_cache_entry meta_cache[MAX_META_ENTRIES];
static int metaCacheHead = 0;
static char file_cache[MAX_FILES_CACHED][MAX_FILENAME_LENGTH];
static int fileCacheHead = 0;
static int dirOffset = 0;

static void addMetaDirectory(char* parentDir, char* sortedPath, char* fileName)
{
	strncpy(meta_cache[metaCacheHead].sortedPath, sortedPath, MAX_PATH_LENGTH);
	strncpy(meta_cache[metaCacheHead].parentDir, parentDir, MAX_PATH_LENGTH);
	strncpy(meta_cache[metaCacheHead].fileName, fileName, MAX_FILENAME_LENGTH);	
	meta_cache[metaCacheHead].fileSize = 0;
	meta_cache[metaCacheHead].owner = getuid();		
	meta_cache[metaCacheHead].isDir = 1;
	meta_cache[metaCacheHead].isShared = 1;
	metaCacheHead = (metaCacheHead + 1);	
	if(metaCacheHead == MAX_META_ENTRIES)
		metaCacheHead = MAX_META_ENTRIES-1;
	printf("Added directory to metax index %d with parent %s filename %s\n", metaCacheHead-1, parentDir, fileName);
}

int isDir(const char* sortedPath)
{
	int i;
	
	for(i = 0; i < MAX_META_ENTRIES && i < metaCacheHead; i++)
	{
		printf("COMPARE DIR*(*( %s %s\n",meta_cache[i].sortedPath,sortedPath);
		if(strcmp(meta_cache[i].sortedPath, sortedPath) == 0)
		{
			printf("%s dir # %d\n", sortedPath, meta_cache[i].isDir);
			return meta_cache[i].isDir;
		}
	}
	return -1;
}

//Query for all metadata
void getMetadataTree(void)
{
	printf("Building Metadata Tree\n");
	metaCacheHead = 0;
	
	FILE *fp;
	size_t nBytes = 256;
	char* lineBuffer;
	lineBuffer = malloc(nBytes + 1);
	char formattedLine[MAX_PATH_LENGTH];
	char newPath[MAX_PATH_LENGTH];

	//Fetch raw json from server
	fp = popen(GET_META_COMMAND, "r");
	if(fp == NULL) 
	{
		printf("pipe error\n");
		return;
	}	
	pclose(fp);  
	//parse album names using jq
	fp = popen("cat ~/.cache/ytl_rawmeta.txt | jq \".albums|keys\"", "r");
	while(!feof(fp))
	{		
		if(getline(&lineBuffer, &nBytes, fp) == -1)
		{
			break;	
		}			
		if(strpbrk(lineBuffer, "\"") != NULL) 	
		{
			sscanf(lineBuffer, " \"%[^\n\"]", formattedLine);
			strncpy(newPath, "/albums/", MAX_PATH_LENGTH);
			strncat(newPath, formattedLine, MAX_PATH_LENGTH - strlen(newPath));
			printf("Adding directory %s\n", newPath);			
			addMetaDirectory("/albums", newPath, formattedLine);
		}
	}
	pclose(fp);  	
	//parse decades using jq
	fp = popen("cat ~/.cache/ytl_rawmeta.txt | jq \".decades|keys\"", "r");
	while(!feof(fp))
	{		
		if(getline(&lineBuffer, &nBytes, fp) == -1)
		{
			break;	
		}			
		if(strpbrk(lineBuffer, "\"") != NULL) 	
		{
			sscanf(lineBuffer, " \"%[^\n\"]", formattedLine);
			strncpy(newPath, "/decades/", MAX_PATH_LENGTH);
			strncat(newPath, formattedLine, MAX_PATH_LENGTH - strlen(newPath));
			printf("Adding directory %s\n", newPath);			
			addMetaDirectory("/decades", newPath, formattedLine);
		}
	}
	pclose(fp);  	
	//free our line buffer
	if(lineBuffer != NULL)
		free(lineBuffer);
}

char* getDirName(char* rootDir)
{
	for(;dirOffset < MAX_META_ENTRIES && dirOffset >= 0 && dirOffset < metaCacheHead; dirOffset++)
	{
		printf("Comparing %s to %s at offset %d\n", meta_cache[dirOffset].parentDir, rootDir, dirOffset);
		if(strcmp(meta_cache[dirOffset].parentDir, rootDir) == 0)
		{
			dirOffset++;
			return meta_cache[dirOffset-1].fileName;
		}
	}
	dirOffset = 0;
	return NULL;
}

//Read file into disk from server
static void getFileInCache(const char* sortedpath)
{
	//GET to /get_file/filename
	sortedpath = sortedpath + 1;
}

//Give path to cached file
void getCachePath(char* cachePathBuf, const char* sortedPath)
{
	int i;

	for(i = 0; i < MAX_META_ENTRIES; i++)
	{
		if(strcmp(sortedPath, meta_cache[i].sortedPath) == 0)
		{
			break;
		}
	}
	//If it isn't found is bug
	if(i == MAX_META_ENTRIES)
	{
		printf("Meta Cache Problem\n");
		cachePathBuf[0] = 0;
		return;
	}	
	//get the file from server since we probably going to use in near future
	getFileInCache(sortedPath);
	//all files are located in same directory, just append filename to root path
	strncpy(cachePathBuf, SERVER_ROOT, MAX_PATH_LENGTH);
	strncat(cachePathBuf, meta_cache[i].fileName, MAX_PATH_LENGTH - strlen(cachePathBuf));
}



void initCache(void)
{
	memset(meta_cache, 0, sizeof(meta_cache_entry) * MAX_META_ENTRIES);
	memset(file_cache, 0, MAX_FILES_CACHED * MAX_FILENAME_LENGTH);
	//Temp
	/*
	strncpy(meta_cache[0].sortedPath, "/All/Song_Name.mp3", MAX_PATH_LENGTH);
	strncpy(meta_cache[0].parentDir, "/All", MAX_PATH_LENGTH);
	strncpy(meta_cache[0].fileName, "TheSearch.mp3", MAX_FILENAME_LENGTH);	
	meta_cache[0].fileSize = 11923920;
	meta_cache[0].owner = getuid();		
	meta_cache[0].isDir = 0;
	meta_cache[0].isShared = 0;
	metaCacheHead = 1;
	*/
	strncpy(file_cache[0], "TheSearch.mp3", MAX_FILENAME_LENGTH);
	fileCacheHead = 1;
	getMetadataTree();	
}


