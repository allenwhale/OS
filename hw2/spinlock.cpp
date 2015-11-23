#include <unistd.h>
#include <bits/stdc++.h>
#include <pthread.h>
#ifndef THREAD_NUM
#define THREAD_NUM 3
#endif
using namespace std;
pthread_spinlock_t m_spinlock;

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
        pthread_spin_lock(&m_spinlock);
        x.Increment();
        pthread_spin_unlock(&m_spinlock);
    }
    return NULL;
}

int main(){
    pthread_spin_init(&m_spinlock, 0);
    pthread_t tid[THREAD_NUM];
    for(int i=0;i<THREAD_NUM;i++)
        pthread_create(&tid[i], NULL, ThreadRunner, 0);
    for(int i=0;i<THREAD_NUM;i++)
        pthread_join(tid[i], NULL);
    x.Print();
    return 0;
}
