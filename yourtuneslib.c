#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
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
char tmpPath[MAX_PATH_LENGTH];
char tmpAttrPath[MAX_PATH_LENGTH];
int writeTmp;

static void ytl_realPath(char pathBuffer[], const char *path)
{  
	getCachePath(pathBuffer, path);
	//printf("REALPATH %s\n", realPath);
}

static int ytl_getattr(const char *path, struct stat *stbuf)
{
	if(writeTmp == 1)
	{
		writeTmp = 0;
		//upload file
		getMetadataTree();
	}
	printf("getAttr path %s | tmpPath %s\n", path, tmpAttrPath);
	memset(stbuf, 0, sizeof(struct stat));
	int type = isDir(path);	
	if(strcmp(path, "/") == 0 || strcmp(path, "/albums") == 0 || strcmp(path, "/decades") == 0 || type == 1) 
	{
		stbuf->st_mode = S_IFDIR | 0755;	
	} 
	else if (type == 0) 
	{
		stbuf->st_mode = S_IFREG | 0777;
		stbuf->st_nlink = 1;
		stbuf->st_size = 11923920;   
	} 
	else if(strcmp(path, tmpAttrPath) == 0)
	{
		stbuf->st_mode = S_IFREG | 0777;
		stbuf->st_nlink = 1;
		stbuf->st_size = 0; 
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

static int ytl_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			   off_t offset, struct fuse_file_info *fi)
{
	//This one can be hardcoded
	if(strcmp(path, "/") == 0)
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		filler(buf, "albums", NULL, 0);
		filler(buf, "decades", NULL, 0);
		filler(buf, "all", NULL, 0);
	}
	else 
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		char* dirName = getDirName(path);
		while(dirName != NULL)
		{
			filler(buf, dirName, NULL, 0);
			printf("fill dir %s\n", dirName);
			dirName = getDirName(path);
		}
	}
		
	return 0;
}

static int ytl_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
	if(strcmp(path, tmpAttrPath) != 0)
	{
		char* index;
		index = strchr(path, '/');
		index = strchr(index+1, '/');
		if(index != NULL || (mode & S_IFREG) == 0)
		{
			printf("FAIL MKNOD %s\n", path);
			return -1;
		}
		strcpy(tmpAttrPath, path);
		strcpy(tmpPath, getenv("HOME"));
		strcat(tmpPath, "/.cache");
		strcat(tmpPath, path);
		printf("TmpPath %s | tmpAttrPath %s\n", tmpPath, tmpAttrPath);
		res = mknod(tmpPath, mode, rdev);
		if (res == -1)
			return -errno;
	}
	return 0;
}

static int ytl_chmod(const char *path, mode_t mode)
{
/*
	int res;
	ytl_realPath(realPath, path);

	res = chmod(realPath, mode);
	if (res == -1)
		return -errno;
*/
	return 0;
}

static int ytl_chown(const char *path, uid_t uid, gid_t gid)
{/*
	int res;

	ytl_realPath(realPath, path);
	res = lchown(realPath, uid, gid);
	if (res == -1)
		return -errno;
*/
	return 0;
}


static int ytl_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	if(strcmp(path, tmpAttrPath) == 0)
	{
		strcpy(realPath, tmpPath);
	}
	else
	{
		//get path from cache
		ytl_realPath(realPath, path);
	}
	
	printf("OPENING REAL PATH %s\n", realPath);
	res = open(realPath, fi->flags);
	if (res == -1)
	{
		printf("\n\nERORRRRRRRRR: %d\n\n", -errno);
		//return -errno;
		return 0;
	} 
	else
	{
		//printf("\n\nOPENED\n\n");
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
	//printf("\n\nREADING! %s\n\n", path);

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
		//printf("READ ERR %d\n", -errno);
		res = -errno;
	}
	//printf("READ OK %s %lx %lx\n", buf, (long)size, (long)offset);

	close(fd);
	return res;
}

static int ytl_write(const char *path, const char *buf, size_t size,
			 off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;

	if(strcmp(path, tmpAttrPath) == 0)
	{	
		writeTmp = 1;
		fd = open(realPath, O_WRONLY);
		if (fd == -1)
			return -errno;

		res = pwrite(fd, buf, size, offset);
		if (res == -1)
			res = -errno;

		close(fd);
		return res;
	}
	return -1;	
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
	.readdir	= ytl_readdir,
	.mknod		= ytl_mknod,	 
	.chmod		= ytl_chmod,
	.chown		= ytl_chown,
	.open		= ytl_open,
	.read		= ytl_read,
	.write		= ytl_write,
	.statfs		= ytl_statfs,
};

int main(int argc, char *argv[])
{	
	printf("STARTIN FUSE\n");
	initCache();
	writeTmp = 0;
	umask(0);
	return fuse_main(argc, argv, &ytl_oper, NULL);
}
