scheduler: scheduler.o log.o job
	gcc -o scheduler scheduler.o log.o 
scheduler.o: scheduler.c log.h
	gcc -c scheduler.c 
log.o: log.c log.h
	gcc -c log.c 
job.o: job.c log.h
	gcc -c job.c
job: job.o log.o
	gcc -o job job.o log.o
