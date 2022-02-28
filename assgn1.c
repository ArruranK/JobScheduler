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

// TO DO
// your own data structure(s) to handle jobs and the 3 queues
// your own auxialiary variables

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
// END TO DO

// function main -------------------------------------------------
int main(int argc, char **argv)
{
  int i, j;
  pid_t pid;

  // TO DO
  // check the number of command line arguments, if not 2, terminate
  // the program with a proper error message on the screen.
  // check if the single command line argument (argv[1]) has value 3 to 6,
  // if not, treminate the program with a proper error message on the
  // screen.
  // set appropriately number_of_jobs

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

  // END TO DO

  create_log("assgn1.log");

  // TO DO
  // prepare mymask1 -- SIGCHLD and SIGALRM blocked, all other signals free
  // using sigemptyset(), sigfillset(), sigaddset(), sigdelset()

  if ((sigemptyset(&mymask1) == -1) || (sigaddset(&mymask1, SIGCHLD) == -1) || (sigaddset(&mymask1, SIGALRM) == -1))
  {
    perror("Failed to setup mymask1");
  }

  // END TO DO

  // TO DO
  // instal mymask1 as the process signal mask using sigrpocmask()
  else if (sigprocmask(SIG_SETMASK, &mymask1, NULL) == -1)
  {
    perror("Failed to block SIGCHLD and SIGALRM");
  }

  // END TO DO

  // TO DO
  // prepare mymask2 -- all signals free
  // using sigemptyset(), sigfillset(), sigaddset(), sigdelset()
  if ((sigemptyset(&mymask2) == -1))
  {
    perror("Failed to setup mymask2");
  }

  // END TO DO

  // TO DO
  // prepare jobmask -- all signals blocked except SIGUSR2
  // using sigemptyset(), sigfillset(), sigaddset(), sigdelset()
  if ((sigfillset(&jobmask) == -1) || (sigdelset(&jobmask, SIGUSR2) == -1))
  {
    perror("Failed to setup jobmask");
  }

  // END TO DO

  // TO DO
  // prepare SIGALRM disposition sa_alarm
  // its handler (sa_handler) is siga_handler()
  // its signal mask (sa_mask) must block all signals
  // its flags (sa_flags) must be set to SA_RESTART
  sa_alarm.sa_handler = siga_handler;
  if (sigfillset(&sa_alarm.sa_mask) == -1)
  {
    perror("Failed to setup sa_mask for sa_alarm");
  }
  sa_alarm.sa_flags = SA_RESTART;

  // END TO DO

  // TO DO
  // instal SIGALRM disposition using sigaction()
  if (sigaction(SIGALRM, &sa_alarm, NULL) == -1)
  {
    perror("Failed to install SIGALRM disposition");
  }

  // END TO DO

  // TO DO
  // prepare SIGCHLD disposition sa_chld
  // its handler (sa_handler) is sigc_handler()
  // its signal mask (sa_mask) must block all signals
  // its flags (sa_flags) must be set to SA_RESTART
  sa_chld.sa_handler = sigc_handler;
  if (sigfillset(&sa_chld.sa_mask) == -1)
  {
    perror("Failed to setup sa_mask for sa_chld");
  }
  sa_chld.sa_flags = SA_RESTART;

  // END TO DO

  // TO DO
  // instal SIGCHLD disposition using sigaction()
  if (sigaction(SIGCHLD, &sa_chld, NULL) == -1)
  {
    perror("Failed to install SIGCHLD disposition");
  }

  // END TO DO

  // TO DO
  // create empty high-priority queue
  // create empty medium-priority queue
  // create empty low-priority queue
  hpq = createQueue();
  mpq = createQueue();
  lpq = createQueue();
  // END TO DO

  // TO DO
  // create a data structure to keep information about jobs - PID, number of runs
  // for(i = 0; i < number_of_jobs; i++) {
  //   pid = create_job(i);
  //   save pid for job i in your data structure
  // }
  // put all jobs in the high-priority queue

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

  // END TO DO

  // TO DO
  // in a loop
  //    if all queues are empty
  //       record it in the log by Msg("All jobs done");
  //       and display it on the screen by msg("All jobs done");
  //       and terminate the loop
  //    "switch on" the first job from the highest-priority non-empty queue
  //    by sending it the SIGUSR1 signal (using sigsend())
  //    Record it in the log using
  //        Msg("Switched on high-priority job %d",job number);  or
  //        Msg("Switched on medium-priority job %d",job number); or
  //        Msg("Switched on low-priority job %d",job number);
  //    announce it on the screen using corresponding msg();
  //    set alarm for 1 second using alarm()
  //    switch the current signal process mask mymask1 to mymask2 while
  //    going to suspension using sigsuspend()
  //    (thus only SIGCHLD or SIGALRM will wake it up from suspension
  //    SIGCHLD indicates that the job that is currently executing just
  //    terminated, SIGALRM indicates that the time for the job currently
  //    executing is up and it must be "switched off")
  // end loop
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

  // END TO DO

  return 0;
} // end function main

// function create_job --------------------------------------------
pid_t create_job(int i)
{
  pid_t pid;
  char argv0[10];
  char argv1[10];

  // TO DO
  // switch process signal mask from mymask1 to jobmask
  // using sigprocmask()

  if (sigprocmask(SIG_SETMASK, &jobmask, NULL) == -1)
  {
    perror("Failed to switch signal mask from mymask1 to jobmask");
  }
  // END TO DO

  if ((pid = fork()) < 0)
    Sys_exit("fork error\n");
  if (pid == 0)
  { // child process
    strcpy(argv0, "job");
    sprintf(argv1, "%d", i);
    execl("job", argv0, argv1, NULL);
  }

  // parent process
  // TO DO
  // switch process signal mask from jobmask back to mymask1
  // using sigprocmask()
  if (sigprocmask(SIG_SETMASK, &mymask1, NULL) == -1)
  {
    perror("Failed to switch signal mask from jobmask to mymask1");
  }

  // END TO DO

  return pid;
} // end function create_job

// function siga_handler ------------------------------------------
void siga_handler()
{
  // TO DO
  // "switch of" the currently executing job by sending it SIGUSR2 signal
  // (using sigsend())
  // either put the job back to the queue it came from (you must count
  // how many times it has been through the queue) or "demote it" to the
  // lower-prority queue.
  // record this in the log using
  //     Msg("Switched off high-priority job %d",job number); or
  //     Msg("Switched off medium-priority job %d",job number); or
  //     Msg("Switched off low-priority job %d",job number);
  // announce it on the screen suing corresponding msg();
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
  // END TO DO
  return;
} // end function siga_handler

// function sigc_handler ------------------------------------------
void sigc_handler()
{
  // TO DO
  // disarm the alarm
  // record in the log that the currently executing job is done by
  // Msg("job %d done",job number);
  // END TO DO
  Msg("job %d done", currentJob.jobNum);
  msg("job %d done", currentJob.jobNum);
  currentJob.jobDone = 1;
  return;
} // end function sigc_handler

// TO DO
// functions for handling your data structures
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

// END TO DO
