#include<string.h>
#include<dirent.h>
#include<stdlib.h>

typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
				const struct stat *stbuf, off_t off);

struct ss_state
{
  char* rootdir;
};

////LINKED SET////

struct lsn
{
	struct lsn *next;
	char* data;
};

typedef struct lsn ls_node;

typedef struct {
	ls_node *first;
	ls_node *last;
}
linkSet;

int ls_isMember(linkSet *l,const char* s)
{
	if(l==NULL) return -1;
	ls_node *n=l->first;
	while(n)
	{
		if(strcmp(n->data,s)==0) return 1;
		n=n->next;
	}
	return 0;
}

linkSet * createLSet()
{
	linkSet *n= malloc(sizeof(ls_node));
	n->first=NULL;
	n->last=NULL;
	return n;
}

int ls_addMember(linkSet *l, char *s)
{
	if (l==NULL)return 0;	
	if(l->first==NULL)
	{
		ls_node * nn = malloc(sizeof(ls_node));
		nn->next=NULL;
		nn->data = s;
		l->first=nn;
		l->last=nn;
		return 1;
	}
	if(strcmp(l->first->data,s)==0) return 0;
	ls_node *n=l->first;
	while(n->next)
	{
		if(strcmp(n->next->data,s)==0) return 0;
		n=n->next;		
	}

	ls_node * nn = malloc(sizeof(ls_node));
	nn->next=NULL;
	nn->data = s;
	n->next=nn;
	return 1;
}

void ls_destroy(linkSet *l)
{
	if(l->first)
	{
		ls_node *n=l->first;
		while(n->next){
			ls_node *t=n;
			n=n->next;
			free(t);
		}
		free(n);
	}
	else return;

}
 ////////////////////////////////


struct ss_state *ss_data;


static void ss_fullpath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, ss_data->rootdir);
    strncat(fpath, path, PATH_MAX); // ridiculously long paths will
				    // break here
}

char* getExtension(const char* path)
{
	return strrchr(path,'.');
}


void str_replace(char *s,const char c,const char w)
{
	while(s && s[0]!='\0')
	{
		if(s[0]==c) s[0]=w;
		s++;
	}
	return;
}

void readAllExt(char *pathname, const char *ext, void *buffer,fuse_fill_dir_t filler)
{
	//filler( buffer, "b", NULL, 0 );
    DIR *d;
    struct dirent *dir;
    d = opendir(pathname);
    while((dir = readdir(d)) != NULL)
    {
        printf("%s\t%u\n", dir->d_name, dir->d_type);
        if(dir->d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0)
        {
            char *str2 = malloc(strlen(pathname)+2+strlen(dir->d_name));
            strcpy(str2, pathname); /* copy name into the new var */
            strcat(str2, "/");
            strcat(str2, dir->d_name);
            readAllExt(str2,ext,buffer,filler);
        }
        else if (dir->d_type == DT_LNK)
        {
            char str1[40];
            int r = readlink(dir->d_name,str1,40);
            
        }
        else
        {
        	char *dext = getExtension(dir->d_name);
        	if(dext && strcmp(ext,dext)==0)
        	{
	            char fpt[PATH_MAX];
	            strcpy(fpt,pathname);
	            strcat(fpt,"/");
	            strcat(fpt,dir->d_name);
	            str_replace(fpt,'/','>');
	            filler(buffer,fpt,NULL,0);
	        }
        	
        }
    }
}

void findExt(char *pathname, void *buffer, fuse_fill_dir_t filler,linkSet *extSet)
{
	DIR *d;
    struct dirent *dir;
    d = opendir(pathname);
    while((dir = readdir(d)) != NULL)
    {
        printf("%s\t%u\n", dir->d_name, dir->d_type);
        if(dir->d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0)
        {
            char *str2 = malloc(strlen(pathname)+2+strlen(dir->d_name));
            strcpy(str2, pathname); /* copy name into the new var */
            strcat(str2, "/");
            strcat(str2, dir->d_name);
            findExt(str2,buffer,filler,extSet);
        }
        else if (dir->d_type == DT_LNK)
        {
            char str1[40];
            int r = readlink(dir->d_name,str1,40);
            
        }
        else
        {
        	char *ext = getExtension(dir->d_name);
	  		if(ext && !(strcmp(ext,".")==0 || strcmp(ext,"..")==0))
	  		{
	  			ext[0]='+';		  			
	  			if(ls_addMember(extSet,ext))		  			
		  				filler( buffer, ext, NULL, 0 );
	  		}
        }
    }

}

void convPath(char* buff, const char *path)
{
	strcpy(buff,path);
	str_replace(buff,'>','/');
}

