#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/times.h>


struct block{
  char** operations;
  int length;
};

struct block_array{
  struct block* blocks;
  int length;
};

struct file_sequence{
  char** seq;
  int length;
};

struct block_array* create(int num_of_blocks){
    struct block_array* arr = calloc(1,sizeof(struct block_array));
    arr->blocks = (struct block*) calloc(num_of_blocks,sizeof(struct block));
    arr->length = -1;

    return arr;
}

struct  file_sequence seq_def(char *str){
    struct file_sequence seq;
    char* ptr;
    int tmp = 1;
    ptr = str;
    while(*ptr){
        if(*ptr==' '){
            tmp++;
        }
        ptr++;
    }

    seq.seq = calloc(tmp, sizeof(char*));
    ptr = strtok(str," ");
    seq.seq[0] = calloc(strlen(ptr)+1,sizeof(char));
    strcpy(seq.seq[0],ptr);
    for(int k=1;k<tmp;k++){
        ptr = strtok(NULL," ");
        seq.seq[k]= calloc(strlen(ptr)+1,sizeof(char));
        strcpy(seq.seq[k],ptr);
    }

    seq.length = tmp;
    return seq;
}

char* compare(struct file_sequence seqence){
    char* com = calloc(200,sizeof(char));
    system("touch tmp");
    int i = 0;
    while(i<seqence.length)
    {
        strcpy(com,"diff ");
        strcat(com,seqence.seq[i++]);
        strcat(com, " ");
        strcat(com, seqence.seq[i++]);
        strcat(com, " >> tmp");


        system("echo \"@\n\" >> tmp");
        system(com);


    }

    return "tmp";
}


void create_blocks(struct block_array*  arr,char* file, int num_of_blocks){

    FILE* f;
    f = fopen(file, "r");
    if(f == NULL){
        exit(EXIT_FAILURE);
    }


    char line[200];
    char tmp_line[200];
    int already_read = 0;
    char buffer [20000];


    while(fgets(line,sizeof(line),f)!=NULL){

        if(line[0]=='@'){
            arr->length+=1;

            arr->blocks[arr->length].length=-1;
            int to_alloc = 0;
            int tested = 0;
            FILE* f1;
            f1 = fopen(file, "r");
            if(f1 == NULL){
                exit(EXIT_FAILURE);
            }
            while(fgets(tmp_line,sizeof(tmp_line),f1)!=NULL){
                tested++;
                if(tested>=already_read){
                    if(tmp_line[0]>='0'&&tmp_line[0]<='9'){
                        to_alloc++;
                        if(tmp_line[0]=='@'){
                            break;
                        }
                    }
                }

            }
            arr->blocks[arr->length].operations=calloc(to_alloc,sizeof(char*));

            fclose(f1);
        }

        if(line[0]>='0'&&line[0]<='9'){
            if(arr->blocks[arr->length].length>=0){
                strcpy(arr->blocks[arr->length].
                        operations[arr->blocks[arr->length].length],buffer);
            }
            buffer[0]='\0';
            arr->blocks[arr->length].length+=1;
            int tested = 0;
            int to_alloc = 0;
            FILE* f1;
            f1 = fopen(file, "r");
            if(f1 == NULL){
                exit(EXIT_FAILURE);
            }
            while(fgets(tmp_line,sizeof(tmp_line),f1)!=NULL){
                tested++;
                if(tested>=already_read){
                    to_alloc+=strlen(tmp_line)+1;

                    if(tmp_line[0]=='@'||(tmp_line[0]>='0'&&tmp_line[0]<='9')){
                        break;
                    }

                }

            }
            arr->blocks[arr->length].
                    operations[arr->blocks[arr->length].length] =
                    calloc(to_alloc+1,sizeof(char));
            strcat(buffer,line);
            fclose(f1);
        }


        if(!(line[0]>='0'&&line[0]<='9')){
            if(line[0]!='@'){
                strcat(buffer,line);
            }
        }

        already_read++;

    }


    fclose(f);



}

int num_of_operations(struct block_array* arr,int index){
    return arr->blocks[index].length;
}

void delete_block(struct block_array* arr,int index){

    int i = num_of_operations(arr,index);

    for(int j=0;j<i;j++){
       free(arr->blocks[index].operations[j]);
    }

    free(arr->blocks[index].operations);

}

void delete_operation(struct block* block_ptr,int index){
  free(block_ptr->operations[index]);
    block_ptr->operations[index] = NULL;
}





double calc_time(clock_t s, clock_t e){
    return ((double)(e-s) / (sysconf(_SC_CLK_TCK)));
}

void print_times(FILE * f,char* operation,clock_t s, clock_t e, struct tms * st, struct tms * et){
   printf("%s", operation);
    printf("\nREAL TIME: %f\n", calc_time(s,e));
    printf("USER TIME: %f\n", calc_time(st->tms_utime,et->tms_utime));
    printf("SYS TIME: %f\n", calc_time(st->tms_stime,et->tms_stime));
    
    fprintf(f,"%s", operation);
    fprintf(f,"\n REAL TIME: %lf", calc_time(s,e));
    fprintf(f,"\n USER TIME: %lf", calc_time(st->tms_utime,et->tms_utime));
    fprintf(f,"\n SYS TIME: %lf", calc_time(st->tms_stime,et->tms_stime));
}

int main(int argc, char* args[]){
    FILE *raport;

    struct block_array* arr;

    int n = 1;
    int num = 0;
    while(argc-n>0){
        if(strcmp(args[n],"create_table")==0){
            arr = create(atoi(args[n+1]));
            num = atoi(args[n+1]);
            n+=2;
        }
        else if(strcmp(args[n],"compare_pairs")==0){
            char buffer[200];

            buffer[0]='\0';

            for(int i=0; i<n+1;i++){
                strcat(buffer,args[n+2+i]);
                strcat(buffer," ");
            }
            struct file_sequence seq = seq_def(buffer);
            char *tmp = compare(seq);
            create_blocks(arr,tmp,num);
            n=n+1+atoi(args[n+1]);
        }else if(strcmp(args[n],"remove_block")==0){
            delete_block(arr,atoi(args[n+1]));
            n+=2;
        }else if(strcmp(args[n],"remove_operation")==0){
            delete_operation(&(arr->blocks[atoi(args[n+1])]),atoi(args[n+2]));
            n+=3;
        }
    }
    printf("TESTS");
    //Tests part
    raport = fopen("raport2.txt","w");
    if(raport==NULL)exit(EXIT_FAILURE);

    clock_t r_times[2];
    struct tms* t_times[2];
    t_times[0] = calloc(1,sizeof(struct tms));
    t_times[1] = calloc(1,sizeof(struct tms));

      r_times[0] = times(t_times[0]);

    struct block_array* array = create(4);
    r_times[1] = times(t_times[1]);

     print_times(raport,"\nCreating array",r_times[0],r_times[1],t_times[0],t_times[1]);

    r_times[0] = times(t_times[0]);
    char names[400];
    strcpy(names,"t1.txt t2.txt b1.txt b2.txt");
    struct file_sequence seq = seq_def(names);
    char *tmp = compare(seq);
    create_blocks(array,tmp,4);
    r_times[1] = times(t_times[1]);

     print_times(raport,"\nCreating blocks",r_times[0],r_times[1],t_times[0],t_times[1]);

    r_times[0] = times(t_times[0]);

    delete_block(array,1);
    delete_block(array,2);
    delete_block(array,3);
    delete_block(array,4);
    r_times[1] = times(t_times[1]);

    print_times(raport,"\nDeleting blocks",r_times[0],r_times[1],t_times[0],t_times[1]);

    
    
}
