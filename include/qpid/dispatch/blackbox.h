#ifndef QPID_DISPATCH_BLACKBOX_H
#define QPID_DISPATCH_BLACKBOX_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "qpid/dispatch/threading.h"


typedef 
struct blackbox_s
{ 
  uint64_t  size,
            next;
  char    * data; 
  pthread_t thread_id;
}
blackbox_t,
* blackbox_p;


extern _Thread_local blackbox_p thread_blackbox;



blackbox_p
bb_new ( uint64_t size );


void
bb_write ( blackbox_p bb, char * str );


void
bb_vwrite ( blackbox_p bb, char const * format, ... );


void
bb_dump ( void );


#endif // QPID_DISPATCH_BLACKBOX_H
