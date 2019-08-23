/*
 * Copyright (c) 2019 Marco Lizza (marco.lizza@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

#ifndef __CORE_TIMERPOOL_H__
#define __CORE_TIMERPOOL_H__

#include <stdbool.h>
#include <stdlib.h>

#define BUNDLE_FROM_INT(i)       ((void*)((long)(i)))
#define BUNDLE_TO_INT(b)         ((int)((long)(b)))

typedef enum _Timer_State_t {
    TIMER_STATE_FINALIZED,
    TIMER_STATE_RUNNING,
    TIMER_STATE_FROZEN
} Timer_State_t;

typedef struct _Timer_t {
    // TODO: add reference to the pool?
    // Timer_Pool_t *pool;
    // SEE: https://blog.noctua-software.com/entity-references.html

    double period;
    size_t repeats;
    void *bundle;

    double age;
    size_t loops;
    Timer_State_t state;

    struct _Timer_t *prev; // The pool is handler as a double-linked-list.
    struct _Timer_t *next;
} Timer_t;

typedef bool (*TimerPool_Callback_t)(Timer_t *timer, void *parameters);

typedef struct _Timer_Pool_t {
    Timer_t *timers;

    TimerPool_Callback_t update_callback;
    void *parameters;
} Timer_Pool_t;

extern void TimerPool_initialize(Timer_Pool_t *pool, TimerPool_Callback_t update_callback, void *parameters);
extern void TimerPool_terminate(Timer_Pool_t *pool);
extern Timer_t *TimerPool_allocate(Timer_Pool_t *pool, double period, size_t repeats, void *bundle);
extern bool TimerPool_update(Timer_Pool_t *pool, double delta_time);
extern void TimerPool_gc(Timer_Pool_t *pool);

extern void TimerPool_release(Timer_t *timer);
extern void TimerPool_reset(Timer_t *timer);
extern void TimerPool_cancel(Timer_t *timer);

#endif  /* __CORE_TIMERPOOL_H__ */