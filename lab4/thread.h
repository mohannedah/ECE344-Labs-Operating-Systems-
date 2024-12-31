#ifndef THREAD_H
#define THREAD_H
#include <ucontext.h>

struct Thread
{
    inline Thread(int id, ucontext_t *thread_context)
    {
        this->id = id;
        this->thread_context = thread_context;
    }
    int id;
    ucontext_t *thread_context;
    int blocking = -1;
    bool isBlocked;
    bool terminated = false;
    int status = 1;
};

#endif
