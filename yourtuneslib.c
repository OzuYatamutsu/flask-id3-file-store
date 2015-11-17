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
#define rootDir "/mnt/.fuse_remote\0"

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
    strcpy(pathBuffer, rootDir);
    strncat(pathBuffer, path, MAX_PATH_LENGTH - strlen(pathBuffer));
    getCachePath(NULL);
    strcpy(realPath, "/home/rhensey/TheSearch.mp3");
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
    /*
    int res;
    
    ytl_realPath(realPath, path);
    res = lstat(realPath, stbuf);
    if (res == -1)
        return -errno;
    */
    memset(stbuf, 0, sizeof(struct stat));
    if(strcmp(path, "/") == 0) 
    {
        stbuf->st_mode = S_IFDIR | 0755;    
    } else if (strcmp(path, "/test.mp3") == 0) 
    {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = 10;;    
    } else
    {
        return -ENOENT;
    }
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
    //DIR *dp;
    //struct dirent *de;

    (void) offset;
    (void) fi;

    ytl_realPath(realPath, path);
    /*dp = opendir(realPath);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        //st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }
    
    closedir(dp);
    */    
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, "test.mp3", NULL, 0);
    return 0;
}

static int ytl_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int res;

    ytl_realPath(realPath, path);
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



static struct fuse_operations ytl_oper = {
    .getattr    = ytl_getattr,
    .access        = ytl_access,
    .readlink    = ytl_readlink,
    .readdir    = ytl_readdir,
    .mknod        = ytl_mknod,
    .mkdir        = ytl_mkdir,
    .symlink    = ytl_symlink,
    .unlink        = ytl_unlink,
    .rmdir        = ytl_rmdir,
    .rename        = ytl_rename,
    .link        = ytl_link,
    .chmod        = ytl_chmod,
    .chown        = ytl_chown,
    .truncate    = ytl_truncate,
#ifdef HAVE_UTIMENSAT
    .utimens    = ytl_utimens,
#endif
    .open        = ytl_open,
    .read        = ytl_read,
    .write        = ytl_write,
    .statfs        = ytl_statfs,
#ifdef HAVE_POSIX_FALLOCATE
    .fallocate    = ytl_fallocate,
#endif
};

int main(int argc, char *argv[])
{    
    umask(0);
    return fuse_main(argc, argv, &ytl_oper, NULL);
}
