#ifndef PLATO_TIMER_H
#define PLATO_TIMER_H

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <time.h>
#endif

typedef struct pl_timer_s {
#if defined(_WIN32)
    LARGE_INTEGER frequency;
    LARGE_INTEGER start_time;
#elif defined(__linux__) || defined(__APPLE__)
    struct timespec start_time;
#endif
} pl_timer_t;

void pl_timer_init(pl_timer_t *timer);
double pl_timer_dt(pl_timer_t *timer);

#if defined(PLATO_IMPLEMENTATION) || defined(PLATO_TIMER_IMPLEMENTATION)

void pl_timer_init(pl_timer_t *timer) {
#if defined(_WIN32)
    QueryPerformanceFrequency(&timer->frequency);
    QueryPerformanceCounter(&timer->start_time);
#elif defined(__linux__) || defined(__APPLE__)
    clock_gettime(CLOCK_MONOTONIC, &timer->start_time);
#endif
}

double pl_timer_dt(pl_timer_t *timer) {
#if defined(_WIN32)
    LARGE_INTEGER end_time;
    QueryPerformanceCounter(&end_time);
    double dt = (double)(end_time.QuadPart - timer->start_time.QuadPart);
    dt /= timer->frequency.QuadPart;
    timer->start_time = end_time;
    return dt;
#elif defined(__linux__) || defined(__APPLE__)
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double dt = (double)(end_time.tv_sec - timer->start_time.tv_sec) + 
                        (double)(end_time.tv_nsec - timer->start_time.tv_nsec) / 1e9;
    timer->start_time = end_time;
    return dt;
#endif
}

#endif // PLATO_TIMER_IMPLEMENTATION
#endif // PLATO_TIMER_H