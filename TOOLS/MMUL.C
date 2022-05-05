#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <argstd.h>
#include <error.h>

#pragma library(wargstdr);

/*
 * A matrix is a set of strings
 */
struct mat {
	int w;
	int h;
	char **elements;
};

#define M(a,x,y) (*(((a)->elements+(a)->w*(y)+(x))))

/*
 * Special strings for values that are 1, 0 or -1
 */
char one[] = "<1>";
char zero[] = "<0>";
char minus_one[] = "<-1>";

char line[256];

struct mat *ReadMat(char *filename)
{
	FILE *fh;
	int i,j;
	struct mat *m;

	NEW_PTR(m);

	fh = FOPEN(filename,"r");

	fscanf(fh," %d %d ",&m->w,&m->h);

	NEW_PTR_N(m->elements,m->w*m->h);

	for(i=0; i< m->h; i++) {
		for(j=0; j< m->w; j++) {
			fscanf(fh," %s ",line);
			if(!strcmp(line,"1"))
				M(m,j,i) = one;
			else if(!strcmp(line,"0"))
				M(m,j,i) = zero;
			else if(!strcmp(line,"-1"))
				M(m,j,i) = minus_one;
			else if(!strcmp(line,"<1>"))
				M(m,j,i) = one;
			else if(!strcmp(line,"<0>"))
				M(m,j,i) = zero;
			else if(!strcmp(line,"<-1>"))
				M(m,j,i) = minus_one;
			else
				M(m,j,i) = strdup(line);
		}
	}
			
	FCLOSE(fh);	

	return m;
}

void WriteMat(struct mat *m)
{
	int i,j;

	printf("%d %d\n",m->w,m->h);

	for(i=0; i< m->h; i++) {
		for(j=0; j< m->w; j++) {
			printf("%10s  ",M(m,j,i));
		}
		printf("\n");
	}
}

char *Mul(char *a, char *b)
{
	char r[256];
	
	if(a == zero || b == zero)
		return zero;

	if(a == one)
		return b;

	if(b == one)
		return a;

	sprintf(r,"(%s*%s)",a,b);

	return strdup(r);
}

char *Add(char *a, char *b)
{
	char r[256];
	
	if(a == zero)
		return b;

	if(b == zero)
		return a;

	sprintf(r,"(%s+%s)",a,b);

	return strdup(r);
}


struct mat *MatMul(struct mat *a,struct mat *b)
{
	int i,j,k;
	struct mat *m;
	char *r;

	if(a->w != b->h)
		ERROR0("Cannot multiply");

	NEW_PTR(m);
	m->w = b->w;
	m->h = a->h;
	NEW_PTR_N(m->elements,m->w*m->h);

	for(i=0; i<m->h; i++) {
		for(j=0; j<m->w; j++) {
			r = zero;
			for(k=0; k< a->w; k++)
				r = Add(r,Mul(M(a,k,i),M(b,j,k)));

			M(m,j,i) = r;
		}
	}

	return m;
}

void main(int argc, char **argv)
{
	struct mat *a,*b,*c;

	/*
	 * Read in the two input matrices
	 */
	a = ReadMat(argv[1]);
	b = ReadMat(argv[2]);

	c = MatMul(a,b);

	WriteMat(c);
}

