#ifndef _CACHE_MANAGER_H_
	#define _CACHE_MANAGER_H_

	#define MAX_PATH_LENGTH 256
	#define MAX_FILENAME_LENGTH 256

	#define MAX_META_ENTRIES 4096
	#define MAX_FILES_CACHED 32
	
	
	typedef struct meta_cache_entry {
		char sortedPath[MAX_PATH_LENGTH];
		char parentDir[MAX_PATH_LENGTH];
		char fileName[MAX_FILENAME_LENGTH];
		char cacheName[MAX_FILENAME_LENGTH];
		uid_t owner;		
		int isDir;
		int isShared;		
		off_t fileSize;
	} meta_cache_entry;		
	
	//Extern Methods
	void getCachePath(char* cachePathBuf, const char* sortedpath);
	char* getDirName(const char* rootDir);
	int isDir(const char* sortedPath);
	void initCache(void);
	void getMetadataTree(void);
	void uploadFile(char* path);
	void deleteFile(const char* path);

#endif
