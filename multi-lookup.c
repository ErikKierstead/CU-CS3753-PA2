/*
 * File: lookup.c
 * Author: Andy Sayler
 * Project: CSCI 3753 Programming Assignment 2
 * Create Date: 2012/02/01
 * Modify Date: 2012/02/01
 * Description:
 * 	This file contains the reference non-threaded
 *      solution to this assignment.
 *  
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "queue.h"
//#include "queue.c"

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
pthread_mutex_t writeQueue;
pthread_mutex_t  readQueue;
pthread_mutex_t  decrement;

void* ReadFile(void* fileName){
    
    printf("blah\n");

    //Setup Local Vars and Handle void
    //   long* tid = threadid;
    long t;
    long numprint = 3;
    FILE* inputfp = (FILE*) fileName;
    char hostname[SBUFSIZE];
    char errorstr[SBUFSIZE];
    char firstipstr[INET6_ADDRSTRLEN];
 
    //Open Input File
    inputfp = fopen(fileName, "r");

    if(!inputfp){
        sprintf(errorstr, "Error Opening Input File: %s", (char*)fileName);
        perror(errorstr);
        printf("Error Opening Input File: %s", (char*)fileName);
        //break;
    }

    while(fscanf(inputfp, INPUTFS, hostname) > 0){

    // Lookup hostname and get IP string 
    //if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
    //    == UTIL_FAILURE){
    //    fprintf(stderr, "dnslookup error: %s\n", hostname);
    //    strncpy(firstipstr, "", sizeof(firstipstr));
    //    }

        printf("From File: %s\n", hostname);

        //printf("%s: %s\n", hostname, firstipstr);
        //pthread_mutex_lock(&writeQueue);
        
        while(queue_is_full(&q))
        {
            printf("is full with hostname %s\n", hostname);
            usleep((rand()%100)*10000+1000);
        }
        
        pthread_mutex_lock(&writeQueue);
        //printf("pushed\n");    
        queue_push(&q, hostname);

        //int queue_push(queue* q, void* new_payload)
        //pthread_mutex_lock(&writeQueue);
       
        pthread_mutex_unlock(&writeQueue);
        // Write to Output File
        //fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
    }


//pthread_mutex_t writeQueue;
    //usleep(1000000000);

    // Close Input File
    fclose(inputfp);
   
     return NULL;
}

void* WriteFile(void* fileName){
    
    printf("blah2\n");

    //Setup Local Vars and Handle void
    //   long* tid = threadid;
    long t;
    long numprint = 3;
    FILE* outputfp = (FILE*) fileName;
    char* hostPointer;
    char hostname[SBUFSIZE];
    //char* hostname;
    char errorstr[SBUFSIZE];
    char firstipstr[INET6_ADDRSTRLEN];
      

    //Open Output File
    outputfp = fopen(fileName, "w");

    if(!outputfp){
        sprintf(errorstr, "Error Opening Output File: %s", (char*)fileName);
        perror(errorstr);
        printf("Error Opening Output File: %s", (char*)fileName);
        //break;
    }

        while(queue_is_empty(&q))
        {
            printf("is empty with hostname %s\n", hostname);
            usleep((rand()%100)*10000+1000);
        }

        printf("popping queue..");

        pthread_mutex_lock(&writeQueue);
        hostPointer = queue_pop(&q);
        sprintf(hostname, "%s", hostPointer);

        printf("que_pop: %s", hostname);

        pthread_mutex_unlock(&writeQueue);

 //       if(dnslookup(

    /*while(fscanf(inputfp, INPUTFS, hostname) > 0){

    // Lookup hostname and get IP string 
    //if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
    //    == UTIL_FAILURE){
    //    fprintf(stderr, "dnslookup error: %s\n", hostname);
    //    strncpy(firstipstr, "", sizeof(firstipstr));
    //    }

        printf("%s: %s\n", hostname, firstipstr);

        pthread_mutex_lock(&writeQueue);
        
        while(queue_is_full(&q))
        {
            usleep((rand()%100)*10000+1000);
        }
            
        queue_push(&q, (void*)hostname);
        //int queue_push(queue* q, void* new_payload)
        //pthread_mutex_lock(&writeQueue);
       
        pthread_mutex_unlock(&writeQueue);
        // Write to Output File
        //fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
    }*/


//pthread_mutex_t writeQueue;
    //usleep(1000000000);

    // Close Input File
    fclose(outputfp);
   
     return NULL;
}



int main(int argc, char* argv[]){

    /* Local Vars */
    FILE* inputfp = NULL;
    FILE* outputfp = NULL;
    char hostname[SBUFSIZE];
    char errorstr[SBUFSIZE];
    char firstipstr[INET6_ADDRSTRLEN];
    int i;


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
    pthread_mutex_init(&readQueue, NULL);
    pthread_mutex_init(&decrement, NULL);
    
    //long num_threads = NUM_THREADS;
    long num_threads = argc - 1;
    //long num_threads = 1;

     // Spawn NUM_THREADS threads
    for(t=0;t<num_threads;t++){
	printf("In main: creating thread %ld\n", t);
	
        //cpyt[t] = t;
	rc = pthread_create(&(threads[t]), NULL, ReadFile, (void*)argv[t+1]); //was &(cpyt[t]));
	if (rc){
	    printf("ERROR; return code from pthread_create() is %d\n", rc);
	    exit(EXIT_FAILURE);
	}
        
        //usleep(1000000);
        //printf("\nExited ReadFile %s\n", (char*)queue_pop(&q)); 
    }

     //secondPool
     for(u=0;t<num_threads;u++){
      
          printf("In main: creating thread %ld\n", u);

        //cpyt[t] = t;
        rd = pthread_create(&(threadsWrite[u]), NULL, WriteFile, (void*)argv[u+1]); //was &(cpyt[t]));
        if (rd){
            printf("ERROR; return code from pthread_create() is %d\n", rd);
            exit(EXIT_FAILURE);
        }
     }

    /* Wait for All Theads to Finish */
    for(t=0;t<num_threads;t++){
	pthread_join(threads[t],NULL);
    }

    /* Wait for All Theads to Finish */
    for(t=0;t<num_threads;t++){
	pthread_join(threadsWrite[t],NULL);
    }

    printf("All of the threads were completed!\n");
    printf("All of the threads were completed!\n");    

    //return EXIT_SUCCESS;
//}

    //--------------------------------------------------

    /* Open Output File */
    outputfp = fopen(argv[(argc-1)], "w");
    if(!outputfp){
	perror("Error Opening Output File");
	return EXIT_FAILURE;
    }

    // Loop Through Input Files
    for(i=1; i<(argc-1); i++){
	
	// Open Input File 
	inputfp = fopen(argv[i], "r");
	if(!inputfp){
	    //sprintf(errorstr, "Error Opening Input File: %s", argv[i]);
	    perror(errorstr);
	    break;
	}	

	// Read File and Process
	while(fscanf(inputfp, INPUTFS, hostname) > 0){
	
	    // Lookup hostname and get IP string
	    if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
	       == UTIL_FAILURE){
		fprintf(stderr, "dnslookup error: %s\n", hostname);
		strncpy(firstipstr, "", sizeof(firstipstr));
	    }
	
	    // Write to Output File
	    //fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
	}

	//Close Input File
	fclose(inputfp);
    }

    // Close Output File
    //fclose(outputfp);

    return EXIT_SUCCESS;
}
