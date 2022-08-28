#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

unsigned int precision,cur_precision;
double upper_bound,lower_bound;
char up[100],low[100];

struct info
{
    unsigned long long int start;
    unsigned long long int range;
    double width;
    double area;
};

void thread(void *para)
{
    double sum_y = 0;
    struct info* tmp = (struct info*)para;
    for(unsigned long long int i=tmp->start;i<tmp->start+tmp->range;i++)
    {
        double x = i*tmp->width;
        double y = sqrt(1-x*x);
        sum_y += y;
    }
    tmp->area = sum_y*tmp->width;
}

unsigned int find(double a,double b)
{
    unsigned int precise = 0;
    char tmp_1[100];
    char tmp_2[100];
    sprintf(tmp_1,"%.30f",a);
    sprintf(tmp_2,"%.30f",b);
    strcpy(up,tmp_1);
    strcpy(low,tmp_2);
    for(int i=2;i<30;i++)
    {
        if(tmp_1[i]==tmp_2[i])
            precise++;
        else
            break;
    }
    return precise;
}

int main(int argc,char** argv)
{
    unsigned int numCPU;
    unsigned long long int partition,size;
    double base,test;
    pthread_t *tid;
    struct info *t_info;
    
    if(argc==2)
    {
        numCPU = sysconf(_SC_NPROCESSORS_ONLN);
        precision = atoi(argv[1]);
    }    
    else if(argc==3)
    {
        numCPU = atoi(argv[2]);
        precision = atoi(argv[1]);
    }    
    else
    {
        printf("syntax error:\nfirst argument is precision.\nsecond argument is CPU count.\n");
        exit(0);
    }

    tid = (pthread_t*)malloc(sizeof(pthread_t)*numCPU);
    t_info = (struct info*)malloc(sizeof(struct info)*numCPU);
    printf("CPU count : %d\n",numCPU);
    
    partition = 10000;
    while(cur_precision<precision)
    {
        upper_bound = 0;
        size = partition/numCPU;
        base = (double)1/partition;
        for(unsigned int i=0;i<numCPU;i++)
        {
            t_info[i].start = i*size;
            t_info[i].range = size;
            t_info[i].width = base;
            pthread_create(&tid[i],NULL,(void *)thread,(void *)&t_info[i]);
        }

        for(unsigned int i=0;i<numCPU;i++)
        {
            pthread_join(tid[i],NULL);
        }

        for(unsigned int i=0;i<numCPU;i++)
        {
            upper_bound += t_info[i].area;
        }
        lower_bound = upper_bound - base;
        cur_precision = find(upper_bound*4,lower_bound*4);

        if(cur_precision>=precision)
        {
            printf("goal percision : %d\ncurrent precision : %d\n",precision,cur_precision);
            printf("upper bound = %.10f\n",upper_bound*4);
            printf("lower bound = %.10f\n",lower_bound*4);
        }
        partition *= 10;
    }
    return 0;
}