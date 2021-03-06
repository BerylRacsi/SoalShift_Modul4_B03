#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#include <stdlib.h>

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;

	res = lstat(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(path);
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

int ext_dilarang(const char *path)
{
	char dilarang[][8] = {".pdf",".doc",".txt"};
	const char *ext = strrchr(path,'.');

	for (int i=0; i<3; i++)
	{
		if (strcmp(ext,dilarang[i])==0)
		{
			return 1;
		}
	
	}
	return 0;
}

char *dirfolder(const char *path)
{
	char *dir = (char*) malloc(100*sizeof(char));
	strcpy(dir,path);
	*(strrchr(dir,'/')) = 0;
	return dir;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	if(strstr(path,"Documents") && ext_dilarang(path))
	{
		system("notify-send 'Terjadi Kesalahan! File berisi konten berbahaya'");
		
		char tambahan[100];
		sprintf(tambahan, "%s.ditandai", path);
		rename(path, tambahan);
		
		
		char folder[100];
		sprintf(folder,"%s/rahasia",dirfolder(path));
		DIR *rahasia = opendir(folder);
		if(rahasia == NULL)
		{
			mkdir(folder,0700);
		}
		
		char pindahan[100];
		sprintf(pindahan, "%s/%s", folder,(strrchr(tambahan,'/'))+1);
		rename(tambahan,pindahan);
		
		
		return -1;
	}

	else
	{
		int fd;
		int res;

		(void) fi;
		fd = open(path, O_RDONLY);
		if (fd == -1)
		return -errno;

		res = pread(fd, buf, size, offset);
		if (res == -1)
		res = -errno;

		close(fd);
		return res;
	}
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
