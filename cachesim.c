#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int time=0;

int log2(int n){
int counter=0;
while(n>1){
	n=n/2;
	counter++;
}
  return counter;
}

int partial(int n,int s, int e){
  int r=(n>>e)<<e;
  r=(n-r)>>s;
  return r;
}

struct instr{
bool isLoad;
int addr;
int size;
char data[64];
bool end;

};

char hex(char c){
	if(c<='9'){
		return c-'0';
	}
	else{
		return c-'a'+10;
	}
}


void parse(char * str, char* data){
int n=strlen(str);

	int i =0;
	while(i < n / 2){
		data[i]=16*hex(str[i*2])+hex(str[i*2+1]);
		i++;
	}


}

struct instr readLine(FILE *input){
	char str[10]; 
	struct instr val;
	char data_str[128];
	if( fscanf(input, "%s", &str[0]) != EOF)
	{

	if(str[0]=='s'){
		val.isLoad=false;
		fscanf(input, "%x", &val.addr);
		fscanf(input, "%d", &val.size);
		fscanf(input, "%s", data_str);
		parse(data_str, val.data);
	}
	else{
		val.isLoad=true;
		fscanf(input, "%x", &val.addr);
		fscanf(input, "%d", &val.size);
	}
	val.end= false;

}
	else{
		val.end=true;
	}


return val;

}



struct block{
	int tag;
	int size;
	bool valid;
	char *data;
	int time;


};


struct set{
 struct block *list;
 int size;
};

struct cache{
	struct set *list;
	int size;
};

struct block createBlock(int size){
	struct block b;
	b.tag=0;
	b.size=size;
	b.valid=false;
	b.data=(char*) malloc(size);
	b.time=0;
	return b;
}

struct set createSet(int setSize, int blockSize){
	struct set s;
	s.list=(struct block*) malloc(setSize*sizeof(struct block));
	s.size=setSize;
	for (int i=0; i<setSize; i++){
		s.list[i]=createBlock(blockSize);
	}
	return s;
}


struct cache createCache(int cacheSize, int setSize, int blockSize){
	struct cache c;
	c.list=(struct set*) malloc(cacheSize*sizeof(struct set));
	c.size=cacheSize;
	for (int i=0; i<cacheSize;i++){
		c.list[i]=createSet(setSize,blockSize);
	}

	return c;
}


char memory [1<<24];

struct result{
	bool miss;
	char data[64];
};








struct result load(int tag, int size, int index, int offset, struct cache c){
	struct set s=c.list[index];
	struct block b;
 	struct result r;
 	
	for (int i=0; i<s.size; i++){
		if(s.list[i].valid==true){
			if(s.list[i].tag==tag){
				r.miss=false;
				time++;
				s.list[i].time=time;
				for(int j=0; j<size; j++){
					r.data[j]=s.list[i].data[j+offset];
				}
				return r;
			}

		}

	}
	r.miss=true;
    
	int indexSize=log2(c.size);
	int offestSize=log2(c.list[0].list[0].size);
    int memStartAddr=(tag<<(indexSize+offestSize))+(index<<offestSize);

    for(int i=0; i<s.size; i++){
    	if(s.list[i].valid==false){
    		s.list[i].tag=tag;
    		s.list[i].valid=true;
    		for (int j=0; j<s.list[i].size; j++){
    			s.list[i].data[j]=memory[memStartAddr+j];
    			//printf("0x%x\n", memStartAddr+j);
    		}

    		for(int k=0; k<size; k++){
					r.data[k]=s.list[i].data[k+offset];
				//	printf("offset 0x%x\n", k+offset);

				}
			time++;
			s.list[i].time=time;
			return r;
    	}

    }

    int minTime=s.list[0].time;
    int minIndex=0;

    for(int i=1; i<s.size; i++){
    	if (s.list[i].time<minTime){
    		minTime=s.list[i].time;
    		minIndex=i;
    	}
    }
    s.list[minIndex].tag=tag;
    s.list[minIndex].valid=true;
	for (int j=0; j<s.list[minIndex].size; j++){
		s.list[minIndex].data[j]=memory[memStartAddr+j];

	}

	for(int k=0; k<size; k++){
			r.data[k]=s.list[minIndex].data[k+offset];

		}
	time++;
	s.list[minIndex].time=time;
	return r;

}


bool store(int tag, int size, int index, int offset, struct cache c, char* data){
	struct set s=c.list[index];
	struct block b;

	int indexSize=log2(c.size);
	int offestSize=log2(c.list[0].list[0].size);
    int memStartAddr=(tag<<(indexSize+offestSize))+(index<<offestSize);




	for (int i=0; i<s.size; i++){
		if(s.list[i].valid==true){
			if(s.list[i].tag==tag){
				time++;
				s.list[i].time=time;
				for (int j=0; j<size;j++){
				s.list[i].data[j+offset]=data[j];
				memory[memStartAddr+j+offset]=data[j];

				}
				return false;
			}
		}
	}

	for (int j=0; j<size;j++){
		memory[memStartAddr+j+offset]=data[j];
	//	printf("storing 0x%x\n", memStartAddr+j+offset);
	}
	return true;
}




int main(int argc, char *argv[]){
FILE *input=fopen(argv[1], "r");
int cacheSize;
sscanf(argv[2],"%d",&cacheSize);
int assoc;
sscanf(argv[3],"%d",&assoc);
int blockSize;
sscanf(argv[4],"%d",&blockSize);
int numSets= (cacheSize<<10)/(assoc*blockSize);

struct cache c=createCache(numSets,assoc,blockSize);
struct instr list;




//printf("%d, %d\n", blockSize, numSets);
list= readLine(input);
while(list.end==false){
	int s=log2(blockSize)+log2(numSets);
	int e=24;
	int tag=partial(list.addr, s,e);
	int index=partial(list.addr,log2(blockSize),s);
	int offset=partial(list.addr, 0, log2(blockSize));
//printf("%d, %d\n", s,e);
	//printf("%d, %d, %d\n", tag,index,offset);

	if(list.isLoad==true){
		printf("load ");
		printf("0x%x ", list.addr);
		struct result res=load(tag,list.size,index,offset,c);
		if(res.miss){

		printf("miss ");
		}
		else{
			printf("hit ");
		}
		for(int i=0; i<list.size; i++){
			int hi=partial(res.data[i],4,8);
			printf("%x",hi);

			int lo=partial(res.data[i],0,4);
			printf("%x",lo);

		}

		printf("\n");
	}
	else{
		printf("store ");
		printf("0x%x ", list.addr);
		bool miss=store(tag,  list.size,  index,  offset, c,  list.data);
		if(miss==true){

		printf("miss\n");
		}
		else{
			printf("hit\n");
		}
	}

	list= readLine(input);

}



}