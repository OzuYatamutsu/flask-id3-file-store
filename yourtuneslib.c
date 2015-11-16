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

#define MAX_PATH_LENGTH 256
#define rootDir "/mnt/.fuse_remote\0"
char realPath[MAX_PATH_LENGTH];
char newRealPath[MAX_PATH_LENGTH];

static void ytl_realPath(char pathBuffer[], const char *path)
{
    strcpy(pathBuffer, rootDir);
    strncat(pathBuffer, path, MAX_PATH_LENGTH - strlen(pathBuffer));
}

static void getFileInfo(const char* realPath)
{
    printf("getting file info for %s\n", realPath);
    FILE *fp;
    char lineBuffer[1024];
    char command[256];
    snprintf(command, 256, "mp3info \"%s\"\n", realPath);
    
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
        printf("%s \n", lineBuffer);
    }

    pclose(fp);  
}

static int ytl_getattr(const char *path, struct stat *stbuf)
{
	int res;
    
    ytl_realPath(realPath, path);
	res = lstat(realPath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int ytl_access(const char *path, int mask)
{
	int res;

    ytl_realPath(realPath, path);
	res = access(realPath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int ytl_readlink(const char *path, char *buf, size_t size)
{
	int res;

    ytl_realPath(realPath, path);
	res = readlink(realPath, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


static int ytl_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

    ytl_realPath(realPath, path);
	dp = opendir(realPath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int ytl_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

    ytl_realPath(realPath, path);
	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(realPath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(realPath, mode);
	else
		res = mknod(realPath, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int ytl_mkdir(const char *path, mode_t mode)
{
	int res;

    ytl_realPath(realPath, path);
	res = mkdir(realPath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int ytl_unlink(const char *path)
{
	int res;

    ytl_realPath(realPath, path);
	res = unlink(realPath);
	if (res == -1)
		return -errno;

	return 0;
}

static int ytl_rmdir(const char *path)
{
	int res;

    ytl_realPath(realPath, path);
	res = rmdir(realPath);
	if (res == -1)
		return -errno;

	return 0;
}

static int ytl_symlink(const char *from, const char *to)
{
	int res;

    ytl_realPath(realPath, from);
    ytl_realPath(newRealPath, to);
	res = symlink(realPath, newRealPath);
	if (res == -1)
		return -errno;

	return 0;
}

static int ytl_rename(const char *from, const char *to)
{
	int res;

    ytl_realPath(realPath, from);
    ytl_realPath(newRealPath, to);
	res = rename(realPath, newRealPath);
	if (res == -1)
		return -errno;

	return 0;
}

static int ytl_link(const char *from, const char *to)
{
	int res;

    ytl_realPath(realPath, from);
    ytl_realPath(newRealPath, to);
	res = link(realPath, newRealPath);
	if (res == -1)
		return -errno;

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

#ifdef HAVE_UTIMENSAT
static int ytl_utimens(const char *path, const struct timespec ts[2])
{
	int res;

    ytl_realPath(realPath, path);
	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, realPath, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -errno;

	return 0;
}
#endif

static int ytl_open(const char *path, struct fuse_file_info *fi)
{
	int res;

    ytl_realPath(realPath, path);

    //Testing function
    getFileInfo(realPath);    

	res = open(realPath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int ytl_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

    ytl_realPath(realPath, path);

    //Testing function
    getFileInfo(realPath);

	fd = open(realPath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int ytl_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_WRONLY);
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

static int ytl_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;
	return 0;
}

static int ytl_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_POSIX_FALLOCATE
static int ytl_fallocate(const char *path, int mode,
			off_t offset, off_t length, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

	if (mode)
		return -EOPNOTSUPP;

    ytl_realPath(realPath, path);
	fd = open(realPath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = -posix_fallocate(fd, offset, length);

	close(fd);
	return res;
}
#endif

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int ytl_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{   
    ytl_realPath(realPath, path);
	int res = lsetxattr(realPath, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int ytl_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
    ytl_realPath(realPath, path);
	int res = lgetxattr(realPath, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int ytl_listxattr(const char *path, char *list, size_t size)
{
    ytl_realPath(realPath, path);
	int res = llistxattr(realPath, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int ytl_removexattr(const char *path, const char *name)
{
    ytl_realPath(realPath, path);
	int res = lremovexattr(realPath, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations ytl_oper = {
	.getattr	= ytl_getattr,
	.access		= ytl_access,
	.readlink	= ytl_readlink,
	.readdir	= ytl_readdir,
	.mknod		= ytl_mknod,
	.mkdir		= ytl_mkdir,
	.symlink	= ytl_symlink,
	.unlink		= ytl_unlink,
	.rmdir		= ytl_rmdir,
	.rename		= ytl_rename,
	.link		= ytl_link,
	.chmod		= ytl_chmod,
	.chown		= ytl_chown,
	.truncate	= ytl_truncate,
#ifdef HAVE_UTIMENSAT
	.utimens	= ytl_utimens,
#endif
	.open		= ytl_open,
	.read		= ytl_read,
	.write		= ytl_write,
	.statfs		= ytl_statfs,
	.release	= ytl_release,
	.fsync		= ytl_fsync,
#ifdef HAVE_POSIX_FALLOCATE
	.fallocate	= ytl_fallocate,
#endif
#ifdef HAVE_SETXATTR
	.setxattr	= ytl_setxattr,
	.getxattr	= ytl_getxattr,
	.listxattr	= ytl_listxattr,
	.removexattr	= ytl_removexattr,
#endif
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &ytl_oper, NULL);
}
