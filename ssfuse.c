 #define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include "ssaux.h"

extern struct ss_state *ss_data;

int do_getattr( const char *path, struct stat *st )
{
	printf( "[getattr] Called\n" );
	printf( "\tAttributes of %s requested\n", path );
	
	if ( strcmp( path, "/" ) == 0 )
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
	else
	{
		st->st_mode = S_IFREG | 0444;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	
	return 0;
}

int ss_unlink(const char *path)
{
	int retstat=0;
	char fp[PATH_MAX];
	char * p =strchr(path,'>');
	if(p)
	{
		convPath(fp,p);
		if(unlink(fp)==-1)
			retstat=-1;
	}
	else
	{
		retstat=-1;
	}
	return retstat;
}

int ss_opendir(const char *path, struct fuse_file_info *fi)
{   

    return 0;
}
int ss_getattr(const char *path, struct stat *statbuf)
{
	int retstat=0;
	char fp[PATH_MAX];
	char * p =strchr(path,'>');
	if(p)
	{
		convPath(fp,p);
		stat(fp,statbuf);
	}else statbuf->st_mode=S_IFDIR | 0666; 


	return retstat;
}

/*int ss_getattr(const char *path, struct stat *statbuf)
{
    int retstat = 0;	
    statbuf->st_mode=S_IFDIR | 0666;    
    return retstat;
}*/
int ss_open(const char *path, struct fuse_file_info *fi)
{
	int retstat = 0;
	int fd ;
	char fp[PATH_MAX];
	char * p =strchr(path,'>');
	if(p)
	{
		convPath(fp,p);
		fd = open(fp,fi->flags);
		if(fd<0) retstat=-1;
		fi->fh=fd;
	}
	return retstat;
}
int ss_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	int retstat = 0;

	if(pread(fi->fh,buf,size,offset)==-1)
	{
		retstat=-1;
	}

	return retstat;

}

int ss_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
  
				
	filler( buffer, ".", NULL, 0 );
	filler( buffer, "..", NULL, 0 );
	
	
	if ( strcmp( path, "/" ) == 0 )
	{
		linkSet *ls=createLSet();
	  	findExt(ss_data->rootdir,buffer,filler,ls);	
	  	ls_destroy(ls);
	}
	else
	{
		char *p = strrchr(path,'/');
		if(p[1]=='+')
		{
			p[1]='.';

			readAllExt(ss_data->rootdir,&p[1],buffer,filler);		
		}
	}
	
	return 0;
}



struct fuse_operations ss_ops = {
    .getattr	= ss_getattr,
    .readdir	= ss_readdir,
    .open = ss_open,
    .read = ss_read,
    .unlink = ss_unlink,
    .opendir = ss_opendir,
    //.rmdir = ss_rmdir,
};


extern struct fuse_operations ss_ops;

int main(int argc, char *argv[])
{
  if ((getuid() == 0) || (geteuid() == 0))
  {
	  fprintf(stderr, "Running SSFS as root opens unnacceptable security holes\n");
	  return 1;
  } 
  
  if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
  {
    fprintf(stderr, "usage:  ssfs [FUSE and mount options] mountPoint rootDir\n");
    exit(1);
  }
  ss_data=malloc(sizeof(struct ss_state));
  if(ss_data==NULL){perror("malloc"); exit(1);}
  ss_data->rootdir=realpath(argv[2],NULL);
  argc--;
  return fuse_main(argc,argv,&ss_ops, (void*)ss_data);
}
