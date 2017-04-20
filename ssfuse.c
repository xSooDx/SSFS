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

int ss_opendir(const char *path, struct fuse_file_info *fi)
{    
	DIR *d;
	char* s_ext = strrchr(path,'+')+1;


    return 0;
}

int ss_getattr(const char *path, struct stat *statbuf)
{
    int retstat = 0;
    statbuf->st_mode=S_IFDIR | 0666;    
    return retstat;
}

int ss_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
  typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
				const struct stat *stbuf, off_t off);
				
	filler( buffer, ".", NULL, 0 );
	filler( buffer, "..", NULL, 0 );
	
	
	if ( strcmp( path, "/" ) == 0 )
	{
	  DIR *d = opendir(ss_data->rootdir);
	  struct  dirent *e;
		while(e=readdir(d))
		{
			if(!(strcmp(e->d_name,".")==0 || strcmp(e->d_name,"..")==0))
		  	{	
		  		char *ext = getExtension(e->d_name);
		  		if(ext)
		  		{
		  			ext[0]='+';		  			
		  			struct stat s;
		  			
  		  			filler( buffer, ext, NULL, 0 );
		  		}
  			}
  		
		}		
	}
	else
	{
		char *p = strrchr(path,'/');
		if(p[1]=='+')
		{
			p[1]='.';
			DIR *d = opendir(ss_data->rootdir);
	 		struct  dirent *e;
			while(e=readdir(d))
			{
				if(!(strcmp(e->d_name,".")==0 || strcmp(e->d_name,"..")==0))
			  	{	
			  		char *ext = getExtension(e->d_name);
			  		if(ext && strcmp(ext,&p[1])==0)
			  		{
  			  			filler( buffer, e->d_name, NULL, 0 );
			  		}
  				}
  			
			}		
		}
	}
	
	return 0;
}

int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
  char file54Text[] = "Hello World From File54!";
	char file349Text[] = "Hello World From File349!";
	char *selectedText = NULL;
	
	if ( strcmp( path, "/file54" ) == 0 )
		selectedText = file54Text;
	else if ( strcmp( path, "/file349" ) == 0 )
		selectedText = file349Text;
	else
		return -1;
	memcpy( buffer, selectedText + offset, size );
		
	return strlen( selectedText ) - offset;
}

struct fuse_operations ss_ops = {
    .getattr	= ss_getattr,
    .readdir	= ss_readdir,
    .read	= do_read,
    .opendir = ss_opendir,
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
