#ifndef _CACHE_MANAGER_H_
    #define _CACHE_MANAGER_H_

    #define MAX_PATH_LENGTH 256

    #define MAX_META_ENTRIES 256
    #define MAX_FILES_CACHED 32
    
    typedef struct meta_cache_entry {
        char path[MAX_PATH_LENGTH];
        int fileCacheIndex;
    } meta_cache_entry;    

    //Extern Methods
    char* getCachePath(char* path);

#endif
