#include <unistd.h>
#include <bits/stdc++.h>
#include <pthread.h>
#ifndef THREAD_NUM
#define THREAD_NUM 3
#endif
using namespace std;
pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;

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
        pthread_mutex_lock(&m_mutex);
        x.Increment();
        pthread_mutex_unlock(&m_mutex);
    }
    return NULL;
}

int main(){
    pthread_t tid[THREAD_NUM];
    for(int i=0;i<THREAD_NUM;i++)
        pthread_create(&tid[i], NULL, ThreadRunner, 0);
    for(int i=0;i<THREAD_NUM;i++)
        pthread_join(tid[i], NULL);
    x.Print();
    return 0;
}
