//GAS STATION 

#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
#include<stdlib.h>

#define carsno 1000
#define atten 3
#define maxcars 7
#define pumpsno 3
#define len 4

pthread_t ts[1008];
pthread_t cash_counter;
sem_t maxcap;
int freepump=pumpsno;
sem_t mut2;
int freepos=len;
sem_t mutqueue;
sem_t semq;
sem_t sempump;
int pump[pumpsno];
sem_t readycar;
sem_t fin[carsno];
sem_t done[carsno];
sem_t fincounter;
sem_t leavepump[pumpsno];
sem_t mut3;
sem_t mut4;
sem_t readypayment;
sem_t receipt[carsno];
int pipe1[2],pipe2[2];

void writepipe(int writefd,int val)
{
    if(write(writefd,&val,sizeof(int)) != sizeof(int)) 
    {
        printf("Pipe write error");
        exit(0);
    }
}
void readpipe(int readfd,int *val,int *stat)
{
    int n;
    if((n=read(readfd,val,sizeof(int))) == -1) 
    {
        printf ("Pipe read error");
        exit(0);
    }
    *stat=(char)n;
}

void Attender(int number);
void PAY();
void Car(int Car_Id);
void serveCar(int myCar,int number,int myPump);
void acceptPayment();
void CarMaker();
void AttenderMaker();

int main() 
{
    int is=time(NULL);
    srand(is);
    int i;
    sem_init(&maxcap,0,maxcars);
    sem_init(&mut2,0,1);
    sem_init(&mut3,0,1);
    sem_init(&mut4,0,1);
    sem_init(&mutqueue,0,1);
    sem_init(&semq,0,len);
    sem_init(&sempump,0,pumpsno);
    sem_init(&readycar,0,0);
    sem_init(&fincounter,0,0);
    sem_init(&readypayment,0,0);

    for(i=0;i<carsno;i++) 
    {
        sem_init(&fin[i],0,0);
        sem_init(&receipt[i],0,0);
    }
    for(i=0;i<pumpsno;i++) 
    {
        sem_init(&leavepump[i],0,0);
        pump[i]=-1;
    }

    if (pipe(pipe1) < 0)
        printf("Can't create pipe1\n");
    if (pipe(pipe2) < 0)
        printf("Can't create pipe2\n");
    AttenderMaker();
    pthread_create(&cash_counter,NULL,(void *)&PAY,NULL);
    CarMaker();
}

void PAY() 
{
    int Car;
    int myid=-1;
    int Mychairno = 0; 
    int stat;
    while(1) 
    {
        sem_wait(&readypayment);
        sem_wait(&mut4);
        readpipe(pipe2[0],&Car,&stat);
        sem_post(&mut4);
        printf("\nCASH COUNTER : Car %d has arrived with payment. Calling an attender\n",Car);
        sem_wait(&mut3);
        writepipe(pipe1[1],myid);
        writepipe(pipe1[1],Mychairno);
        sem_post(&mut3);
        sem_post(&readycar);
        printf("\nCASH COUNTER : Waiting payment confirmation from Car %d\n",Car);
        sem_wait(&fincounter);
        printf("\nPayment started\n");
        sleep(1);
        printf("\nCASH COUNTER : Car %d has paid\n",Car);
        sem_post(&receipt[Car]);
    }
}

void AttenderMaker()
{
    int i=1;
    while (i<=atten) {
        pthread_create(&ts[i],NULL,(void *)&Attender,(void *)i);
        i++;
    }
}

void CarMaker()
{
    int i=0;
    while (i <carsno) 
    {
        sleep(rand()%3);
        pthread_create(&ts[i+atten],NULL,(void *)&Car,(void *)i);
        printf("\nCar %d has arrived\n",i );
        i++;
    }
}

void Attender(int num) 
{
    int mycar, mypump, stat;
    while(1) 
    {
        printf("\nAttender %d waiting for a car\n",num);
        sem_wait(&readycar);
        sem_wait(&mut3);
        readpipe(pipe1[0],&mycar,&stat);
        readpipe(pipe1[0],&mypump,&stat);
        sem_post(&mut3);
        if (mycar!=-1)
            serveCar(mycar,num,mypump);
        else 
            acceptPayment();
    }
}

void acceptPayment()
{
    sem_post(&fincounter);
}

void serveCar(int mycar,int num,int mypump)
{
    printf("\nAttender %d fills gas in Car %d using Pump %d\n",num,mycar,mypump);
    sleep(3);
    printf("\nFilling completed for Car %d by Attender %d using Pump %d\n",mycar,num,mypump);
    sem_post(&fin[mycar]);
    sem_wait(&leavepump[mypump]);
    printf("\nAttender %d instructs Car %d to leave Pump %d and go to CASH COUNTER\n",num,mycar,mypump);
    sem_post(&sempump);
}

void Car(int carid) 
{
    int i,k;
    sem_wait(&maxcap);
    printf("\nCar %d enters the gas station\n",carid);
    sem_wait(&mut2);
    sem_wait(&mutqueue);
    if ((freepump==0) || (freepos<len))
    {
        sem_post(&mutqueue);
        sem_post(&mut2);
        sem_wait(&mutqueue);
        if (freepos<=0) 
            printf("\nCar %d is waiting\n",carid);
        freepos--;
        sem_post(&mutqueue);
        sem_wait(&semq);
        printf("\nCar %d is in the waiting lane now\n",carid);
        sem_wait(&sempump);
        sem_wait(&mutqueue);
        freepos++;
        sem_post(&mutqueue);
        sem_post(&semq);
        printf("\nCar %d has released its queue position\n",carid);
    }
    else 
    {
        sem_post(&mutqueue);
        sem_post(&mut2);
        sem_wait(&sempump);
    }
    sem_wait(&mut2);
    i = 0;
    while ((pump[i]!=-1) && (i<pumpsno))
    i++;
    if (i == pumpsno) 
        exit(0);
    else 
    {
        pump[i] = carid;
        freepump--;
        printf("\nCar %d occupies Pump %d. \nNumber of free pumps is %d\n",carid,i,freepump);
    }
    sem_post(&mut2);
    sem_wait(&mut3);
    printf("\nFilling for Car %d started\n",carid);
    writepipe(pipe1[1],carid);
    writepipe(pipe1[1],i);
    sem_post(&mut3);
    sem_post(&readycar);
    sem_wait(&fin[carid]);
    sem_wait(&mut2);
    freepump++;
    pump[i]=-1;
    sem_post(&leavepump[i]);
    sem_post(&mut2);
    printf("\nCar %d left Pump %d to go to the CASH COUNTER. \nNumber of free pumps is %d\n",carid,i,freepump);
    sem_wait(&mut4);
    writepipe(pipe2[1],carid);
    sem_post(&mut4);
    printf("\nCar %d ready to pay\n",carid);
    sem_post(&readypayment);
    sem_wait(&receipt[carid]);
    printf("\nCar %d has paid\n",carid);
    sem_post(&done[carid]);
    sem_post(&maxcap);
    if (carid==(carsno-1)) 
    {
        for(k=0;k<=(carsno-1);k++)
        sem_wait(&done[k]);
        puts("\nAll cars served");
        exit(0);
    } 
    else  pthread_exit(0); 
}
