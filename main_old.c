#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct trio_t
{
    int a, b, c, finished;
    pthread_mutex_t vars;
    pthread_cond_t enter;
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
    t.a = 0;
    t.b = 0;
    t.c = 0;
    t.finished =0;
    pthread_mutex_init(&(t.vars), NULL);
    pthread_cond_init(&(t.enter), NULL);
    pthread_cond_init(&(t.can_start), NULL);
};
void trio_enter(trio_t* t, int my_type)
{
    
    pthread_mutex_lock(&(t->vars));
    if (my_type == 1)
    {
           
        // existe espaço para a entrar e o trio está aceitando novas threads
        // o trio não aceita novas threads até todas as threads que já finalizaram sairem do trio
        while (t->finished == 1 || t->a == 1)
        {
            // printf("%d espera finishado %d \n",my_type,t->finished);
            pthread_cond_wait(&(t->enter), &(t->vars));
        }
       
     
        t->a = 1;
        // se as tres threads existem no trio , logo todas devem sair antes de alguem entrar
       
        while (t->a + t->b + t->c != 3)
        {
            // printf("%d espera  %d %d %d\n",my_type,t->a,t->b,t->c);
            pthread_cond_wait(&(t->can_start),&(t->vars));
        }

       
        t->finished=1;
        pthread_cond_signal(&(t->can_start));



        
    }
    if (my_type == 2)
    {
       
        while (t->finished == 1 || t->b == 1)
        {
        //    printf("%d espera finishado %d \n",my_type,t->finished);
            pthread_cond_wait(&(t->enter), &(t->vars));
        }
        
       
        t->b = 1;

        while (t->a + t->b + t->c != 3)
        {
            // printf("%d espera  %d %d %d\n",my_type,t->a,t->b,t->c);
            pthread_cond_wait(&(t->can_start),&(t->vars));
        }
        
        t->finished=1;
        pthread_cond_signal(&(t->can_start));
        
    }   
    if (my_type == 3)
    {
         
        while (t->finished == 1 || t->c == 1)
        {
            //  printf("%d espera finishado %d \n",my_type,t->finished);
            pthread_cond_wait(&(t->enter), &(t->vars));
        }
      
    
        t->c = 1;

        while (t->a + t->b + t->c != 3)
        {
            // printf("%d espera  %d %d %d\n",my_type,t->a,t->b,t->c);
            pthread_cond_wait(&(t->can_start),&(t->vars));
        }
        
        t->finished=1;
        pthread_cond_signal(&(t->can_start));
        
    }
    // printf("%d entrou \n",my_type);
    
    pthread_mutex_unlock(&(t->vars));
};
void trio_leave(trio_t *t, int my_type)
{
    
    // printf("\n \n \n \n \n \n %d \n \n \n \n ", my_type);
    pthread_mutex_lock(&(t->vars));
    if (my_type==1){
        t->a=0;
    }
    if (my_type==2){
        t->b=0;
    }
    if (my_type==3){
        t->c=0;
    }
    // printf("%d saiu %d  \n",my_type,t->a+t->b+t->c);
    //se todos sairem , o trio aceita nova threads.
    if(t->a + t->b + t->c ==0){
        t->finished=0;
        // printf("liberado \n");
        pthread_cond_broadcast(&(t->enter));
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
    ttrio =arg->ttrio;
    
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
    clock_t begin = clock();
    char line[100];
    pthread_t th[1000];
    int n_line = 0;
    trio_t t;
    init_trio(t);
       

/* here, do your time-consuming job */
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