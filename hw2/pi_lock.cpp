#include <bits/stdc++.h>
#include <pthread.h>
#ifndef THREAD_NUM
#define THREAD_NUM 2
#endif
using namespace std;
int total, in_circle;
pthread_mutex_t mutex_t;
class PiCounter{
    private:
        double x_min, x_max, y_min, y_max;
        unsigned int seed;
    public:
        PiCounter(): seed(rand()){}
        void set(double x1, double x2, double y1, double y2){
            x_min = x1;
            x_max = x2;
            y_min = y1;
            y_max = y2;
        }
        void calc(){
            double x = ((double)rand_r(&seed) / RAND_MAX) * (x_max-x_min) + x_min;
            double y = ((double)rand_r(&seed) / RAND_MAX) * (x_max-x_min) + x_min;
            pthread_mutex_lock(&mutex_t);
            if(x*x+y*y <= 1.0){
                in_circle++;
            }
            total++;
            pthread_mutex_unlock(&mutex_t);
        }
};
typedef pair<int, int> PI;
PiCounter pi[THREAD_NUM];
void *Runner(void *args){
    int idx = (*(PI*)args).first;
    int T = (*(PI*)args).second;
    for(int i=0;i<T;i++){
        pi[idx].calc(); 
    }
    return NULL;
}
int main(int argc, char *argv[]){
    if(argc < 2){
        return 0;
    }
    srand(time(0));
    pthread_t tid[THREAD_NUM];
    double delta = 2.0 / THREAD_NUM;
    int T;
    sscanf(argv[1], "%d", &T);
    for(int i=0;i<THREAD_NUM;i++){
        pi[i].set(-1, 1, -1 + delta * i, -1 + delta * (i+1));
    }
    int each_T = T / THREAD_NUM;
    PI args[THREAD_NUM];
    for(int i=0;i<THREAD_NUM;i++){
        args[i] = {i, min(each_T, T)};
        pthread_create(&tid[i], NULL, Runner, &args[i]);
        T -= min(each_T, T);
    }
    for(int i=0;i<THREAD_NUM;i++){
        pthread_join(tid[i], NULL);
    }
    printf("pi estimate = %lf\n", (double)in_circle / total * 4);
    return 0;
}
