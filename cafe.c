#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

sem_t tray_full_sem;

pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t tray_lock = PTHREAD_MUTEX_INITIALIZER; //access the tray number
pthread_mutex_t stud_wait_lock = PTHREAD_MUTEX_INITIALIZER; //when new student arrives
pthread_mutex_t stud_tot_lock = PTHREAD_MUTEX_INITIALIZER;//to access total no of students
pthread_mutex_t cook_sleep_lock = PTHREAD_MUTEX_INITIALIZER; //for cook's sleep state

int trays = 8; //current trays

int cook_sleep = 0;
char *cook_str[] = { "Working", "Sleeping" };

int tray_tot = 8; //total no of trays
int stud_tot = 0;
int stud_fetch = 0;

static time_t START_TIME;

void *cook (void *);
void *student (void *);
void screen (void);

int main ()
{
  pthread_t thread_cook;
  pthread_t thread_student;

  if (sem_init (&tray_full_sem, 0, 0) == -1)
    {
      perror ("sem_init");
      exit (EXIT_FAILURE);
    }
  
  START_TIME = time (NULL);
  printf ("\n\t\t WELCOME TO THE CAFETERIA %s\n", ctime (&START_TIME));
  pthread_create (&thread_cook, NULL, &cook, NULL); //cook created
  
  while (1)
    {
      sleep (rand () % 3 + 1); //time for student to arrive; 1-3 secs
      pthread_create (&thread_student, NULL, &student, NULL);
    }
  void *status;
  pthread_join (thread_cook, status);
  return 0;
}

void * cook (void *arg)
{
  pthread_mutex_lock (&print_lock);
  pthread_mutex_unlock (&print_lock);
  unsigned int random_time;
  time_t now;

  while (1)
    {
      pthread_mutex_lock (&tray_lock);
      if (trays == 8)
      //if there are 8 trays, the cook will sleep 
    {
      pthread_mutex_lock (&cook_sleep_lock);
      cook_sleep = 1;
      pthread_mutex_unlock (&cook_sleep_lock);
      pthread_mutex_lock (&print_lock);
      printf ("[ %ld ] - Cook goes to sleep \n",time (NULL) - START_TIME); //tells the time (sec) when cook goes to sleep
      screen();
      pthread_mutex_unlock (&print_lock);
      pthread_mutex_unlock (&tray_lock); // releases tray lock before going to sleep
      sem_wait (&tray_full_sem);
      pthread_mutex_lock (&cook_sleep_lock);
      cook_sleep = 0;
      pthread_mutex_unlock (&cook_sleep_lock);
      pthread_mutex_lock (&print_lock);
      printf ("[ %ld ] - Cook is awake \n", time (NULL) - START_TIME); //tells the time (sec) when cook is awake
      screen ();
      pthread_mutex_unlock (&print_lock);
    }
      else
        pthread_mutex_unlock (&tray_lock);

      random_time = rand () % 5 + 2; //time taken to fill a tray  
      now = time (0);

      pthread_mutex_lock (&print_lock);
      printf ("[ %ld ] Cook starts filling tray no. %d\n", time (NULL) - START_TIME, tray_tot + 1); 
      screen ();
      pthread_mutex_unlock (&print_lock);
      
      while (time (0) - now < random_time); //wait till the tray is being filled. 
      
      pthread_mutex_lock (&tray_lock);
      trays++;
      pthread_mutex_unlock (&tray_lock);
      tray_tot++;
      pthread_mutex_lock (&print_lock);
      printf ("[ %ld ] Cook finishes filling tray no. %d\n", time (NULL) - START_TIME, tray_tot);
      screen ();
      pthread_mutex_unlock (&print_lock);
    }
}

void * student (void *arg)
{
  pthread_mutex_lock (&stud_tot_lock);
  stud_tot++;
  pthread_mutex_unlock (&stud_tot_lock);
  pthread_mutex_lock (&print_lock);
  printf ("[ %ld ] Student no. %d arrives \n", time (NULL) - START_TIME, stud_tot); 
  screen ();
  pthread_mutex_unlock (&print_lock);
  pthread_mutex_lock (&stud_wait_lock);
  while (trays == 0); //there are no trays so the student waits
  sleep (1);
  pthread_mutex_lock (&tray_lock);
  trays--;
  
  if (trays == 7) //waking up the cook
    {
      pthread_mutex_lock (&cook_sleep_lock);
      if (cook_sleep == 1)
      sem_post (&tray_full_sem);
      pthread_mutex_unlock (&cook_sleep_lock);
    }
    
  stud_fetch++;
  pthread_mutex_lock (&print_lock);
  printf ("[ %ld ] Student no. %d fetches his tray \n", time (NULL) - START_TIME, stud_fetch);
  screen ();
  pthread_mutex_unlock (&print_lock);
  pthread_mutex_unlock (&tray_lock);
  pthread_mutex_unlock (&stud_wait_lock);
  pthread_exit (NULL);
}

void screen (void)
{
  printf ("\n|-------------------------------------------------------|\n");
  printf ("|\t\t\t\t\t\tCONVEYOR \t\t\t\t\t\t|\n");
  printf ("|\t\t\t\t---------------------- \t\t\t\t\t|\n");
  printf ("|\t\tTRAYS READY\t:%d \t\t\t\t\t\t\t\t\t|\n", trays);
  printf ("|\t\tTRAYS AVAILABLE\t:%d \t\t\t\t\t\t\t\t|\n", (8 - trays));

  printf ("|\tCOOK\t\t\t\t\tWAITING LINE \t\t\t\t|\n");
  printf ("|  ---------\t\t       ---------------- \t\t\t|\n");
  printf ("|  %s\t\t\t\t      %d \t\t\t\t\t\t|\n", cook_str[cook_sleep],
      (stud_tot - stud_fetch));
  printf ("|\t\t\t\t\t\t    Students waiting \t\t\t|\n");
  if (cook_sleep == 0)
    printf ("|  FILLING TRAY NO. %d \t\t\t\t\t\t\t\t\t|\n", (tray_tot + 1));
  printf ("|\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\n");

  printf ("|\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\n");
  printf ("|\t\t    CAFETERIA STATISTICS \t\t\t\t\t\t|\n");
  printf ("|\t\t---------------------------- \t\t\t\t\t|\n");
  printf ("|\t\tNO. OF TRAYS FILLED\t\t:%d\t\t\t\t\t\t|\n", tray_tot);
  printf ("|\t\tNO. OF STUDENTS ARRIVED\t:%d\t\t\t\t\t\t|\n", stud_tot);
  printf ("|\t\tNO. OF TRAYS FETCHED\t:%d\t\t\t\t\t\t|\n", stud_fetch);
  printf ("|-------------------------------------------------------|\n\n");
}
