#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#include <unistd.h>
#include <errno.h>

struct Monitor {
    int sizeBuffer;
    sem_t mutex, full, empty;
}m;

int semWait(sem_t* sem) {
    if(sem_wait(sem)) {
        perror(NULL);
        return errno;
    }
}

int semPost(sem_t* sem) {
    if(sem_post(sem)) {
        perror(NULL);
        return errno;
    }
}

void* producer (void* param) {
    int* numItems = (int*) param;

    while(1) {
        semWait(&m.full);
        semWait(&m.mutex);

        if (*numItems > 0 ){
            printf("%d added\n", 1);
            (*numItems)--;
        }

        if (*numItems == 0) {
            semPost(&m.empty);
            semPost(&m.mutex);
            return 0;
        }

        semPost(&m.empty);
        semPost(&m.mutex);
    } 
}

void* consumer (void* param) {
    int* numItems = (int*) param;

    while (1) {
        semWait(&m.empty); 
        semWait(&m.mutex);

        if (*numItems > 0) {
            printf("%d taken\n", 1);
            (*numItems) --;
        }
        
        if (*numItems == 0) {
            semPost(&m.full);
            semPost(&m.mutex);
            return 0;
        }

        semPost(&m.full);
        semPost(&m.mutex);
    }
}
int init(struct Monitor* m, int pSizeBuff) {
    
    m->sizeBuffer = pSizeBuff;
    if (sem_init(&m->mutex, 0, 1) || sem_init(&m->full, 0, pSizeBuff) || sem_init(&m->empty, 0, 0)) {
        perror(NULL);
        return errno;
    }    
    return 0;
}
int main (int argc, char*argv[]) {
    // freopen("test1.txt", "r", stdin);

    int sizeBuff, numOfProducer, numOfConsumer;
    scanf("%d%d", &sizeBuff, &numOfProducer);

    int prod[numOfProducer];
    int prod_sum = 0;
    for (int i = 0; i < numOfProducer; i++) {
        scanf("%d", &prod[i]);
        prod_sum += prod[i];
    }

    scanf("%d", &numOfConsumer);
    int cons_sum = 0;
    int cons[numOfConsumer];
    for( int i = 0; i < numOfConsumer; i++) {
        scanf("%d", &cons[i]);
        cons_sum += cons[i];
    }
    if (cons_sum > prod_sum) {
        printf("Too many requests!\n");
        return 0;
    }
    if (prod_sum - cons_sum > sizeBuff) {
        printf("Buffer size is too small\n");
        return 0;
    }

    init (&m, sizeBuff);
    pthread_t ptid[numOfProducer];
    pthread_t ctid[numOfConsumer];

    for (int i = 0; i < numOfConsumer; i++) {
        int *x = (int*)malloc(sizeof(int));
        *x = cons[i];
        if (pthread_create(&ctid[i], NULL, consumer, (void*)(x))) {
            perror(NULL);
            return errno;
        }
    }
    for (int i = 0; i < numOfProducer; i++) {
        int *x = (int*)malloc(sizeof(int));
        *x = prod[i];
        if (pthread_create(&ptid[i], NULL, producer, (void*)(x))){
            perror(NULL);
            return errno;
        }
    }

    for(int i = 0; i < numOfProducer;i++) {
        if (pthread_join(ptid[i], NULL)) {
            perror(NULL);
            return errno;
        }
    }

    for(int i = 0; i < numOfConsumer;i++) {
        if (pthread_join(ctid[i], NULL)) {
            perror(NULL);
            return errno;
        }
    }

    if (sem_destroy (&m.mutex) ) {
        perror(NULL);
        return errno;
    }
    
    if (sem_destroy (&m.empty) ) {
        perror(NULL);
        return errno;
    }
    if (sem_destroy (&m.full) ) {
        perror(NULL);
        return errno;
    }
    
    return 0;
}
