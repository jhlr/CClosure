#ifndef _CLOSURE_H_
#define _CLOSURE_H_

#include <stdlib.h>

//#define CLOSURE_PARALLEL
/* NOT THREAD SAFE!
** TODO implement the code for CLOSURE_PARALLEL
** Probably a different code for each thread library
*/

#define CLOSURE_LABEL1(a) closure_temp_ ## a
#define CLOSURE_LABEL0(a) CLOSURE_LABEL1(a)
#define CLOSURE_LABEL CLOSURE_LABEL0(__LINE__)

#define $(closure) ({\
    if(0){\
        CLOSURE_LABEL:\
        do closure while(0);\
        closure_return(NULL);\
    };\
    &&CLOSURE_LABEL;\
})

#define closure_call(c, arg) ({closure_add(&&CLOSURE_LABEL, (void*) arg); goto *c; CLOSURE_LABEL: closure_get_return_value();})
#define closure_return(value) ({_closure_return(value); goto *closure_remove();})

typedef void* Closure;

typedef struct closure_stack_t {
    int size;
    int counter;
    Closure* array;
    void** args;
} closure_stack_t;

int closure_num_stack = 0;
closure_stack_t* closure_stack = NULL;

int closure_get_tid(){
#ifndef CLOSURE_PARALLEL
    return 0;
#else
#error
#endif
}

int closure_get_num_threads(){
#ifndef CLOSURE_PARALLEL
    return 1;
#else
#error
#endif
}

int closure_create_stack(){
    if(closure_stack == NULL){
        closure_num_stack = closure_get_num_threads();
        closure_stack = (closure_stack_t*)calloc(closure_num_stack, sizeof(closure_stack_t));
        return 0;
    } else {
        closure_num_stack++;
        closure_stack = (closure_stack_t*)realloc(closure_stack, sizeof(closure_stack_t)*(closure_num_stack));
        return closure_num_stack-1;
    }
}


void closure_add(Closure c, void* arg){
    if(closure_stack == NULL){
        closure_create_stack();
    }
    const int tid = closure_get_tid();
    closure_stack_t* stack = &closure_stack[tid];
    if(stack->array == NULL){
        stack->size = 32;
        stack->array = (void**)calloc(stack->size, sizeof(void*));
        stack->args = (void**)calloc(stack->size, sizeof(void*));
    }
    if(stack->counter == stack->size){
        stack->array = (void**)realloc(stack->array, sizeof(void*)*(stack->size + 32));
        stack->args = (void**)realloc(stack->args, sizeof(void*)*(stack->size + 32));
        stack->size += 32;
    }
    stack->counter++;
    stack->array[stack->counter] = c;
    stack->args[stack->counter] = arg;
}

Closure closure_remove(){
    const int tid = closure_get_tid();
    closure_stack_t* stack = &closure_stack[tid];
    if(stack->counter == -1){
        return NULL;
    }
    Closure result = stack->array[stack->counter];
    stack->counter--;
    return result;

}

void* closure_argument(){
    const int tid = closure_get_tid();
    closure_stack_t* stack = &closure_stack[tid];
    return stack->args[stack->counter];
}

void* closure_get_return_value(){
    const int tid = closure_get_tid();
    closure_stack_t* stack = &closure_stack[tid];
    return stack->args[stack->counter+1];
}

void _closure_return(void* value){
    const int tid = closure_get_tid();
    closure_stack_t* stack = &closure_stack[tid];
    stack->args[stack->counter] = value;
}


#endif
