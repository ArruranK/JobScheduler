# JobScheduler
Created a job scheduler as part of a concurrent systems course I took

The program takes in as arguments a number n from 1-6 and runs a scheduler with n jobs.
Each job runs for a random amount of time before exiting.
The job scheduler is made up of three priority queues, high, medium, and low. All jobs start in the high priority and move down to lower priorities based on how long they run.

This is the first time that I've worked with signals and concurrent programming and this project really helped me better understand the concepts I learned. 
