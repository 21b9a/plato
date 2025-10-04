#ifndef PLATO_THREAD_H
#define PLATO_THREAD_H

#include <stdlib.h>
#include <time.h>

#if defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
        #define __UNDEF_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #ifdef __UNDEF_LEAN_AND_MEAN
        #undef WIN32_LEAN_AND_MEAN
        #undef __UNDEF_LEAN_AND_MEAN
  #endif
  #include <process.h>
  #include <sys/timeb.h>
#else
    #undef _FEATURES_H
    #if !defined(_GNU_SOURCE)
        #define _GNU_SOURCE
    #endif
    #if !defined(_POSIX_C_SOURCE) || ((_POSIX_C_SOURCE - 0) < 199309L)
        #undef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 199309L
    #endif
    #if !defined(_XOPEN_SOURCE) || ((_XOPEN_SOURCE - 0) < 500)
        #undef _XOPEN_SOURCE
        #define _XOPEN_SOURCE 500
    #endif
    #define _XPG6
    #include <pthread.h>
    #include <signal.h>
    #include <sched.h>
    #include <unistd.h>
    #include <sys/time.h>
    #include <errno.h>
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  #define PL_THREAD_NORETURN _Noreturn
#elif defined(__GNUC__)
  #define PL_THREAD_NORETURN __attribute__((__noreturn__))
#else
  #define PL_THREAD_NORETURN
#endif

#ifndef TIME_UTC
    #define TIME_UTC 1
    #define _PL_EMULATE_TIMESPEC_GET_

    #if defined(_WIN32)
        struct _pl_thread_timespec {
            time_t tv_sec;
            long   tv_nsec;
        };
        #define timespec _pl_thread_timespec
    #endif
    int _pl_thread_timespec_get(struct timespec *ts, int base);
    #define timespec_get _pl_thread_timespec_get
#endif

#if !(defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201102L)) && !defined(_Thread_local)
    #if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__SUNPRO_CC) || defined(__IBMCPP__)
        #define _Thread_local __thread
    #else
        #define _Thread_local __declspec(thread)
    #endif
#elif defined(__GNUC__) && defined(__GNUC_MINOR__) && (((__GNUC__ << 8) | __GNUC_MINOR__) < ((4 << 8) | 9))
    #define _Thread_local __thread
#endif

#if defined(_WIN32)
    #define TSS_DTOR_ITERATIONS (4)
#else
    #define TSS_DTOR_ITERATIONS PTHREAD_DESTRUCTOR_ITERATIONS
#endif

#define PL_THREAD_ERROR   0
#define PL_THREAD_SUCCESS 1
#define PL_THREAD_TIMEOUT 2
#define PL_THREAD_BUSY    3
#define PL_THREAD_NOMEM   4

#define PL_MTX_PLAIN     0
#define PL_MTX_TIMED     1
#define PL_MTX_RECURSIVE 2

#if defined(_WIN32)
    typedef struct pl_mtx_s pl_mtx_t;
#else
    typedef pthread_mutex_t mtx_t;
#endif

int pl_mtx_init(pl_mtx_t *mtx, int type);
void pl_mtx_destroy(pl_mtx_t *mtx);
int pl_mtx_lock(pl_mtx_t *mtx);
int pl_mtx_timedlock(pl_mtx_t *mtx, const struct timespec *ts);
int pl_mtx_trylock(pl_mtx_t *mtx);
int pl_mtx_unlock(pl_mtx_t *mtx);

#if defined(_WIN32)
    typedef struct pl_cnd_s pl_cnd_t;
#else
    typedef pthread_cond_t pl_cnd_t;
#endif

int pl_cnd_init(pl_cnd_t *cond);
void pl_cnd_destroy(pl_cnd_t *cond);
int pl_cnd_signal(pl_cnd_t *cond);
int pl_cnd_broadcast(pl_cnd_t *cond);
int pl_cnd_wait(pl_cnd_t *cond, pl_mtx_t *mtx);
int pl_cnd_timedwait(pl_cnd_t *cond, pl_mtx_t *mtx, const struct timespec *ts);

#if defined(_WIN32)
    typedef HANDLE pl_thread_t;
#else
    typedef pthread_t pl_thread_t;
#endif

typedef int (*pl_thread_start_t)(void *arg);

int pl_thread_create(pl_thread_t *thr, pl_thread_start_t func, void *arg);
pl_thread_t pl_thread_current(void);
int pl_thread_detach(pl_thread_t thr);
int pl_thread_equal(pl_thread_t thr0, pl_thread_t thr1);
PL_THREAD_NORETURN void pl_thread_exit(int res);
int pl_thread_join(pl_thread_t thr, int *res);
int pl_thread_sleep(const struct timespec *duration, struct timespec *remaining);
void pl_thread_yield(void);

#if defined(_WIN32)
    typedef DWORD pl_tss_t;
#else
    typedef pthread_key_t pl_tss_t;
#endif

typedef void (*pl_tss_dtor_t)(void *val);

int pl_tss_create(pl_tss_t *key, pl_tss_dtor_t dtor);
void pl_tss_delete(pl_tss_t key);
void *pl_tss_get(pl_tss_t key);
int pl_tss_set(pl_tss_t key, void *val);

#if defined(_WIN32)
    typedef struct pl_once_flag_s pl_once_flag_t;
    #define PL_ONCE_FLAG_INIT {0,}
#else
    #define pl_once_flag_t pthread_once_t
    #define PL_ONCE_FLAG_INIT PTHREAD_ONCE_INIT
#endif

#if defined(_WIN32)
    void pl_call_once(pl_once_flag_t *flag, void (*func)(void));
#else
    #define pl_call_once(flag,func) pthread_once(flag,func)
#endif

#ifdef PLATO_THREAD_IMPLEMENTATION

#if defined(_WIN32)
    typedef struct pl_mtx_s {
        union {
            CRITICAL_SECTION cs;
            HANDLE mut;
        } mHandle;
        int mAlreadyLocked;
        int mRecursive;
        int mTimed;
    } pl_mtx_t;

    typedef struct pl_cnd_s {
        HANDLE mEvents[2];
        unsigned int mWaitersCount;
        CRITICAL_SECTION mWaitersCountLock;
    } pl_cnd_t;

    typedef struct pl_once_flag_s {
        LONG volatile status;
        CRITICAL_SECTION lock;
    } pl_once_flag_t;
#endif

int pl_mtx_init(pl_mtx_t *mtx, int type) {
#if defined(_WIN32)
    mtx->mAlreadyLocked = 0;
    mtx->mRecursive = type & PL_MTX_RECURSIVE;
    mtx->mTimed = type & PL_MTX_TIMED;
    if(!mtx->mTimed) {
        InitializeCriticalSection(&(mtx->mHandle.cs));
    }
    else {
        mtx->mHandle.mut = CreateMutex(NULL, 0, NULL);
        if(mtx->mHandle.mut == NULL) return PL_THREAD_ERROR;
    }
    return PL_THREAD_SUCCESS;
#else
    int ret;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if(type & PL_MTX_RECURSIVE) pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    ret = pthread_mutex_init(mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    return ret == 0 ? PL_THREAD_SUCCESS : PL_THREAD_ERROR;
#endif
}

void pl_mtx_destroy(pl_mtx_t *mtx) {
#if defined(_WIN32)
    if(!mtx->mTimed) DeleteCriticalSection(&(mtx->mHandle.cs));
    else CloseHandle(mtx->mHandle.mut);
#else
    pthread_mutex_destroy(mtx);
#endif
}

int pl_mtx_lock(pl_mtx_t *mtx) {
#if defined(_WIN32)
    if(!mtx->mTimed) EnterCriticalSection(&(mtx->mHandle.cs));
    else {
        switch(WaitForSingleObject(mtx->mHandle.mut, INFINITE)) {
            case WAIT_OBJECT_0:
                break;
            case WAIT_ABANDONED:
            default:
                return PL_THREAD_ERROR;
        }
    }

    if(!mtx->mRecursive) {
        while(mtx->mAlreadyLocked) Sleep(1);
        mtx->mAlreadyLocked = TRUE;
    }
    return PL_THREAD_SUCCESS;
#else
    return pthread_mutex_lock(mtx) == 0 ? PL_THREAD_SUCCESS : PL_THREAD_ERROR;
#endif
}

int pl_mtx_timedlock(pl_mtx_t *mtx, const struct timespec *ts) {
#if defined(_WIN32)
    struct timespec current_ts;
    DWORD timeoutMs;

    if(!mtx->mTimed) return PL_THREAD_ERROR;

    timespec_get(&current_ts, TIME_UTC);

    if((current_ts.tv_sec > ts->tv_sec) || 
    ((current_ts.tv_sec == ts->tv_sec) && (current_ts.tv_nsec >= ts->tv_nsec))) {
        timeoutMs = 0;
    }
    else {
        timeoutMs  = (DWORD)(ts->tv_sec  - current_ts.tv_sec)  * 1000;
        timeoutMs += (ts->tv_nsec - current_ts.tv_nsec) / 1000000;
        timeoutMs += 1;
    }

    switch(WaitForSingleObject(mtx->mHandle.mut, timeoutMs)) {
        case WAIT_OBJECT_0:
            break;
        case WAIT_TIMEOUT:
            return PL_THREAD_TIMEOUT;
        case WAIT_ABANDONED:
        default:
            return PL_THREAD_ERROR;
    }

    if(!mtx->mRecursive) {
        while(mtx->mAlreadyLocked) Sleep(1);
        mtx->mAlreadyLocked = 1;
    }

    return PL_THREAD_SUCCESS;
#elif defined(_POSIX_TIMEOUTS) && (_POSIX_TIMEOUTS >= 200112L) && defined(_POSIX_THREADS) && (_POSIX_THREADS >= 200112L)
    switch(pthread_mutex_timedlock(mtx, ts)) {
        case 0:
            return PL_THREAD_SUCCESS;
        case ETIMEDOUT:
            return PL_THREAD_TIMEOUT;
        default:
            return PL_THREAD_ERROR;
    }
#else
    int rc;
    struct timespec cur, dur;

    while((rc = pthread_mutex_trylock (mtx)) == EBUSY) {
        timespec_get(&cur, TIME_UTC);

        if((cur.tv_sec > ts->tv_sec) || 
        ((cur.tv_sec == ts->tv_sec) && (cur.tv_nsec >= ts->tv_nsec))) {
            break;
        }

        dur.tv_sec = ts->tv_sec - cur.tv_sec;
        dur.tv_nsec = ts->tv_nsec - cur.tv_nsec;
        if(dur.tv_nsec < 0) {
            dur.tv_sec--;
            dur.tv_nsec += 1000000000;
        }

        if((dur.tv_sec != 0) || (dur.tv_nsec > 5000000)) {
            dur.tv_sec = 0;
            dur.tv_nsec = 5000000;
        }

        nanosleep(&dur, NULL);
    }

    switch(rc) {
        case 0:
            return PL_THREAD_SUCCESS;
        case ETIMEDOUT:
        case EBUSY:
            return PL_THREAD_TIMEOUT;
        default:
            return PL_THREAD_ERROR;
    }
#endif
}

int pl_mtx_trylock(pl_mtx_t *mtx) {
#if defined(_WIN32)
    int ret;

    if(!mtx->mTimed) {
        ret = TryEnterCriticalSection(&(mtx->mHandle.cs)) ? PL_THREAD_SUCCESS : PL_THREAD_BUSY;
    }
    else {
        ret = (WaitForSingleObject(mtx->mHandle.mut, 0) == WAIT_OBJECT_0) ? PL_THREAD_SUCCESS : PL_THREAD_BUSY;
    }

    if((!mtx->mRecursive) && (ret == PL_THREAD_SUCCESS)) {
        if (mtx->mAlreadyLocked) {
            LeaveCriticalSection(&(mtx->mHandle.cs));
            ret = PL_THREAD_BUSY;
        }
        else mtx->mAlreadyLocked = 1;
    }
    return ret;
#else
    return (pthread_mutex_trylock(mtx) == 0) ? PL_THREAD_SUCCESS : PL_THREAD_BUSY;
#endif
}

int pl_mtx_unlock(pl_mtx_t *mtx) {
#if defined(_WIN32)
    mtx->mAlreadyLocked = 0;
    if(!mtx->mTimed) LeaveCriticalSection(&(mtx->mHandle.cs));
    else {
        if(!ReleaseMutex(mtx->mHandle.mut)) return PL_THREAD_ERROR;
    }
    return PL_THREAD_SUCCESS;
#else
    return pthread_mutex_unlock(mtx) == 0 ? PL_THREAD_SUCCESS : PL_THREAD_ERROR;;
#endif
}

#if defined(_WIN32)
    #define _CONDITION_EVENT_ONE 0
    #define _CONDITION_EVENT_ALL 1
#endif

int pl_cnd_init(pl_cnd_t *cond) {
#if defined(_WIN32)
    cond->mWaitersCount = 0;
    InitializeCriticalSection(&cond->mWaitersCountLock);
    cond->mEvents[_CONDITION_EVENT_ONE] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(cond->mEvents[_CONDITION_EVENT_ONE] == NULL) {
        cond->mEvents[_CONDITION_EVENT_ALL] = NULL;
        return PL_THREAD_ERROR;
    }
    cond->mEvents[_CONDITION_EVENT_ALL] = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(cond->mEvents[_CONDITION_EVENT_ALL] == NULL) {
        CloseHandle(cond->mEvents[_CONDITION_EVENT_ONE]);
        cond->mEvents[_CONDITION_EVENT_ONE] = NULL;
        return PL_THREAD_ERROR;
    }
    return PL_THREAD_SUCCESS;
#else
    return pthread_cond_init(cond, NULL) == 0 ? PL_THREAD_SUCCESS : PL_THREAD_ERROR;
#endif
}

void pl_cnd_destroy(pl_cnd_t *cond) {
#if defined(_WIN32)
    if(cond->mEvents[_CONDITION_EVENT_ONE] != NULL) {
        CloseHandle(cond->mEvents[_CONDITION_EVENT_ONE]);
    }
    if(cond->mEvents[_CONDITION_EVENT_ALL] != NULL) {
        CloseHandle(cond->mEvents[_CONDITION_EVENT_ALL]);
    }
    DeleteCriticalSection(&cond->mWaitersCountLock);
#else
    pthread_cond_destroy(cond);
#endif
}

int pl_cnd_signal(pl_cnd_t *cond) {
#if defined(_WIN32)
    int haveWaiters;

    EnterCriticalSection(&cond->mWaitersCountLock);
    haveWaiters = (cond->mWaitersCount > 0);
    LeaveCriticalSection(&cond->mWaitersCountLock);

    if(haveWaiters) {
        if(SetEvent(cond->mEvents[_CONDITION_EVENT_ONE]) == 0) return PL_THREAD_ERROR;
    }

    return PL_THREAD_SUCCESS;
#else
    return pthread_cond_signal(cond) == 0 ? PL_THREAD_SUCCESS : PL_THREAD_ERROR;
#endif
}

int pl_cnd_broadcast(pl_cnd_t *cond) {
#if defined(_WIN32)
    int haveWaiters;

    EnterCriticalSection(&cond->mWaitersCountLock);
    haveWaiters = (cond->mWaitersCount > 0);
    LeaveCriticalSection(&cond->mWaitersCountLock);

    if(haveWaiters) {
        if(SetEvent(cond->mEvents[_CONDITION_EVENT_ALL]) == 0) return PL_THREAD_ERROR;
    }

    return PL_THREAD_SUCCESS;
#else
    return pthread_cond_broadcast(cond) == 0 ? PL_THREAD_SUCCESS : PL_THREAD_ERROR;
#endif
}

#if defined(_WIN32)
static int _pl_cnd_timedwait_win32(pl_cnd_t *cond, pl_mtx_t *mtx, DWORD timeout) {
    DWORD result;
    int lastWaiter;

    EnterCriticalSection(&cond->mWaitersCountLock);
    ++cond->mWaitersCount;
    LeaveCriticalSection(&cond->mWaitersCountLock);

    pl_mtx_unlock(mtx);

    result = WaitForMultipleObjects(2, cond->mEvents, FALSE, timeout);
    if(result == WAIT_TIMEOUT) {
        pl_mtx_lock(mtx);
        return PL_THREAD_TIMEOUT;
    }
    else if(result == WAIT_FAILED) {
        pl_mtx_lock(mtx);
        return PL_THREAD_ERROR;
    }

    EnterCriticalSection(&cond->mWaitersCountLock);
    --cond->mWaitersCount;
    lastWaiter = (result == (WAIT_OBJECT_0 + _CONDITION_EVENT_ALL)) && (cond->mWaitersCount == 0);
    LeaveCriticalSection(&cond->mWaitersCountLock);

    if(lastWaiter) {
        if(ResetEvent(cond->mEvents[_CONDITION_EVENT_ALL]) == 0) {
            pl_mtx_lock(mtx);
            return PL_THREAD_ERROR;
        }
    }

    pl_mtx_lock(mtx);

    return PL_THREAD_SUCCESS;
}
#endif

int pl_cnd_wait(pl_cnd_t *cond, pl_mtx_t *mtx) {
#if defined(_WIN32)
    return _pl_cnd_timedwait_win32(cond, mtx, INFINITE);
#else
    return pthread_cond_wait(cond, mtx) == 0 ? PL_THREAD_SUCCESS : PL_THREAD_ERROR;
#endif
}

int pl_cnd_timedwait(pl_cnd_t *cond, pl_mtx_t *mtx, const struct timespec *ts) {
#if defined(_WIN32)
    struct timespec now;
    if(timespec_get(&now, TIME_UTC) == TIME_UTC) {
        unsigned long long nowInMilliseconds = now.tv_sec * 1000 + now.tv_nsec / 1000000;
        unsigned long long tsInMilliseconds  = ts->tv_sec * 1000 + ts->tv_nsec / 1000000;
        DWORD delta = (tsInMilliseconds > nowInMilliseconds) ?
        (DWORD)(tsInMilliseconds - nowInMilliseconds) : 0;
        return _pl_cnd_timedwait_win32(cond, mtx, delta);
    }
    else return PL_THREAD_ERROR;
#else
    int ret;
    ret = pthread_cond_timedwait(cond, mtx, ts);
    if(ret == ETIMEDOUT) return PL_THREAD_TIMEOUT;
    return ret == 0 ? PL_THREAD_SUCCESS : PL_THREAD_ERROR;
#endif
}

#if defined(_WIN32)
    struct pl_tss_data_s {
        void* value;
        pl_tss_t key;
        struct pl_tss_data_s* next;
    };

    static pl_tss_dtor_t _pl_tss_dtors[1088] = { NULL, };
    static _Thread_local struct pl_tss_data_s* _pl_tss_head = NULL;
    static _Thread_local struct pl_tss_data_s* _pl_tss_tail = NULL;

    static void _pl_tss_cleanup(void);
    static void _pl_tss_cleanup(void) {
        struct pl_tss_data_s* data;
        int iteration;
        unsigned int again = 1;
        void* value;

        for(iteration = 0; iteration < TSS_DTOR_ITERATIONS && again > 0; iteration++) {
            again = 0;
            for(data = _pl_tss_head; data != NULL; data = data->next) {
                if(data->value != NULL) {
                    value = data->value;
                    data->value = NULL;

                    if(_pl_tss_dtors[data->key] != NULL) {
                        again = 1;
                        _pl_tss_dtors[data->key](value);
                    }
                }
            }
        }

        while(_pl_tss_head != NULL) {
            data = _pl_tss_head->next;
            free(_pl_tss_head);
            _pl_tss_head = data;
        }
        _pl_tss_head = NULL;
        _pl_tss_tail = NULL;
    }

    static void NTAPI _pl_tss_callback(PVOID h, DWORD dwReason, PVOID pv) {
        (void)h;
        (void)pv;

        if(_pl_tss_head != NULL && (dwReason == DLL_THREAD_DETACH || dwReason == DLL_PROCESS_DETACH)) {
            _pl_tss_cleanup();
        }
    }

    #if defined(_MSC_VER)
        #ifdef _M_X64
            #pragma const_seg(".CRT$XLB")
        #else
            #pragma data_seg(".CRT$XLB")
        #endif
        PIMAGE_TLS_CALLBACK p_thread_callback = _pl_tss_callback;
        #ifdef _M_X64
            #pragma data_seg()
        #else
            #pragma const_seg()
        #endif
        #else
        PIMAGE_TLS_CALLBACK p_thread_callback __attribute__((section(".CRT$XLB"))) = _pl_tss_callback;
    #endif
#endif

typedef struct {
  pl_thread_start_t mFunction;
  void *mArg;
} _pl_thread_start_info_s;

#if defined(_WIN32)
    static DWORD WINAPI _pl_thread_wrapper_function(LPVOID aArg)
#else
    static void *_pl_thread_wrapper_function(void *aArg)
#endif
{
    pl_thread_start_t fun;
    void *arg;
    int  res;

    _pl_thread_start_info_s *ti = (_pl_thread_start_info_s*)aArg;
    fun = ti->mFunction;
    arg = ti->mArg;

    free((void *)ti);

    res = fun(arg);

    #if defined(_WIN32)
        if(_pl_tss_head != NULL) _pl_tss_cleanup();
        return (DWORD)res;
    #else
        return (void*)(intptr_t)res;
    #endif
}

int pl_thread_create(pl_thread_t *thr, pl_thread_start_t func, void *arg)
{
  _pl_thread_start_info_s* ti = (_pl_thread_start_info_s*)malloc(sizeof(_pl_thread_start_info_s));
  if(ti == NULL) return PL_THREAD_NOMEM;
  ti->mFunction = func;
  ti->mArg = arg;

#if defined(_WIN32)
    *thr = CreateThread(NULL, 0, _pl_thread_wrapper_function, (LPVOID) ti, 0, NULL);
#else
    if(pthread_create(thr, NULL, _pl_thread_wrapper_function, (void *)ti) != 0) *thr = 0;
#endif

  if(!*thr) {
    free(ti);
    return PL_THREAD_ERROR;
  }
  return PL_THREAD_SUCCESS;
}

pl_thread_t pl_thread_current(void) {
#if defined(_WIN32)
    return GetCurrentThread();
#else
    return pthread_self();
#endif
}

int pl_thread_detach(pl_thread_t thr) {
#if defined(_WIN32)
  return CloseHandle(thr) != 0 ? PL_THREAD_SUCCESS : PL_THREAD_ERROR;
#else
  return pthread_detach(thr) == 0 ? PL_THREAD_SUCCESS : PL_THREAD_ERROR;
#endif
}

int pl_thread_equal(pl_thread_t thr0, pl_thread_t thr1) {
#if defined(_WIN32)
    return GetThreadId(thr0) == GetThreadId(thr1);
#else
    return pthread_equal(thr0, thr1);
#endif
}

void pl_thread_exit(int res) {
#if defined(_WIN32)
    if(_pl_tss_head != NULL) _pl_tss_cleanup();
    ExitThread((DWORD)res);
#else
    pthread_exit((void*)(intptr_t)res);
#endif
}

int pl_thread_join(pl_thread_t thr, int *res) {
#if defined(_WIN32)
    DWORD dwRes;
    if(WaitForSingleObject(thr, INFINITE) == WAIT_FAILED) return PL_THREAD_ERROR;
    if(res != NULL) {
        if(GetExitCodeThread(thr, &dwRes) != 0) *res = (int) dwRes;
        else return PL_THREAD_ERROR;
    }
    CloseHandle(thr);
#else
    void *pres;
    if(pthread_join(thr, &pres) != 0) return PL_THREAD_ERROR;
    if(res != NULL) *res = (int)(intptr_t)pres;
#endif
    return PL_THREAD_SUCCESS;
}

int pl_thread_sleep(const struct timespec *duration, struct timespec *remaining) {
#if defined(_WIN32)
    struct timespec start;
    DWORD t;

    timespec_get(&start, TIME_UTC);

    t = SleepEx((DWORD)(duration->tv_sec * 1000 +
                duration->tv_nsec / 1000000 +
                (((duration->tv_nsec % 1000000) == 0) ? 0 : 1)),
                TRUE);

    if(t == 0) return 0;
    else {
        if(remaining != NULL) {
            timespec_get(remaining, TIME_UTC);
            remaining->tv_sec -= start.tv_sec;
            remaining->tv_nsec -= start.tv_nsec;

            if(remaining->tv_nsec < 0) {
                remaining->tv_nsec += 1000000000;
                remaining->tv_sec -= 1;
            }
        }
        return (t == WAIT_IO_COMPLETION) ? -1 : -2;
    }
#else
    int res = nanosleep(duration, remaining);
    if(res == 0) return 0;
    else if (errno == EINTR) return -1;
    else return -2;
#endif
}

void pl_thrd_yield(void) {
#if defined(_WIN32)
    Sleep(0);
#else
    sched_yield();
#endif
}

int pl_tss_create(pl_tss_t *key, pl_tss_dtor_t dtor) {
#if defined(_WIN32)
    *key = TlsAlloc();
  if(*key == TLS_OUT_OF_INDEXES) return PL_THREAD_ERROR;
  _pl_tss_dtors[*key] = dtor;
#else
    if(pthread_key_create(key, dtor) != 0) return PL_THREAD_ERROR;
#endif
    return PL_THREAD_SUCCESS;
}

void pl_tss_delete(pl_tss_t key) {
#if defined(_WIN32)
    struct pl_tss_data_s *data = (struct pl_tss_data_s*)TlsGetValue(key);
    struct pl_tss_data_s *prev = NULL;
    if(data != NULL) {
        if(data == _pl_tss_head) _pl_tss_head = data->next;
        else {
            prev = _pl_tss_head;
            if(prev != NULL) {
                while(prev->next != data) prev = prev->next;
            }
        }

        if(data == _pl_tss_tail) _pl_tss_tail = prev;
        free (data);
    }

    _pl_tss_dtors[key] = NULL;
    TlsFree(key);
#else
    pthread_key_delete(key);
#endif
}

void *pl_tss_get(pl_tss_t key) {
#if defined(_WIN32)
    struct pl_tss_data_s *data = (struct pl_tss_data_s*)TlsGetValue(key);
    if (data == NULL) return NULL;
    return data->value;
#else
    return pthread_getspecific(key);
#endif
}

int pl_tss_set(pl_tss_t key, void *val) {
#if defined(_WIN32)
    struct pl_tss_data_s *data = (struct pl_tss_data_s*)TlsGetValue(key);
    if(data == NULL) {
        data = (struct pl_tss_data_s*)malloc(sizeof(struct pl_tss_data_s));
        if(data == NULL) return PL_THREAD_ERROR;

        data->value = NULL;
        data->key = key;
        data->next = NULL;

        if (_pl_tss_tail != NULL) _pl_tss_tail->next = data;
        else _pl_tss_tail = data;

        if(_pl_tss_head == NULL) _pl_tss_head = data;

        if(!TlsSetValue(key, data)) {
            free (data);
            return PL_THREAD_ERROR;
        }
    }
    data->value = val;
#else
    if(pthread_setspecific(key, val) != 0) return PL_THREAD_ERROR;
#endif
    return PL_THREAD_SUCCESS;
}

#if defined(_PL_EMULATE_TIMESPEC_GET_)
int _pl_thread_timespec_get(struct timespec *ts, int base) {
#if defined(_WIN32)
    struct _timeb tb;
#elif !defined(CLOCK_REALTIME)
    struct timeval tv;
#endif

  if(base != TIME_UTC) return 0;

#if defined(_WIN32)
    _ftime_s(&tb);
    ts->tv_sec = (time_t)tb.time;
    ts->tv_nsec = 1000000L * (long)tb.millitm;
#elif defined(CLOCK_REALTIME)
    base = (clock_gettime(CLOCK_REALTIME, ts) == 0) ? base : 0;
#else
    gettimeofday(&tv, NULL);
    ts->tv_sec = (time_t)tv.tv_sec;
    ts->tv_nsec = 1000L * (long)tv.tv_usec;
#endif

    return base;
}
#endif /* _PL_EMULATE_TIMESPEC_GET_ */

#if defined(_WIN32)
void pl_call_once(pl_once_flag_t *flag, void (*func)(void)) {
    while (flag->status < 3) {
        switch (flag->status) {
        case 0:
            if(InterlockedCompareExchange(&(flag->status), 1, 0) == 0) {
                InitializeCriticalSection(&(flag->lock));
                EnterCriticalSection(&(flag->lock));
                flag->status = 2;
                func();
                flag->status = 3;
                LeaveCriticalSection(&(flag->lock));
                return;
            }
            break;
        case 1:
            break;
        case 2:
            EnterCriticalSection(&(flag->lock));
            LeaveCriticalSection(&(flag->lock));
            break;
        }
    }
}
#endif

#endif // PLATO_THREAD_IMPLEMENTATION
#endif // PLATO_THREAD_H