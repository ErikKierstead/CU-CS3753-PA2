/*
 * File: multi-lookup.c
 * Author for multi-lookup: Erik Kierstead
 * Author for single-lookup: Andy Sayler
 * Updated by Erik Kierstead

 * Project: CSCI 3753 Programming Assignment 2
 * Create Date: 2012/02/01
 * Modify Date: 2014/02/28
 * Description:
 * 	Multithreaded solution to the DNS
 *      requester / resolver threads.  
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "queue.h"
#include "multi-lookup.h"

#include "util.h"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"
#define NUM_THREADS 5
#define QUE_SIZE 10
#define MYSTACKSIZE 40960

//Global Variables:
queue q;
pthread_mutex_t  writeQueue;
pthread_mutex_t outputQueue;
pthread_mutex_t   decrement;

//Keeps Track of How Many Requester Threads Awake:
int openRequesters = 0;

int main(int argc, char* argv[]){

    /* Local Vars */
    //FILE* inputfp = NULL;
    FILE* outputfp = NULL;
    //char hostname[SBUFSIZE];
    //char errorstr[SBUFSIZE];
    //char firstipstr[INET6_ADDRSTRLEN];
    //int i;

    /* Check Arguments */
    if(argc < MINARGS){
	fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
	fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
	return EXIT_FAILURE;
    }

    // Setup Local Vars
    pthread_t threads[NUM_THREADS];
    int rc;
    long t;

    pthread_t threadsWrite[NUM_THREADS];
    int rd;
    long u;
 
    //Initialize Global Queue:
    if(queue_init(&q, QUE_SIZE) == QUEUE_FAILURE){
        fprintf(stderr, "error: queue_init failed!\n");
    }

    //Initialize Mutex Variables:
    pthread_mutex_init(&writeQueue, NULL);
    pthread_mutex_init(&outputQueue, NULL);
    pthread_mutex_init(&decrement, NULL);
    
    //long num_threads = NUM_THREADS;
    long num_threads = argc - 2;

    //Ensures the Output File is Created Blank:
    outputfp = fopen(argv[(argc-1)], "w");

     if(!outputfp){
                 perror("Error Opening Output File");
                 return EXIT_FAILURE;
             }


    fclose(outputfp);

    // Spawn Requester Threads (one thread per file):
    for(t=0;t<num_threads;t++){
	printf("In main: creating thread for first pool %ld\n", t);
	
        //Requester Threads Call Function ReadFile with Filename as Argument (argv[t+1]):
	rc = pthread_create(&(threads[t]), NULL, ReadFile, (void*)argv[t+1]);

	if (rc){
	    printf("ERROR; return code from pthread_create() is %d\n", rc);
	    exit(EXIT_FAILURE);
	}

        //Locks Requester Thread Count and Increments:
        pthread_mutex_lock(&decrement);
        openRequesters = openRequesters + 1;
        pthread_mutex_unlock(&decrement);
   
    }

     //Spawns Resolver Threads (one thread per file):
     for(u=0;u< num_threads;u++){
      
        printf("In main: creating thread for second pool %ld\n", u);

        //Resolver Threads Call Function WRiteFile with Filename as Last Argument in argv:
        rd = pthread_create(&(threadsWrite[u]), NULL, WriteFile, (void*)argv[(argc-1)]);

        if (rd){
            printf("ERROR; return code from pthread_create() is %d\n", rd);
            exit(EXIT_FAILURE);
        }
     }

    /* Wait for All Threads to Finish */
    for(t=0;t<num_threads;t++){
	pthread_join(threads[t],NULL);
    }

    /* Wait for All Threads to Finish */
    for(u=0;u<num_threads;u++){
	pthread_join(threadsWrite[u],NULL);
    }

    return EXIT_SUCCESS;
}

void* ReadFile(void* fileName){
    
    FILE* inputfp = (FILE*) fileName;
    char hostname[SBUFSIZE];
    char errorstr[SBUFSIZE];
 
    //Open Input File
    inputfp = fopen(fileName, "r");

    if(!inputfp){
        sprintf(errorstr, "Error Opening Input File: %s", (char*)fileName);
        perror(errorstr);
        printf("Error Opening Input File: %s", (char*)fileName);
    }


    while(fscanf(inputfp, INPUTFS, hostname) > 0){

        //Locks Mutex, checks if queue is full,
        //if full, unlock and sleep
        //if room, then make a new pointer, string copy,
        //    push pointer to queque, and unlocks:

        pthread_mutex_lock(&writeQueue);
        
        while(queue_is_full(&q))
        {
            pthread_mutex_unlock(&writeQueue);
   
            usleep((rand()%100)*10000+100);
            pthread_mutex_lock(&writeQueue);
        }
        
        //Malloc to New Pointer and String Copy:
        char* hostPointer = malloc(sizeof(hostname));
        strcpy(hostPointer, hostname);
 
        //Push to Queue:
        queue_push(&q, hostPointer);
        
        //free(hostPointer);

        pthread_mutex_unlock(&writeQueue);
    }

    //Decrements How Many Requesters Available:    
    pthread_mutex_lock(&decrement);
    openRequesters = openRequesters - 1;
    pthread_mutex_unlock(&decrement);

    // Close Input File
    fclose(inputfp);
   
    return NULL;
}

void* WriteFile(void* fileName){
    
    FILE* outputfp = (FILE*) fileName;

    char* hostPointer;
    char hostname[SBUFSIZE];
    char firstipstr[INET6_ADDRSTRLEN];
      

    //While there Exists Open Requesters OR Queue Not Empty (handles last cases):
    while(openRequesters > 0 || !queue_is_empty(&q))
    {

        pthread_mutex_lock(&writeQueue);

        //If Queue is Empty, Wait:
        if(queue_is_empty(&q))
        {
          pthread_mutex_unlock(&writeQueue);
          usleep((rand()%100)*1000+10000);
        }

        else
        {
             
             pthread_mutex_lock(&outputQueue);
             
             //Pops Pointer and String Copies to Var:
             hostPointer = queue_pop(&q);
             sprintf(hostname, "%s", (char*) hostPointer);

             //Performs DNS Lookup for Receive IP from Hostname:
             if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
                 == UTIL_FAILURE){
                     fprintf(stderr, "dnslookup error: %s\n", hostname);
                     strncpy(firstipstr, "", sizeof(firstipstr));
                     }


             //Malloc a New POinter and String Copies:
             char* ipPointer = malloc(sizeof(firstipstr));
             strcpy(ipPointer, firstipstr);

             /* Open Output File */
             outputfp = fopen(fileName, "a");
    
             //Appends to Output File:
             fprintf(outputfp, "%s,%s\n", hostname, (char*)ipPointer);
       
             free(ipPointer);

           //Close Output File to Relinquish Control to Other Threads:
             fclose(outputfp);

             pthread_mutex_unlock(&writeQueue);
             pthread_mutex_unlock(&outputQueue);
        }
    }

    // Close Input File
    printf("Closing Output File\n");

    return NULL;
}

