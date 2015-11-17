#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
#include "cache_manager.h"

#define MAX_COMMAND_LENGTH 256
#define MAX_INFO_LENGTH 256

typedef struct songInfo {
	char title[MAX_INFO_LENGTH];
	char track[MAX_INFO_LENGTH];
	char artist[MAX_INFO_LENGTH];
	char album[MAX_INFO_LENGTH];
	char year[MAX_INFO_LENGTH];
	char comment[MAX_INFO_LENGTH];
	char genre[MAX_INFO_LENGTH];
} songInfo;

char realPath[MAX_PATH_LENGTH];
char newRealPath[MAX_PATH_LENGTH];
songInfo currentSongInfo;



static void ytl_realPath(char pathBuffer[], const char *path)
{  
	strcpy(pathBuffer, getCachePath(NULL));
	printf("REALPATH %s\n", realPath);
}

static void extractInfo(FILE* fp, char* lineBuffer, char* destination)
{
	int needSpace = 0;
	while(fscanf(fp, "%s", lineBuffer) == 1)
	{			
		if(strpbrk(lineBuffer, ":") == NULL) 
		{
			if(needSpace)
				strncat(destination, " ", MAX_INFO_LENGTH - strlen(destination));
			strncat(destination, lineBuffer, MAX_INFO_LENGTH - strlen(destination));
		}
		else
		   return;//linebuffer ready to check without another scan				 
		needSpace = 1;
	}
}

static void getFileInfo(const char* realPath)
{
	//make it check to make sure mp3
	printf("***getting file info for %s***\n", realPath);
	FILE *fp;
	char lineBuffer[1024];
	char command[MAX_COMMAND_LENGTH];
	snprintf(command, MAX_COMMAND_LENGTH, "mp3info \"%s\"\n", realPath);
	
	fp = popen(command, "r");
	if(fp == NULL) 
	{
		printf("pipe error\n");
		//couldn't run pipe
		//think best way to handle this
	}
	
	while(!feof(fp))
	{		
		if(fscanf(fp, "%s", lineBuffer) != 1)
			break;		
		//0 is equal
		if(strcmp(lineBuffer, "Title:") == 0)
		{
			currentSongInfo.title[0] = '\0';
			extractInfo(fp, lineBuffer, currentSongInfo.title);			
		}
		if(strcmp(lineBuffer, "Track:") == 0)
		{
			currentSongInfo.track[0] = '\0';
			extractInfo(fp, lineBuffer, currentSongInfo.track);
		}
		if(strcmp(lineBuffer, "Artist:") == 0)
		{
			currentSongInfo.artist[0] = '\0';
			extractInfo(fp, lineBuffer, currentSongInfo.artist);
		}
		if(strcmp(lineBuffer, "Album:") == 0)
		{
			currentSongInfo.album[0] = '\0';
			extractInfo(fp, lineBuffer, currentSongInfo.album);
		}
		if(strcmp(lineBuffer, "Year:") == 0)
		{
			currentSongInfo.year[0] = '\0';
			extractInfo(fp, lineBuffer, currentSongInfo.year);
		}
		if(strcmp(lineBuffer, "Comment:") == 0)
		{
			currentSongInfo.comment[0] = '\0';
			extractInfo(fp, lineBuffer, currentSongInfo.comment);
		}
		if(strcmp(lineBuffer, "Genre:") == 0)
		{
			currentSongInfo.genre[0] = '\0';
			extractInfo(fp, lineBuffer, currentSongInfo.genre);
		}
	}
	printf("ALLINFO: %s %s %s %s %s %s %s \n", currentSongInfo.title, currentSongInfo.track, currentSongInfo.artist,
		currentSongInfo.album, currentSongInfo.year, currentSongInfo.comment, currentSongInfo.genre);
	pclose(fp);  
}

static int ytl_getattr(const char *path, struct stat *stbuf)
{
	printf("\n\nGET ATTR %s\n\n", path);
	memset(stbuf, 0, sizeof(struct stat));
	if(strcmp(path, "/") == 0) 
	{
		stbuf->st_mode = S_IFDIR | 0755;	
	} 
	else if (strcmp(path, "/Song_Name.mp3") == 0) 
	{
		stbuf->st_mode = S_IFREG | 0777;
		stbuf->st_nlink = 1;
		stbuf->st_size = 11923920;   
	} 
	else
	{
		return -ENOENT;
	}
	return 0;
}

static int ytl_access(const char *path, int mask)
{
	//Eventually decide if can't access return -1
	return 0;
}

static int ytl_readlink(const char *path, char *buf, size_t size)
{
	//I don't think we need linking
	return 0;
}


static int ytl_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			   off_t offset, struct fuse_file_info *fi)
{
	//Go fetch metadata from meta_cache, for now just manually fill
	//filler(buf, ".", NULL, 0);
	//filler(buf, "..", NULL, 0);
	filler(buf, "Song_Name.mp3", NULL, 0);
	return 0;
}

static int ytl_mknod(const char *path, mode_t mode, dev_t rdev)
{
	/*
	int res;

	ytl_realPath(realPath, path);
	res = mknod(realPath, mode, rdev);
	if (res == -1)
		return -errno;
	*/
	//want to make new file/dir
	return 0;
}

static int ytl_chmod(const char *path, mode_t mode)
{
	int res;
	ytl_realPath(realPath, path);

	res = chmod(realPath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int ytl_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;

	ytl_realPath(realPath, path);
	res = lchown(realPath, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int ytl_truncate(const char *path, off_t size)
{
	int res;

	ytl_realPath(realPath, path);
	res = truncate(realPath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int ytl_open(const char *path, struct fuse_file_info *fi)
{
	int res;

	//get path from cache
	ytl_realPath(realPath, path);
	
	//Testing function
	getFileInfo(realPath);	
	
	res = open(realPath, fi->flags);
	if (res == -1)
	{
		printf("\n\nERORRRRRRRRR: %d\n\n", -errno);
		//return -errno;
		return 0;
	} 
	else
	{
		printf("\n\nOPENED\n\n");
	}

	close(res);
	return 0;
}

static int ytl_read(const char *path, char *buf, size_t size, off_t offset,
			struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;
	printf("\n\nREADING! %s\n\n", path);

	ytl_realPath(realPath, path);

	fd = open(realPath, O_RDONLY);
	if (fd == -1)
	{
		printf("READ OPEN ERR %d\n", -errno);
		return -errno;
	}

	res = pread(fd, buf, size, offset);
	if (res == -1) 
	{
		printf("READ ERR %d\n", -errno);
		res = -errno;
	}
	printf("READ OK %s %lx %lx\n", buf, (long)size, (long)offset);

	close(fd);
	return res;
}

static int ytl_write(const char *path, const char *buf, size_t size,
			 off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

	ytl_realPath(realPath, path);

	fd = open(realPath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int ytl_statfs(const char *path, struct statvfs *stbuf)
{
	int res;

	ytl_realPath(realPath, path);
	res = statvfs(realPath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}


static struct fuse_operations ytl_oper = {
	.getattr	= ytl_getattr,
	.access		= ytl_access,
	.readlink	= ytl_readlink,
	.readdir	= ytl_readdir,
	.mknod		= ytl_mknod,	 
	.chmod		= ytl_chmod,
	.chown		= ytl_chown,
	.truncate	= ytl_truncate,
	.open		= ytl_open,
	.read		= ytl_read,
	.write		= ytl_write,
	.statfs		= ytl_statfs,
};

int main(int argc, char *argv[])
{	
	printf("STARTIN FUSE\n");
	umask(0);
	return fuse_main(argc, argv, &ytl_oper, NULL);
}
