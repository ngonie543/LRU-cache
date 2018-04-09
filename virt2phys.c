#include<stdio.h>
#include<stdlib.h>



int main(int argc, char *argv[]){
FILE *input=fopen(argv[1], "r");
int add;
fscanf(input, "%d", &add);
int pSize;
fscanf(input, "%d", &pSize);

int offset=0;

while(pSize>1){
	pSize=pSize/2;
	offset++;
}

int pageTableSize=add-offset;
int size=1<<pageTableSize;
int *pTable=(int *)malloc(size*sizeof(int));

for (int i=0; i<size; i++){
	fscanf(input, "%d", &pTable[i]);
}
fclose(input);

int virtAdd;
sscanf(argv[2],"%x",&virtAdd);
int pageNum=virtAdd>>offset;
int data =pTable[pageNum];

if (data==-1){
	printf("PAGEFAULT\n");
}
else{
int pOffset=virtAdd-(pageNum<<offset);
int physAddress= (data<<offset) +pOffset;
printf("%x\n",physAddress);

}
return 0;
}