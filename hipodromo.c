#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

const int PROGRESS_BAR_LENGTH = 75;

#define ESC "\033"
#define CSI "["
#define PREVLINE "F"
#define BACKSPACE "D"

typedef struct
{
  int count_to_val;
  int progress;
  pthread_t thethread;
  int horse_number;
} thread_args;

bool race_over = false;                                 // Variable global para indicar si la carrera ha terminado
int winner = -1;                                        // Variable global para almacenar el número del caballo ganador
pthread_mutex_t race_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para proteger el acceso a las variables globales

void update(thread_args *tinfo)
{
  int num_blocks = (tinfo->progress * 100 / tinfo->count_to_val) * PROGRESS_BAR_LENGTH / 100;

  printf("Caballo %d: [", tinfo->horse_number);
  for (int i = 0; i < num_blocks; i++)
  {
    printf("-");
  }

  if (tinfo->progress < tinfo->count_to_val)
  {
    printf(ESC CSI BACKSPACE ">");
  }

  for (int i = 0; i < PROGRESS_BAR_LENGTH - num_blocks; i++)
  {
    printf(" ");
  }
  printf("]\n");
}

void *mythreadfunc(void *arg)
{
  thread_args *tinfo = arg;

  while (tinfo->progress < tinfo->count_to_val)
  {
    tinfo->progress += rand() % 5 + 1; // Incremento aleatorio entre 1 y 5
    if (tinfo->progress > tinfo->count_to_val)
    {
      tinfo->progress = tinfo->count_to_val;
    }
    usleep(10000); // Espera de 100ms entre incrementos

    pthread_mutex_lock(&race_mutex);
    if (!race_over && tinfo->progress >= tinfo->count_to_val)
    {
      race_over = true;
      winner = tinfo->horse_number;
    }
    pthread_mutex_unlock(&race_mutex);
  }

  return NULL;
}

int main()
{
  srand(time(NULL)); // Inicializa el generador de números aleatorios

  int NUM_HORSES;

  printf("\nHIPODROMO\n");
  printf("====================================================\n");
  printf("Bienvenio a la carrera de caballos\n");
  printf("¿Cuántos caballos quieres que corran?\n");
  scanf("%d", &NUM_HORSES);
  printf("====================================================\n\n");

  thread_args threads[NUM_HORSES];

  for (int i = 0; i < NUM_HORSES; i++)
  {
    threads[i].count_to_val = rand() % 400 + 600;
    threads[i].progress = 0;
    threads[i].horse_number = i + 1;
    pthread_create(&threads[i].thethread, NULL, mythreadfunc, &threads[i]);
  }

  bool done = false;

  while (!done)
  {
    done = true;
    for (int i = 0; i < NUM_HORSES; i++)
    {
      update(&threads[i]);
      if (threads[i].progress < threads[i].count_to_val)
      {
        done = false;
      }
    }

    if (!done)
    {
      printf(ESC CSI "%d" PREVLINE, NUM_HORSES);
    }
    usleep(10000);

    pthread_mutex_lock(&race_mutex);
    if (race_over)
    {
      done = true;
    }
    pthread_mutex_unlock(&race_mutex);
  }
  printf("\n\n\n\n\n¡Carrera terminada!\n\n");
  if (winner != -1)
  {
    printf("¡El caballo %d es el ganador!\n", winner);
  }

  return 0;
}
