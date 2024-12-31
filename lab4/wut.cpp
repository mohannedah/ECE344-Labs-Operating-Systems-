#include <iostream>
#include <sys/wait.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include <sys/mman.h>   // mmap, munmap
#include <sys/signal.h> // SIGSTKSZ
#include <ucontext.h>
#include "wut.h"
#include "dynamic_array.h"
#include "my_queue.h"
#include "thread.h"
using namespace std;
DynamicArray<Thread *> threads;
MyQueue<Thread *> thread_queue(nullptr);

int currently_executing_thread = 0;

char *new_stack(void)
{

    char *stack = (char *)mmap(
        NULL,
        SIGSTKSZ,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANONYMOUS | MAP_PRIVATE,
        -1,
        0);
    if (stack == MAP_FAILED)
    {
        cerr << "Stack allocation failed!" << endl;
        exit(errno);
    }
    return stack;
}

void run_wrapper(void (*run)(void))
{
    run();
    wut_exit(0);
}

Thread *init_process(void (*run)(void))
{
    int id = threads.size;
    Thread *thread = new Thread(id, (ucontext_t *)malloc(sizeof(ucontext_t)));
    getcontext(thread->thread_context);
    thread->thread_context->uc_stack.ss_size = SIGSTKSZ;
    thread->thread_context->uc_stack.ss_sp = new_stack();
    makecontext(thread->thread_context, (void (*)(void))run_wrapper, 1, run);
    return thread;
};

void wut_init()
{
    Thread *main_thread = new Thread(0, (ucontext_t *)malloc(sizeof(ucontext_t)));
    threads.push_array(main_thread);
};

int wut_id()
{
    return currently_executing_thread;
}

int wut_create(void (*run)(void))
{
    Thread *newly_created_thread = init_process(run);
    threads.push_array(newly_created_thread);
    thread_queue.add_queue(newly_created_thread);
    return newly_created_thread->id;
}

void free_thread(Thread *thread)
{
    free(thread->thread_context->uc_stack.ss_sp);
    free(thread->thread_context);
}

int wut_cancel(int idx)
{
    Thread *thread = threads.get(idx);
    thread_queue.pop_queue(idx);
    if (!thread->terminated)
    {
        thread->status = 128;
        thread->terminated = 1;
        free_thread(thread);
    }
    return 0;
}

int process_threads()
{
    Thread *top_thread = thread_queue.pop_front_queue();

    if (top_thread == nullptr)
        return -1;

    int temp = currently_executing_thread;
    currently_executing_thread = top_thread->id;

    swapcontext(threads.get(temp)->thread_context, top_thread->thread_context);
    return 1;
}

int wut_join(int idx)
{
    if (idx < 0 || idx >= threads.size)
        return -1;
    Thread *current_thread = threads.get(currently_executing_thread);
    Thread *blocking_thread = threads.get(idx);
    if ((current_thread->id == blocking_thread->id) || (blocking_thread->blocking != -1) || (blocking_thread->isBlocked))
        return -1;

    if (blocking_thread->terminated)
    {
        return blocking_thread->status;
    }
    current_thread->isBlocked = 1;
    blocking_thread->blocking = current_thread->id;
    process_threads();
    return blocking_thread->status;
};

void wut_exit(int status)
{
    Thread *current_thread = threads.get(currently_executing_thread);
    current_thread->status = status;
    if (current_thread->blocking != -1)
    {
        Thread *blocked_thread = threads.get(current_thread->blocking);
        if (!blocked_thread->terminated)
        {
            blocked_thread->isBlocked = 0;
            thread_queue.add_queue(blocked_thread);
        }
    }
    current_thread->blocking = -1;
    process_threads();
}

int wut_yield()
{
    Thread *current_thread = threads.get(currently_executing_thread);
    thread_queue.add_queue(current_thread);
    int status = process_threads();
    return status;
}

void thread_two_run()
{
    int status = wut_join(2);
    cout << "Thread three finished with status" << " " << status << endl;
    cout << "Thread two finished executing" << endl;
}

void thread_three_run()
{
    cout << "Thread three finished executing" << endl;
}

int main()
{
    wut_init();
    wut_create(&thread_two_run);
    wut_create(&thread_three_run);
    wut_yield();
    cout
        << "Returned from the yield 1" << endl;
    wut_yield();
    cout
        << "Returned from the yield 2" << endl;
    return 0;
}