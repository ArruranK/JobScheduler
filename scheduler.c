#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "log.h"

// define priorities
#define high 0
#define medium 1
#define low 2

// define how many times in each queue
// in the high priority queue, a job can be twice
#define hpq_times 2
// in the medium priority queue, a job can be 4 times
#define mpq_times 4
// in the low priority quere, a job can be unilimited many times

int logindex = 0;
int *logi = &logindex;

pid_t create_job(int i);
void siga_handler();       // handler for signal SIGALRM
void sigc_handler();       // handler for signal SIGCHLD
sigset_t mymask1;          // normal signal process mask when all signals are free
                           // but SIGALRM and SIGCHLD are blocked
sigset_t mymask2;          // special signal process mask when all signals are free
sigset_t jobmask;          // job signal process mask blocking all signals, leaving
                           // only SIGUSR2 unblocked
struct sigaction sa_alarm; // disposition for SIGALRM
struct sigaction sa_chld;  // disposition for SIGCHLD

int number_of_jobs;



typedef struct Job
{
  pid_t pid;
  int hpqtimes;
  int mpqtimes;
  int priority;
  int jobDone;
  int jobNum;
} Job;

typedef struct JobQueue
{
  int first;
  int last;
  Job jobs[6];
} JobQueue;

JobQueue createQueue();
void addtoQueue(JobQueue *queue, Job *job);
Job popFromQueue(JobQueue *queue);
JobQueue hpq;
JobQueue mpq;
JobQueue lpq;
Job currentJob;


// function main -------------------------------------------------
int main(int argc, char **argv)
{
  int i, j;
  pid_t pid;



  if (argc != 2)
  {
    printf("Incorrect number of command line arguments \n");
    exit(1);
  }

  number_of_jobs = atoi(argv[1]);

  if (number_of_jobs < 3 || number_of_jobs > 6)
  {
    printf("Invalid number of jobs (Must be between 3-6) \n");
    exit(1);
  }



  create_log("assgn1.log");



  if ((sigemptyset(&mymask1) == -1) || (sigaddset(&mymask1, SIGCHLD) == -1) || (sigaddset(&mymask1, SIGALRM) == -1))
  {
    perror("Failed to setup mymask1");
  }




  else if (sigprocmask(SIG_SETMASK, &mymask1, NULL) == -1)
  {
    perror("Failed to block SIGCHLD and SIGALRM");
  }




  if ((sigemptyset(&mymask2) == -1))
  {
    perror("Failed to setup mymask2");
  }




  if ((sigfillset(&jobmask) == -1) || (sigdelset(&jobmask, SIGUSR2) == -1))
  {
    perror("Failed to setup jobmask");
  }


  sa_alarm.sa_handler = siga_handler;
  if (sigfillset(&sa_alarm.sa_mask) == -1)
  {
    perror("Failed to setup sa_mask for sa_alarm");
  }
  sa_alarm.sa_flags = SA_RESTART;


  if (sigaction(SIGALRM, &sa_alarm, NULL) == -1)
  {
    perror("Failed to install SIGALRM disposition");
  }

  sa_chld.sa_handler = sigc_handler;
  if (sigfillset(&sa_chld.sa_mask) == -1)
  {
    perror("Failed to setup sa_mask for sa_chld");
  }
  sa_chld.sa_flags = SA_RESTART;


  if (sigaction(SIGCHLD, &sa_chld, NULL) == -1)
  {
    perror("Failed to install SIGCHLD disposition");
  }


  hpq = createQueue();
  mpq = createQueue();
  lpq = createQueue();


  for (i = 0; i < number_of_jobs; i++)
  {
    pid = create_job(i);
    Job job;
    job.pid = pid;
    job.hpqtimes = 1;
    job.mpqtimes = 0;
    job.priority = high;
    job.jobDone = 0;
    job.jobNum = i;
    addtoQueue(&hpq, &job);
  }


  while (hpq.first != -1 || mpq.first != -1 || lpq.first != -1)
  {
    if (hpq.first != -1)
    {
      currentJob = popFromQueue(&hpq);
    }
    else if (mpq.first != -1)
    {
      currentJob = popFromQueue(&mpq);
    }
    else if (lpq.first != -1)
    {
      currentJob = popFromQueue(&lpq);
    }
    else
    {
      break;
    }

    if (kill(currentJob.pid, SIGUSR1))
    {
      perror("Failed to send signal SIGUSR1 to job");
    }
    if (currentJob.priority == high)
    {
      Msg("Switched on high-priority job %d", currentJob.jobNum);
      msg("Switched on high-priority job %d", currentJob.jobNum);
    }
    else if (currentJob.priority == medium)
    {
      Msg("Switched on medium-priority job %d", currentJob.jobNum);
      msg("Switched on medium-priority job %d", currentJob.jobNum);
    }
    else
    {
      Msg("Switched on low-priority job %d", currentJob.jobNum);
      msg("Switched on low-priority job %d", currentJob.jobNum);
    }

    alarm(1);
    sigsuspend(&mymask2);
  }
  Msg("All jobs done");
  msg("All jobs done");



  return 0;
} // end function main

// function create_job --------------------------------------------
pid_t create_job(int i)
{
  pid_t pid;
  char argv0[10];
  char argv1[10];



  if (sigprocmask(SIG_SETMASK, &jobmask, NULL) == -1)
  {
    perror("Failed to switch signal mask from mymask1 to jobmask");
  }


  if ((pid = fork()) < 0)
    Sys_exit("fork error\n");
  if (pid == 0)
  { // child process
    strcpy(argv0, "job");
    sprintf(argv1, "%d", i);
    execl("job", argv0, argv1, NULL);
  }


  if (sigprocmask(SIG_SETMASK, &mymask1, NULL) == -1)
  {
    perror("Failed to switch signal mask from jobmask to mymask1");
  }


  return pid;
} // end function create_job

// function siga_handler ------------------------------------------
void siga_handler()
{

  if (kill(currentJob.pid, SIGUSR2) == -1)
  {
    perror("Failed to send SIGUSR2 to job");
  }
  if (currentJob.hpqtimes < hpq_times)
  {
    currentJob.hpqtimes++;
    addtoQueue(&hpq, &currentJob);
    Msg("Switched off high-priority job %d", currentJob.jobNum);
    msg("Switched off high-priority job %d", currentJob.jobNum);
  }
  else if (currentJob.mpqtimes < mpq_times)
  {
    if (currentJob.priority == high)
    {
      currentJob.priority = medium;
      Msg("Switched off high-priority job %d", currentJob.jobNum);
      msg("Switched off high-priority job %d", currentJob.jobNum);
    }
    else
    {
      Msg("Switched off medium-priority job %d", currentJob.jobNum);
      msg("Switched off medium-priority job %d", currentJob.jobNum);
    }
    currentJob.mpqtimes++;
    addtoQueue(&mpq, &currentJob);
  }
  else
  {
    if (currentJob.priority == medium)
    {
      currentJob.priority = low;
      Msg("Switched off medium-priority job %d", currentJob.jobNum);
      msg("Switched off medium-priority job %d", currentJob.jobNum);
    }
    else
    {
      Msg("Switched off low-priority job %d", currentJob.jobNum);
      msg("Switched off low-priority job %d", currentJob.jobNum);
    }
    addtoQueue(&lpq, &currentJob);
  }

  return;
} // end function siga_handler

// function sigc_handler ------------------------------------------
void sigc_handler()
{

  Msg("job %d done", currentJob.jobNum);
  msg("job %d done", currentJob.jobNum);
  currentJob.jobDone = 1;
  return;
} // end function sigc_handler


JobQueue createQueue()
{
  JobQueue queue;
  queue.first = -1;
  queue.last = -1;
  return queue;
}

void addtoQueue(JobQueue *queue, Job *job)
{
  if (queue->first == -1)
  {
    queue->first = 0;
    queue->last = 0;
  }
  else
  {
    queue->last = queue->last + 1;
  }

  if (queue->last > 5)
  {
    queue->last = 0;
  }

  queue->jobs[queue->last] = *job;
}

Job popFromQueue(JobQueue *queue)
{
  if (queue->first != -1)
  {
    Job t = queue->jobs[queue->first];
    if (queue->first == queue->last)
    {
      queue->first = -1;
    }
    else if (queue->first != 5)
    {
      queue->first = queue->first + 1;
    }
    else
    {
      queue->first = 0;
    }
    return t;
  }
  else
  {
    perror("popping from empty queue");
    return;
  }
}


