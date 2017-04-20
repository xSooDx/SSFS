#include<string.h>
#include<dirent.h>
#include<stdlib.h>

#define DICT_LIM  200
char *EXTS[DICT_LIM];
int dict_n = 0;

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
	if (l==NULL)return -1;	
	if(l->first==NULL)
	{
		ls_node * nn = malloc(sizeof(ls_node));
		nn->next=NULL;
		nn->data = s;
		l->first=nn;
		l->last=nn;
		return 1;
	}
	if(strcmp(l->first->data,s)==0) return -1;
	ls_node *n=l->first;
	while(n->next)
	{
		if(strcmp(n->next->data,s)==0) return -1;
		n=n->next;		
	}

	ls_node * nn = malloc(sizeof(ls_node));
	nn->next=NULL;
	nn->data = s;
	n->next=nn;
	return 1;
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
