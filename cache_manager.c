#include <stdlib.h>
#include <stdio.h>
#include "cache_manager.h"

static meta_cache_entry meta_cache[MAX_META_ENTRIES];

//Read file into disk from server
static void getFileInCache(char* path)
{
	//avoid compiler warnings
    path = path + 1;
}

//Give path to cached file
char* getCachePath(char* path)
{
	//avoid compiler warnings
    meta_cache[0].fileCacheIndex = 0;
    path = path + 1;
    getFileInCache(NULL);
    return "/home/rhensey/TheSearch.mp3";
}
