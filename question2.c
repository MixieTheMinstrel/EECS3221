#include <stdio.h>
#include <pthread.h>
#include  <stdlib.h>
#include  <string.h>
#include <unistd.h>


//************STRUCT DECLARATIONS*****************//
struct process
{
    int Time;
    int PID;
    struct process* next;
};


struct queue
{
    int numItems;
    struct process* head;
    struct process* tail;
};


//************FUNCTION DECLARATIONS*****************//
struct queue* queue_add_element(struct queue* s, const int processID, const int processTime);
struct queue* queue_remove_element(struct queue*);
struct process* queue_get_element(struct queue* s);


struct queue* queue_new(void);
struct queue* queue_free( struct queue* );

void queue_print( const struct queue* );
void queue_print_element(const struct process* );

void *long_term_scheduler_method(void *param);
void *short_term_scheduler_method(void *param);


//************GLOBAL VARIABLE DECLARATIONS*****************//
struct queue* newQueue = NULL;
struct queue* readyQueue = NULL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int flag = 1; // 1 == LONG-TERM; 0 == SHORT-TERM
int long_term_complete_flag = 0; //0 == running; 1 == terminated
int long_term_START_prompt = 0; //0 == first loop; 1 == more loop


//main program (kernel) will have two threads running --> two schedulers (short and long term)
int main() {

    //used to define range of TIME in rand generator (between 1 and 30)
    const int LOWER = 1;
    const int UPPER = 30;

    //pthreads long_term and short_term
    pthread_t long_term_scheduler;
    pthread_t short_term_scheduler;


    //different seed for random number generator
    srand((unsigned)time(NULL));

    //initialize queues
    newQueue = queue_new();
    readyQueue = queue_new();


    //create 100 new processes and populate the NEW queue with it; generate new time for each process
    for (int i = 1; i < 101; i++){
        int time = (rand() % (UPPER - LOWER + 1)) + LOWER;

        queue_add_element(newQueue, i, time);

    }

    long_term_scheduler = pthread_create(&long_term_scheduler, NULL, long_term_scheduler_method, NULL);
    //error with creation of thread.
    if(long_term_scheduler)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", (int) long_term_scheduler);
        exit(EXIT_FAILURE);
    }

    sleep(1);

    short_term_scheduler = pthread_create(&short_term_scheduler, NULL, short_term_scheduler_method, NULL);
    //error with creation of thread.
    if(short_term_scheduler)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", (int) short_term_scheduler);
        exit(EXIT_FAILURE);
    }

    sleep(1);

    pthread_join(long_term_scheduler, NULL);
    pthread_join(short_term_scheduler, NULL);

    printf("Program end.");

    return 0;

}

//long term scheduler
void *long_term_scheduler_method(void *param){

    //dequeue element from new queue and put into ready queue
    while (flag == 1){

        if (long_term_START_prompt == 0){
            printf("START: Long-term scheduler.\n");
        }


        pthread_mutex_lock(&mutex);

        //if there are still items to be put into readyQueue...
        if (readyQueue->numItems < 5 && newQueue->numItems > 0){
            struct process* temp;
            temp = queue_get_element(newQueue);
            queue_add_element(readyQueue, temp->PID, temp->Time);
            printf("ACTION: Adding process %d from new queue into ready queue. \n", temp->PID);
            queue_remove_element(newQueue);
            long_term_START_prompt = 1;

            //ready queue is empty, thread is finished.
        } else if (newQueue->numItems == 0){
            printf("STOP: Long-term scheduler TERMINATED.\n\n");
            printf("----------------------------------\n\n");
            long_term_complete_flag = 1;
            flag = 0;
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
            long_term_START_prompt = 1;
            return NULL;

            //ready queue transfers control to short-term scheduler
        } else {

            //display suitable logging message
            printf("STOP: Passing control to short-term scheduler.\n\n");
            printf("----------------------------------\n\n");

            //pass control to short-term scheduler
            flag = 0;
            long_term_START_prompt = 0;
            pthread_cond_broadcast(&cond);
            pthread_cond_wait(&cond, &mutex);

        }

        pthread_mutex_unlock(&mutex);
    }

}


//short term scheduler
void *short_term_scheduler_method(void *param){

    int time, pid;

    while (flag == 0){

        printf("START: Short-term-scheduler\n");

        pthread_mutex_lock(&mutex);

            //readyQueue goes 5 times.
            for (int i = 0; i < 5; i++){

                //if there are items remaining...
                if (readyQueue->numItems > 0){

                    //need to create new copy of element to be removed, otherwise free() removes the item in remove_element
                    printf("ACTION: taking process %d with time %d and putting in ready-queue\n", queue_get_element(readyQueue)->PID, queue_get_element(readyQueue)->Time);
                    time = queue_get_element(readyQueue)->Time - 2;
                    pid = queue_get_element(readyQueue)->PID;

                    printf("ACTION: process %d now has time %d\n", pid, time);
                    queue_remove_element(readyQueue);

                    //if Time left > 0 then back in the queue it goes
                    if (time > 0){
                        printf("ACTION: process %d goes back in the ready queue\n\n", pid);
                        queue_add_element(readyQueue, pid, time);
                    } else {
                        printf("ACTION: process %d is terminated!\n\n", pid);
                    }

                    //otherwise terminate thread
                } else {
                        printf("STOP: Short-term scheduler TERMINATED.\n\n");
                        printf("----------------------------------\n\n");
                        pthread_mutex_unlock(&mutex);
                        return NULL;

                }

            }

            //if long_term scheduler still running, carry on as usual
            if (long_term_complete_flag == 0){
                printf("STOP: Passing control to long-term scheduler.\n\n");
                printf("----------------------------------\n\n");
                //pass control to other thread, wait for other thread to give back broadcast
                flag = 1;
                pthread_cond_broadcast(&cond);
                pthread_cond_wait(&cond, &mutex);
                pthread_mutex_unlock(&mutex);
            } else {
                printf("STOP: Long-term scheduler terminated. Resuming short term scheduler...\n\n");
                printf("----------------------------------\n\n");
                pthread_mutex_unlock(&mutex);
            }



    }

}



//***********************QUEUE DATA STRUCTURE************************/


/* Will always return the pointer to queue */
struct queue* queue_add_element(struct queue* s, const int processID, const int processTime)
{
    struct process* p = malloc( 1 * sizeof(*p) );

    if( NULL == p )
    {
        fprintf(stderr, "IN %s, %s: malloc() failed\n", __FILE__, "list_add");
        return s;
    }

    p->PID = processID;
    p->Time = processTime;
    p->next = NULL;
    s->numItems++;


    if( NULL == s )
    {
        printf("Queue not initialized\n");
        free(p);
        return s;
    }
    else if( NULL == s->head && NULL == s->tail )
    {
        /* printf("Empty list, adding p->PID: %d\n\n", p->PID);  */
        s->head = s->tail = p;
        return s;
    }
    else if( NULL == s->head || NULL == s->tail )
    {
        fprintf(stderr, "There is something seriously wrong with your assignment of head/tail to the list\n");
        free(p);
        return NULL;
    }
    else
    {
        /* printf("List not empty, adding element to tail\n"); */
        s->tail->next = p;
        s->tail = p;
    }

    return s;
}


/* This is a queue and it is FIFO, so we will always remove the first element */
struct queue* queue_remove_element( struct queue* s )
{
    struct process* h = NULL;
    struct process* p = NULL;

    if( NULL == s )
    {
        printf("List is empty\n");
        return s;
    }
    else if( NULL == s->head && NULL == s->tail )
    {
        printf("Well, List is empty\n");
        return s;
    }
    else if( NULL == s->head || NULL == s->tail )
    {
        printf("There is something seriously wrong with your list\n");
        printf("One of the head/tail is empty while other is not \n");
        return s;
    }

    h = s->head;
    p = h->next;
    //we won't free it since we need the info in memory
    free(h);
    s->head = p;
    if( NULL == s->head )  s->tail = s->head;   /* The element tail was pointing to is free(), so we need an update */
    s->numItems--;

    return s;
}

struct process* queue_get_element( struct queue* s )
{
    struct process* h = NULL;
    struct process* p = NULL;

    if( NULL == s )
    {
        printf("List is empty\n");
        return NULL;
    }
    else if( NULL == s->head && NULL == s->tail )
    {
        printf("Well, List is empty\n");
        return NULL;
    }
    else if( NULL == s->head || NULL == s->tail )
    {
        printf("There is something seriously wrong with your list\n");
        printf("One of the head/tail is empty while other is not \n");
        return NULL;
    }

    return s->head;
}


/* ---------------------- small helper fucntions ---------------------------------- */
struct queue* queue_free( struct queue* s )
{
    while( s->head )
    {
        queue_remove_element(s);
    }

    return s;
}

struct queue* queue_new(void)
{
    struct queue* p = malloc( 1 * sizeof(*p));

    if( NULL == p )
    {
        fprintf(stderr, "LINE: %d, malloc() failed\n", __LINE__);
    }

    p->head = p->tail = NULL;

    return p;
}


void queue_print( const struct queue* ps )
{
    struct process* p = NULL;

    if( ps )
    {
        for( p = ps->head; p; p = p->next )
        {
            queue_print_element(p);
            printf("numItems = %d\n", ps->numItems);
        }
    }

    printf("------------------\n");
}


void queue_print_element(const struct process* p )
{
    if( p )
    {
        printf("PID = %d\n", p->PID);
        printf("Time = %d\n", p->Time);
    }
    else
    {
        printf("Can not print NULL struct \n");
    }
}
