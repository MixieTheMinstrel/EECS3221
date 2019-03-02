#include <stdio.h>
#include <pthread.h>
#include  <stdlib.h>
#include  <string.h>


//implement a struct with two elements --> simulates a process
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


struct queue* queue_add_element(struct queue* s, const int processID, const int processTime);
struct queue* queue_remove_element( struct queue*);


struct queue* queue_new(void);
struct queue* queue_free( struct queue* );

void queue_print( const struct queue* );
void queue_print_element(const struct process* );



//creating a new thread
//int pthread_create (pthread_t *thread_id, const pthread_attr_t *attributes, void *(*thread_function)(void *), void *arguments);

//main program (kernel) will have two threads running --> two schedulers (short and long term)
int main() {

    //used to define range of TIME in rand generator (between 1 and 30)
    const int LOWER = 1;
    const int UPPER = 30;

    //different seed for random number generator
    srand((unsigned)time(NULL));

    //create two queue's, ready queue and new queue
    struct queue*  newQueue = NULL;
    newQueue = queue_new();
    struct queue*  readyQueue = NULL;
    readyQueue = queue_new();


    //create 100 new processes and populate the NEW queue with it; generate new time for each process
    for (int i = 1; i < 101; i++){
        int time = (rand() % (UPPER - LOWER + 1)) + LOWER;

        queue_add_element(newQueue, i, time);

    }

    //print out queue for testing
    queue_print(newQueue);

    return 0;

}

//short term scheduler


//long term scheduler

//***********************QUEUE DATA STRUCTURE************************/
/* This Queue implementation of singly linked list in C implements 3
 * operations: add, remove and print elements in the list.  Well, actually,
 * it implements 4 operations, lats one is queue_free() but free() should not
 * be considered the operation but  a mandatory practice like brushing
 * teeth every morning, otherwise you will end up loosing some part of
 * your body(the software) Its is the modified version of my singly linked
 * list suggested by Ben from comp.lang.c . I was using one struct to do
 * all the operations but Ben added a 2nd struct to make things easier and
 * efficient.
 *
 * I was always using the strategy of searching through the list to find the
 *  end and then addd the value there. That way list_add() was O(n). Now I
 * am keeping track of tail and always use  tail to add to the linked list, so
 * the addition is always O(1), only at the cost of one assignment.
 *
 *
 * VERISON 0.5
 *
 */

// Example in main()

/*    struct queue*  mt = NULL;

    mt = queue_new();
    queue_add_element(mt, 1);
    queue_add_element(mt, 2);
    queue_add_element(mt, 3);
    queue_add_element(mt, 4);

    queue_print(mt);

    queue_remove_element(mt);
    queue_print(mt);

    queue_free(mt);   *//* always remember to free() the malloc()ed memory *//*
    free(mt);        *//* free() if list is kept separate from free()ing the structure, I think its a good design *//*
    mt = NULL;      *//* after free() always set that pointer to NULL, C will run havon on you if you try to use a dangling pointer *//*

    queue_print(mt);*/



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
    free(h);
    s->head = p;
    if( NULL == s->head )  s->tail = s->head;   /* The element tail was pointing to is free(), so we need an update */
    s->numItems--;

    return s;
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

