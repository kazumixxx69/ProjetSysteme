#define _POSIX_SOURCE

#include "SDL.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include "timer.h"
#include "asde_dlist.h"

#define MAX_TIMERS 100

// Return number of elapsed µsec since... a long time ago
static unsigned long get_time (void)
{
  struct timeval tv;

  gettimeofday (&tv ,NULL);

  // Only count seconds since beginning of 2016 (not jan 1st, 1970)
  tv.tv_sec -= 3600UL * 24 * 365 * 46;
  
  return tv.tv_sec * 1000000UL + tv.tv_usec;
}

#ifdef PADAWAN

DList volatile L = NULL;
int volatile id = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void demon_handler(int s){
  pthread_mutex_lock(&mutex);
  //fprintf(stderr, "pthread %lu\n", pthread_self());
  //printf("sdl_push_event (%p) appelée au temps %ld\n", glob_param, get_time());
  sdl_push_event(asde_dlist_param(L));
  if(L != NULL){
    //unsigned long prev_val = asde_dlist_data(L);
    /*while(p != NULL){
      unsigned long tmp_val = asde_dlist_data(p);
      p = asde_dlist_modify_data(p, tmp_val - prev_val);
      p = asde_dlist_next(p);
    }*/
    L = asde_dlist_delete_first(L);
    if(L != NULL){
      struct itimerval val;
      struct timeval it_value;
      it_value.tv_sec = (asde_dlist_data(L) - get_time()) / 1000000UL;
      it_value.tv_usec = (asde_dlist_data(L) - get_time()) % 1000000UL;
      struct timeval it_interval;
      it_interval.tv_sec = 1;
      it_interval.tv_usec = 0;
      val.it_value = it_value;
      val.it_interval = it_interval;
      setitimer(ITIMER_REAL, &val, NULL);
      if((it_value.tv_sec == 0) && (it_value.tv_usec < 10000UL)){
        raise(SIGALRM);
      }
    }
  }
  pthread_mutex_unlock(&mutex);
}

void* demon_thread(void* p){
  struct sigaction act;
  act.sa_handler = demon_handler;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);

  sigaction(SIGALRM, &act, NULL);

  while(1){
    sigsuspend(&act.sa_mask);
  }
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
  pthread_t tid;
  pthread_create(&tid, NULL, demon_thread, NULL);

  sleep(1);

  /*for(int i = 0; i < 10; i++){
    raise(SIGALRM);
  }*/

  return 1; // Implementation not ready
}

timer_id_t timer_set (Uint32 delay, void *param)
{

  struct itimerval val;
  struct timeval it_value;
  it_value.tv_sec = delay / 1000;
  it_value.tv_usec = (delay % 1000) * 1000;
  struct timeval it_interval;
  it_interval.tv_sec = 1;
  it_interval.tv_usec = 0;
  val.it_value = it_value;
  val.it_interval = it_interval;

  pthread_mutex_lock(&mutex);
  id++;
  DList p = L;
  int cpt = 0;
  while(p != NULL){
    cpt++;
    fprintf(stderr, "%lu\n", asde_dlist_data(p));
    p = asde_dlist_next(p);
  }
  if(cpt == MAX_TIMERS){
    raise(SIGALRM);
  }
  fprintf(stderr, "%d\n", cpt);

  p = L;

  if(L == NULL){
    L = asde_dlist_prepend(L, get_time() + val.it_value.tv_sec * 1000000UL + val.it_value.tv_usec, param, id);
    setitimer(ITIMER_REAL, &val, NULL);
  }
  else{
    cpt = 0;
    unsigned long tmp_val = asde_dlist_data(p);
    unsigned long new_val;
    int check = 0;
    while(tmp_val < (new_val = get_time() + val.it_value.tv_sec * 1000000UL + val.it_value.tv_usec)){
       cpt++;
       if(asde_dlist_next(p) == NULL){
         L = asde_dlist_insert_after(L, p, new_val, param, id);
         check = 1;
       }
       else {
         p = asde_dlist_next(p);
         tmp_val = asde_dlist_data(p);
       }
    }
    if(check != 1) L = asde_dlist_insert_before(L, p, new_val, param, id);
    if(cpt == 0){
      setitimer(ITIMER_REAL, &val, NULL);
    }
  }
  pthread_mutex_unlock(&mutex);  
  fprintf(stderr, "yo\n");

  return (timer_id_t) id;
}

int timer_cancel (timer_id_t timer_id)
{
  pthread_mutex_lock(&mutex);
  DList p = L;
  
  while((p != NULL) || ((asde_dlist_id(p)) != ((int) (timer_id)))){
    p = asde_dlist_next(p);
  }
  if (p != NULL){
    if((asde_dlist_prev(p)) == NULL){
      L = asde_dlist_delete_first(L);
      if(L != NULL){
        struct itimerval val;
        struct timeval it_value;
        it_value.tv_sec = (asde_dlist_data(L) - get_time()) / 1000000UL;
        it_value.tv_usec = (asde_dlist_data(L) - get_time()) % 1000000UL;
        struct timeval it_interval;
        it_interval.tv_sec = 1;
        it_interval.tv_usec = 0;
        val.it_value = it_value;
        val.it_interval = it_interval;
        setitimer(ITIMER_REAL, &val, NULL);
        if((it_value.tv_sec == 0) && (it_value.tv_usec < 10000UL)){
          raise(SIGALRM);
        }
      }
    }
    else {
      L = asde_dlist_delete_after(L, asde_dlist_prev(p));
    }
  }

  pthread_mutex_unlock(&mutex); 

  return 0; // failure
}

#endif

/* IMPLEMENTATION SIMPLE

#define _POSIX_SOURCE

#include "SDL.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include "timer.h"

// Return number of elapsed µsec since... a long time ago
static unsigned long get_time (void)
{
  struct timeval tv;

  gettimeofday (&tv ,NULL);

  // Only count seconds since beginning of 2016 (not jan 1st, 1970)
  tv.tv_sec -= 3600UL * 24 * 365 * 46;
  
  return tv.tv_sec * 1000000UL + tv.tv_usec;
}

#ifdef PADAWAN

void* glob_param;

void demon_handler(int s){
  //fprintf(stderr, "pthread %lu\n", pthread_self());
  printf("sdl_push_event (%p) appelée au temps %ld\n", glob_param, get_time());
  //sdl_push_event(glob_param);
}

void* demon_thread(void* p){
  struct sigaction act;
  act.sa_handler = demon_handler;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);

  sigaction(SIGALRM, &act, NULL);

  while(1){
    sigsuspend(&act.sa_mask);
  }
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
  pthread_t tid;
  pthread_create(&tid, NULL, demon_thread, NULL);

  sleep(1);

  //for(int i = 0; i < 10; i++){
  //  raise(SIGALRM);
  //}

  return 0; // Implementation not ready
}

timer_id_t timer_set (Uint32 delay, void *param)
{
  glob_param = param;

  struct itimerval val;
  struct timeval it_value;
  it_value.tv_sec = delay / 1000;
  it_value.tv_usec = (delay % 1000) * 1000;
  struct timeval it_interval;
  it_interval.tv_sec = 0;
  it_interval.tv_usec = 0;
  val.it_value = it_value;
  val.it_interval = it_interval;

  setitimer(ITIMER_REAL, &val, NULL);
  

  return (timer_id_t) NULL;
}

int timer_cancel (timer_id_t timer_id)
{
  // TODO

  return 0; // failure
}

#endif
*/
