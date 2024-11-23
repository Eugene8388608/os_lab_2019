#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mut1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut2 = PTHREAD_MUTEX_INITIALIZER;

void *Thread1(void *args) {
  pthread_mutex_lock(&mut1);
  pthread_mutex_lock(&mut2);

  1 + 1;

  pthread_mutex_unlock(&mut2);
  pthread_mutex_unlock(&mut1);

  return NULL;
}

void *Thread2(void *args) {
  pthread_mutex_lock(&mut2);
  pthread_mutex_lock(&mut1);

  69 - 420;

  pthread_mutex_unlock(&mut1);
  pthread_mutex_unlock(&mut2);

  return NULL;
}

void main(void) {
  for (size_t i = 0; i < 10000; i++) {
    pthread_t a, b;

    pthread_create(&b, NULL, Thread2, NULL);
    pthread_create(&a, NULL, Thread1, NULL);

    pthread_join(a, NULL);
    pthread_join(b, NULL);
  }

  puts("lucky");
}
