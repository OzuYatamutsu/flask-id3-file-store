#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "cache_manager.h"

#define SERVER_ROOT "/.cache/ytl_cacheFile"
#define GET_META_COMMAND "curl -H \"Accept: application/json\" -H \"Content-Type: application/json\" http://localhost:9880/ls > ~/.cache/ytl_rawmeta.txt"

static meta_cache_entry meta_cache[MAX_META_ENTRIES];
static int metaCacheHead = 0;
static char file_cache[MAX_FILES_CACHED][MAX_FILENAME_LENGTH];
static int fileCacheHead = 0;
static int dirOffset = 0;

void uploadFile(char* path)
{
	printf("Uploading file %s\n", path);
}

static void addMetaDirectory(char* parentDir, char* fileName)
{
	strncpy(meta_cache[metaCacheHead].parentDir, parentDir, MAX_PATH_LENGTH);
	strncpy(meta_cache[metaCacheHead].fileName, fileName, MAX_FILENAME_LENGTH);	
	strncpy(meta_cache[metaCacheHead].sortedPath, parentDir, MAX_PATH_LENGTH);
	if(strcmp(parentDir, "/") != 0)
	{
		strncat(meta_cache[metaCacheHead].sortedPath, "/", MAX_PATH_LENGTH - strlen(meta_cache[metaCacheHead].sortedPath));
	}
	strncat(meta_cache[metaCacheHead].sortedPath, meta_cache[metaCacheHead].fileName, MAX_PATH_LENGTH - strlen(meta_cache[metaCacheHead].sortedPath));
	meta_cache[metaCacheHead].cacheName[0] = 0;
	meta_cache[metaCacheHead].fileSize = 0;
	meta_cache[metaCacheHead].owner = getuid();		
	meta_cache[metaCacheHead].isDir = 1;
	meta_cache[metaCacheHead].isShared = 1;
	
	//printf("Added directory parent:\"%s\" name:\"%s\" fullPath:\"%s\"\n", parentDir, fileName, meta_cache[metaCacheHead].sortedPath);
	
	metaCacheHead = (metaCacheHead + 1);	
	if(metaCacheHead == MAX_META_ENTRIES)
		metaCacheHead = MAX_META_ENTRIES-1;
}

int isDir(const char* sortedPath)
{
	int i;
	
	for(i = 0; i < MAX_META_ENTRIES && i < metaCacheHead; i++)
	{
		//printf("COMPARE DIR  %s %s\n",meta_cache[i].sortedPath,sortedPath);
		if(strcmp(meta_cache[i].sortedPath, sortedPath) == 0)
		{
			//printf("%s dir # %d\n", sortedPath, meta_cache[i].isDir);
			return meta_cache[i].isDir;
		}
	}
	return -1;
}

static void buildDirPath(char dirList[16][MAX_FILENAME_LENGTH], char* buf, int curDepth)
{
	int i;
	strcpy(buf, "/");
	for(i = 0; i < curDepth; i++)
	{
		if(i != 0)
		{
			strncat(buf, "/", MAX_PATH_LENGTH - strlen(buf));
		}
		strncat(buf, dirList[i], MAX_PATH_LENGTH - strlen(buf));
		
	}
	//printf("Built path %s\n", buf);
	
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
	char directoryNesting[16][MAX_FILENAME_LENGTH];
	char formattedLine[MAX_PATH_LENGTH];
	char newPath[MAX_PATH_LENGTH];
	int directoryLevel = 0;
	int care = 0;

	//Fetch raw json from server
	fp = popen(GET_META_COMMAND, "r");
	if(fp == NULL) 
	{
		printf("pipe error\n");
		return;
	}	
	pclose(fp);  
	fp = popen("cat ~/.cache/ytl_rawmeta.txt | jq .", "r");
	while(!feof(fp))
	{		
		if(getline(&lineBuffer, &nBytes, fp) == -1)
		{
			break;	
		}			
		if(strpbrk(lineBuffer, "\"") != NULL) 	
		{
			sscanf(lineBuffer, " \"%[^\n\"]", formattedLine);
			if(strcmp(formattedLine, "filename") == 0)
			{				
				sscanf(lineBuffer, " \"filename\": \"%[^\n\"]", formattedLine);
				strncpy(meta_cache[metaCacheHead].cacheName, formattedLine, MAX_PATH_LENGTH);
				//printf("GOT A FILENAME %s\n", formattedLine);
			}
			else if(strcmp(formattedLine, "filesize") == 0)
			{
				sscanf(lineBuffer, " \"filesize\": %[^\n,]", formattedLine);
				meta_cache[metaCacheHead].fileSize = atoi(formattedLine);
				//printf("GOT A FILESIZE %s\n", formattedLine);
			}
			else if(strcmp(formattedLine, "title") == 0)
			{
				sscanf(lineBuffer, " \"title\": \"%[^\n\"]", formattedLine);
				strncpy(meta_cache[metaCacheHead].fileName, formattedLine, MAX_PATH_LENGTH);
				//printf("GOT A TITLE %s\n", formattedLine);
			}
			else if(strcmp(formattedLine, "track") == 0)
			{
				sscanf(lineBuffer, " \"track\": %[^\n,]", formattedLine);
				//printf("GOT A TRACK %s\n", formattedLine);								
				strncpy(newPath, formattedLine, MAX_FILENAME_LENGTH);
				strncat(newPath, " - ", MAX_FILENAME_LENGTH - strlen(newPath));
				strncat(newPath, meta_cache[metaCacheHead].fileName, MAX_FILENAME_LENGTH - strlen(newPath));
				strncat(newPath, ".mp3", MAX_FILENAME_LENGTH - strlen(newPath));
				strncpy(meta_cache[metaCacheHead].fileName, newPath, MAX_FILENAME_LENGTH);
				buildDirPath(directoryNesting, newPath, directoryLevel);
				strncpy(meta_cache[metaCacheHead].parentDir, newPath, MAX_PATH_LENGTH);
				strncpy(meta_cache[metaCacheHead].sortedPath, newPath, MAX_PATH_LENGTH);
				strncat(meta_cache[metaCacheHead].sortedPath, "/", MAX_PATH_LENGTH - strlen(meta_cache[metaCacheHead].sortedPath));
				strncat(meta_cache[metaCacheHead].sortedPath, meta_cache[metaCacheHead].fileName, MAX_PATH_LENGTH - strlen(meta_cache[metaCacheHead].sortedPath));				
				meta_cache[metaCacheHead].owner = getuid();		
				meta_cache[metaCacheHead].isDir = 0;
				meta_cache[metaCacheHead].isShared = 1;	
	
				metaCacheHead++;
				if(metaCacheHead > MAX_META_ENTRIES)
					metaCacheHead--;
			}
			else
			{
				care = 0;
				strncpy(directoryNesting[directoryLevel], formattedLine, MAX_FILENAME_LENGTH);				
				//printf("Adding %s to directory level %d\n", formattedLine, directoryLevel);
				directoryLevel++;
				buildDirPath(directoryNesting, newPath, directoryLevel-1);
				addMetaDirectory(newPath, formattedLine);
			}
			strncpy(newPath, "/albums/", MAX_PATH_LENGTH);
			strncat(newPath, formattedLine, MAX_PATH_LENGTH - strlen(newPath));
			//printf("Adding directory %s\n", newPath);			
			
		}
		else
		{		
			if(strpbrk(lineBuffer, "]") != NULL)
			{
				care = 1;
				directoryLevel--;
				//printf("ENDED DIRECTORY ]\n");
			}
			if(strpbrk(lineBuffer, "}") != NULL)
			{
				if(care == 1)
				{
					care = 0;
					directoryLevel--;
					//printf("ENDED DIRECTORY }\n");
				}
			}
		}	
	}
	pclose(fp);  	
	
	//free our line buffer
	if(lineBuffer != NULL)
		free(lineBuffer);
}

char* getDirName(const char* rootDir)
{
	for(;dirOffset < MAX_META_ENTRIES && dirOffset >= 0 && dirOffset < metaCacheHead; dirOffset++)
	{
		//printf("Comparing %s to %s at offset %d\n", meta_cache[dirOffset].parentDir, rootDir, dirOffset);
		if(strcmp(meta_cache[dirOffset].parentDir, rootDir) == 0)
		{
			dirOffset++;
			return meta_cache[dirOffset-1].fileName;
		}
	}
	dirOffset = 0;
	return NULL;
}

static void getCacheIndex(char* fileName, char* buf)
{
	int i;
	
	for(i = 0; i < MAX_FILES_CACHED; i++)
	{
		if(strcmp(file_cache[i], fileName) == 0)
		{			
			sprintf(buf, "%d", i);
			return;
		}
	}
	sprintf(buf, "%d", -1);
	return;
}

//Read file into disk from server
static void getFileInCache(char* fileName)
{
	char command[256];
	char index[16];
	getCacheIndex(fileName, index);
	if(strcmp(index, "-1") == 0)
	{
		//printf("Getting file in cache %s\n", fileName);
		strncpy(command, "curl http://localhost:9880/get_file/", 256);
		strncat(command, fileName, 256 - strlen(command));
		strncat(command, " -o ~/.cache/ytl_cacheFile", 256 - strlen(command));
		sprintf(index, "%d", fileCacheHead);
		strncat(command, index, 256 - strlen(command));	
		//printf("command %s\n", command);
	
		FILE *fp;
		fp = popen(command, "r");
		if(fp == NULL) 
		{
			printf("pipe error\n");
			return;
		}	
		pclose(fp);  

		strncpy(file_cache[fileCacheHead], fileName, MAX_FILENAME_LENGTH);
		fileCacheHead = (fileCacheHead + 1)	% MAX_FILES_CACHED;
		//printf("Finished loading %s to %s\n",fileName, index);
	}
}

//Give path to cached file
void getCachePath(char* cachePathBuf, const char* sortedPath)
{
	int i;
	char strIndex[16];

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
		//printf("Meta Cache Problem, couldn't find %s\n", sortedPath);
		cachePathBuf[0] = 0;
		return;
	}	
	if(meta_cache[i].isDir == 1)
	{
		return;
	}
	//get the file from server since we probably going to use in near future
	getFileInCache(meta_cache[i].cacheName);
	//all files are located in same directory, just append filename to root path
	strncpy(cachePathBuf, getenv("HOME"), MAX_PATH_LENGTH);
	strncat(cachePathBuf, SERVER_ROOT, MAX_PATH_LENGTH - strlen(cachePathBuf));
	getCacheIndex(meta_cache[i].cacheName, strIndex);
	strncat(cachePathBuf, strIndex, MAX_PATH_LENGTH - strlen(cachePathBuf));
	//printf("For %s got cache path %s\n", sortedPath, cachePathBuf);
}



void initCache(void)
{
	memset(meta_cache, 0, sizeof(meta_cache_entry) * MAX_META_ENTRIES);
	memset(file_cache, 0, MAX_FILES_CACHED * MAX_FILENAME_LENGTH);
	getMetadataTree();	
}


