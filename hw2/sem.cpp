#include <unistd.h>
#include <bits/stdc++.h>
#include <semaphore.h>
#ifndef THREAD_NUM
#define THREAD_NUM 3
#endif
using namespace std;
sem_t sem;

class Counter{
    private:
        int value;
    public:
        Counter() : value(0) {}
        void Increment(){
            value++;
        }
        void Print(){
            cout << value;
        }
};
Counter x;

void* ThreadRunner(void*){
    for(int k = 0 ; k < 100000000 ; k++){
        sem_wait(&sem);
        x.Increment();
        sem_post(&sem);
    }
    return NULL;
}

int main(){
    sem_init(&sem, 0, 1);
    pthread_t tid[THREAD_NUM];
    for(int i=0;i<THREAD_NUM;i++)
        pthread_create(&tid[i], NULL, ThreadRunner, 0);
    for(int i=0;i<THREAD_NUM;i++)
        pthread_join(tid[i], NULL);
    sem_destroy(&sem);
    x.Print();
    return 0;
}
