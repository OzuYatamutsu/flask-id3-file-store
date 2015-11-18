#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cache_manager.h"

#define SERVER_ROOT "/home/rhensey/"

static meta_cache_entry meta_cache[MAX_META_ENTRIES];
static int metaCacheHead = 0;
static char fileCacheIndex[MAX_FILES_CACHED][MAX_FILENAME_LENGTH];
static int fileCacheHead = 0;

//Read file into disk from server
static void getFileInCache(const char* sortedpath)
{
	//avoid compiler warnings
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
	//Fetch metadata
	if(i == MAX_META_ENTRIES)
	{
		//store at metaCacheHead
		//increment head % size
	}	
	//get the file from server since we probably going to use in near future
	getFileInCache(sortedPath);
	strncpy(cachePathBuf, SERVER_ROOT, MAX_PATH_LENGTH);
	strncat(cachePathBuf, meta_cache[i].fileName, MAX_PATH_LENGTH - strlen(cachePathBuf));
}

void initCache(void)
{
	memset(meta_cache, 0, sizeof(meta_cache_entry) * MAX_META_ENTRIES);
	//Temp
	strncpy(meta_cache[0].sortedPath, "/Song_Name.mp3", MAX_PATH_LENGTH);
	strncpy(meta_cache[0].fileName, "TheSearch.mp3", MAX_FILENAME_LENGTH);	
	meta_cache[0].fileSize = 11923920;
	metaCacheHead = 1;
	strncpy(fileCacheIndex[0], "TheSearch.mp3", MAX_FILENAME_LENGTH);
	fileCacheHead = 1;
}


