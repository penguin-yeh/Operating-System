#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)
long g;

static inline int my_spin_lock (atomic_int *lock) {
    int val=0; //將value的初始值設為0
    if (likely(atomic_exchange_explicit(lock, 1, memory_order_acq_rel) == 0)) //將兩參數的值進行對調，然後回傳lock對調前本來的值。使用memory_order_acq_rel代表在此之前執行的指令不可跑到後面執行，在後面執行的指令亦不可跑到前面執行。
        return 0; // 如果lock本來為0，就會回傳0，成功獲得lock。
    do {
        do {
            asm("pause"); //不要讓while跑太快，讓CPU負擔不要那麼重，可有效降低CPU溫度。
        } while (*lock != 0); //如果lock=1，表示別人先lock了，因此會一直在這個while裡面直到發現lock為0，才會跳出此迴圈。
        val = 0; // 將val設為0，表示目前發現lock=0，為未被上鎖的狀態。
    } while (!atomic_compare_exchange_weak_explicit(lock, &val, 1, memory_order_acq_rel, memory_order_relaxed)); //如果lock=val，則會將lock=1(上鎖)，並回傳True，跳出此loop，否則會將val=lock並回傳False。用weak表示在未來的某個時間點一定會有一個thread成功上鎖，跳出此迴圈。
    return 0; //成功獲得lock
}
static inline int my_spin_unlock(atomic_int *lock) {
    atomic_store_explicit(lock, 0, memory_order_release); //將lock設為0，表示目前為被上鎖，然後用release傳遞結果給別人知道。
    return 0; //成功解鎖
}

atomic_int a_lock;
atomic_long count_array[256];
int numCPU;

void sigHandler(int signo) {
    for (int i=0; i<numCPU; i++) {
        printf("%i, %ld\n", i, count_array[i]);
    }
    exit(0);
}

atomic_int in_cs=0;
atomic_int wait=1;

void thread(void *givenName) {
    int givenID = (intptr_t)givenName; //將pointer轉型成int
    srand((unsigned)time(NULL)); //依照time設定rand()時的種子
    unsigned int rand_seq;

    //底下兩行會設定把第n個thread跑在第n顆CPU上面
    cpu_set_t set; CPU_ZERO(&set); CPU_SET(givenID, &set);
    sched_setaffinity(gettid(), sizeof(set), &set);

    while(atomic_load_explicit(&wait, memory_order_acquire)) //讓每個thread可以同時一起執行，不會有人偷跑。
        ;
    while(1) {
        my_spin_lock(&a_lock); //上鎖
        atomic_fetch_add(&in_cs, 1); //將in_cs++，表示有人進入了critical section。
        atomic_fetch_add_explicit(&count_array[givenID], 1, memory_order_relaxed); //記錄下每個thread進入critical section的次數
        if (in_cs != 1) { //若是發現in_cs!=1，表示可能發生了同時有兩個人在critical section的情形，不符合mutual exclusion的條件。
            printf("violation: mutual exclusion\n");
            exit(0); //終止程式
        }
        atomic_fetch_add(&in_cs, -1); //離開critical section時將in_cs--，表示目前沒人在critical section。
        my_spin_unlock(&a_lock); //解鎖

        //底下兩行相當於sleep。由於thread間隱含著同步效應，因此需要讓thread sleep一下
        int delay_size = rand_r(&rand_seq)%73; //決定下面for迴圈要跑幾次
        for (int i=0; i<delay_size; i++);
        // {
        //     g++;
        // }    
    }
}

int main(int argc, char **argv) {
    signal(SIGALRM, sigHandler);
    alarm(5);
    numCPU = sysconf( _SC_NPROCESSORS_ONLN );
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t) * numCPU);

    for (long i=0; i< numCPU; i++)
        pthread_create(&tid[i],NULL,(void *) thread, (void*)i);
    atomic_store(&wait,0);

    for (int i=0; i< numCPU; i++)
        pthread_join(tid[i],NULL);
}
