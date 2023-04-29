#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef struct trio_t
{
    int a, b, c, finished;
    pthread_mutex_t vars;
    pthread_cond_t a_enter;
    pthread_cond_t b_enter;
    pthread_cond_t c_enter;
    pthread_cond_t can_start;
} trio_t;

struct arg_struct
{
    int tid;
    int ttype;
    int tsolo;
    int ttrio;
    trio_t *t_ptr;
};

void init_trio(trio_t t)
{
    t.a = 0;                                 // representa threads do tipo 1
    t.b = 0;                                 // tipo 2
    t.c = 0;                                 // tipo 3
    t.finished = 0;                          // controla se o trio aceita novas threads
    pthread_mutex_init(&(t.vars), NULL);     // mutex de acesso as variaveis
    pthread_cond_init(&(t.a_enter), NULL);   // condição que controla se threads A podem entrar no trio
    pthread_cond_init(&(t.b_enter), NULL);   // condição que controla se threads B podem entrar no trio
    pthread_cond_init(&(t.b_enter), NULL);   // condição que controla se threads C podem entrar no trio
    pthread_cond_init(&(t.can_start), NULL); // controla se threads em um trio podem começar
};
void trio_enter(trio_t *t, int my_type)
{

    pthread_mutex_lock(&(t->vars));
   
    if (my_type == 1)
    {

        // espera até existir espaço para A entrar e o trio estar aceitando novas threads
        // o trio não aceita novas threads até todas as threads  sairem do trio
  
        while (t->finished == 1 || t->a == 1)
        {
           
            pthread_cond_wait(&(t->a_enter), &(t->vars));
        }
     
        t->a = 1;
    }
    if (my_type == 2)
    {

        while (t->finished == 1 || t->b == 1)
        {
          

            pthread_cond_wait(&(t->b_enter), &(t->vars));
        }
      
        t->b = 1;
    }
    if (my_type == 3)
    {

        while (t->finished == 1 || t->c == 1)
        {
            
            pthread_cond_wait(&(t->c_enter), &(t->vars));
        }

        t->c = 1;
    }
   
    // o trio está completo, as threads podem começar
    while (t->a + t->b + t->c != 3)
    {
        pthread_cond_wait(&(t->can_start), &(t->vars));
    }
    // se as tres threads existem no trio , logo todas devem sair antes de alguem entrar
    t->finished = 1;
    pthread_cond_broadcast(&(t->can_start));
    pthread_mutex_unlock(&(t->vars));
};
void trio_leave(trio_t *t, int my_type)
{

    pthread_mutex_lock(&(t->vars));
    if (my_type == 1)
    {
        t->a = 0;
    }
    if (my_type == 2)
    {
        t->b = 0;
    }
    if (my_type == 3)
    {
        t->c = 0;
    }

    // se todos sairem , o trio aceita nova threads.
    if (t->a + t->b + t->c == 0)
    {
        t->finished = 0;

        pthread_cond_signal(&(t->a_enter));
        pthread_cond_signal(&(t->b_enter));
        pthread_cond_signal(&(t->c_enter));
      
        
    }
   
    pthread_mutex_unlock(&(t->vars));
   
};

void *routine(void *args)
{

    struct arg_struct *arg = args;

    int tid, ttype, tsolo, ttrio;
    tid = arg->tid;
    ttype = arg->ttype;
    tsolo = arg->tsolo;
    ttrio = arg->ttrio;

    spend_time(tid, ttype, "S", tsolo);
    trio_enter((arg->t_ptr), ttype);
    spend_time(tid, ttype, "T", ttrio);
    trio_leave((arg->t_ptr), ttype);
    free(args);
    pthread_exit(NULL);
    return NULL;
}

int main()
{
    char line[100];
    pthread_t th[1000];
    int n_line = 0;
    trio_t t;
    init_trio(t);
    clock_t begin = clock();

    while (fgets(line, 100, stdin))
    {
        
        int tid, ttype, tsolo, ttrio;

        char *token = strtok(line, " ");
        tid = atoi(token);

        token = strtok(NULL, " ");
        ttype = atoi(token);

        token = strtok(NULL, " ");
        tsolo = atoi(token);

        token = strtok(NULL, " ");
        ttrio = atoi(token);

        struct arg_struct *arg = malloc(sizeof *arg);
        arg->tid = tid;
        arg->ttype = ttype;
        arg->tsolo = tsolo;
        arg->ttrio = ttrio;
        arg->t_ptr = &(t);

        if (pthread_create(&th[n_line], NULL, &routine, arg) != 0)
        {
            printf("errror creating thread!\n");
            return -1;
        }

        n_line++;
    }
    for (int i = 0; i < n_line; i++)
    {
        pthread_join(th[i], NULL);
    }
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("%f",time_spent);
}