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

inline int semWait(sem_t* sem) {
    if(sem_wait(sem)) {
        perror(NULL);
        return errno;
    }
}

inline int semPost(sem_t* sem) {
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
    if (sizeBuff == 0 || numOfProducer == 0) {
        printf("Wrong input\n");
        return 0;
    }

    int prod[numOfProducer];
    for (int i = 0; i < numOfProducer; i++) {
        scanf("%d", &prod[i]);
    }

    scanf("%d", &numOfConsumer);
    if(numOfConsumer == 0) {
        printf("Wrong input\n");
        return 0;
    }
    int cons[numOfConsumer];
    for( int i = 0; i < numOfConsumer; i++) {
        scanf("%d", &cons[i]);
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
        pthread_join(ptid[i], NULL);
    }

    for(int i = 0; i < numOfConsumer;i++) {
        pthread_join(ctid[i], NULL);
    }
    
    return 0;
}