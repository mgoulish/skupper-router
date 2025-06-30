#define _GNU_SOURCE
#include <pthread.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>


#include <qpid/dispatch/blackbox.h>


_Thread_local blackbox_p thread_blackbox = NULL;

#define MAX_BLACKBOXES  100
int blackbox_count = 0;
blackbox_p blackboxes [ MAX_BLACKBOXES ];



blackbox_p
bb_new ( uint64_t size )
{
  if ( size == 0 )
  {
    fprintf ( stderr, "bb_new: bad size\n" );
    exit ( 1 ); 
  }
  
  blackbox_p bb = (blackbox_p) malloc ( sizeof(blackbox_t) );
  
  if ( ! bb )
  {
    fprintf ( stderr, "bb_new: Can't malloc struct.\n" );
    exit ( 1 );
  }

  bb->size = size;
  bb->data = (char *) malloc ( size );

  if ( ! bb->data )
  {
    fprintf ( stderr, "bb_new: Can't malloc data.\n" );
    exit ( 1 );
  }

  memset ( bb->data, 0, bb->size );
  bb->next  = 0;
  bb->thread_id = pthread_self();

  char thread_name[16];
  pthread_getname_np(pthread_self(), thread_name, sizeof(thread_name));
  fprintf ( stderr, "MDEBUG bb_new: made bb for thread |%s|\n", thread_name);

  if ( blackbox_count < MAX_BLACKBOXES ) 
  {
    blackboxes[blackbox_count] = bb;
    ++ blackbox_count;
  }

  return bb;
}





void
bb_write ( blackbox_p bb, char * str )
{
  if ( ! bb )
    return;

  int len = strlen(str);

  for ( int i = 0; i < len; ++ i )
  {
    char c = str[i];
    bb->data[bb->next % bb->size] = c;
    bb->next ++;
  }

  fprintf ( stderr,  "BB %p size %ld\n", (void *) bb, bb->next );
}





void
bb_vwrite ( blackbox_p bb, char const * format, ... )
{
  if ( ! bb ) {
    char thread_name[16];
    pthread_getname_np(pthread_self(), thread_name, sizeof(thread_name));
    fprintf ( stderr, "MDEBUG bb_vwrite: no bb in thread |%s|\n", thread_name);
    return;
  }

  char str [ 1000 ];
  va_list ap;
  va_start ( ap, format );
  vsnprintf ( str, 1000, format, ap );
  va_end ( ap );

  bb_write ( bb, str );
}





static 
void
_bb_dump_one_blackbox ( blackbox_p bb )
{
  char * msg = "Black Box --------------------------------------\n";
  write ( STDERR_FILENO, msg, strlen(msg) );
  fsync(STDERR_FILENO);
  char buf[200];
  snprintf(buf, 200, "bb %p size %ld\n", (void *)bb, bb->next );
  write ( STDERR_FILENO, buf, strlen(buf) );
  fsync(STDERR_FILENO);

  int i = bb->next;
  while ( 1 )
  {
    char c = bb->data [ i % bb->size ];
    if ( c )
    {
      //fputc ( c, fp );
      write(STDERR_FILENO, &c, 1);
      fsync(STDERR_FILENO);
    }

    i ++;

    if ( i % bb->size == bb->next % bb->size )
      break;
  }
}


void
bb_dump ( void )
{
  for ( int i = 0; i < blackbox_count; ++ i )
  {
    _bb_dump_one_blackbox ( blackboxes[i] );
  }
}


