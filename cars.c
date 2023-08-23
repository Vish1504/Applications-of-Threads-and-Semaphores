#include<stdio.h>
#include<sys/types.h>
#include<semaphore.h>
#include<pthread.h>
#include <unistd.h>

int clear_cycle = 1;
int part[2], gen = 0;
char *parts[] = { "Engine", "Chassis", "Music System" };

sem_t ready;
int place[4] = { 0, 1, 2, 3 };

void * branch (void *arg)
{
  int i, j, k = 0;
  while (1)
    {
      sleep (1);
      sem_wait (&ready);
      if (clear_cycle == 1)
	{
	  i = k;
	  j = i + 1;
	  if (j == 3)
	    j = 0;
	  k = j;
	  part[0] = i;
	  part[1] = j;
	  printf ("Headquarters can take %s and %s\n", parts[i], parts[j]);
	  gen = 1;
	  clear_cycle = 0;
	}
      sem_post (&ready);
      printf ("Parts are being moved\n");
    }
}

void * assembly (void *arg)
{
  int p_id = (*(int *) arg);
  int flag = 1;
  while (1)
    {
      sleep (1);
      sem_wait (&ready);
      if (clear_cycle == 0)
	{
	  if (gen && part[0] != p_id && part[1] != p_id)
	    {
	      printf ("Branch %d has completed the assembly\n", p_id);
	      clear_cycle = 1;
	      gen = 0;
	      flag = 0;
	    }
	}
      sem_post (&ready);
      if (clear_cycle == 1 && flag == 0)
	printf ("The cycle is complete\n");
    }
}

int main ()
{
  pthread_t branch1, branch2, branch3, headquarters;
  sem_init (&ready, 0, 1);
  pthread_create (&headquarters, NULL, branch, &place[0]);
  pthread_create (&branch1, NULL, assembly, &place[1]);
  pthread_create (&branch2, NULL, assembly, &place[2]);
  pthread_create (&branch3, NULL, assembly, &place[3]);
  while (1);
  return 0;
}
