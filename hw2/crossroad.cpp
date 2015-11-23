#include <bits/stdc++.h>
#include <pthread.h>
#include <unistd.h>
using namespace std;
/*
 * N: 0 2
 * W: 2 3
 * S: 3 1
 * E: 1 0
 *
 */

int counter[4], left_car[4], road[4];
pthread_mutex_t counter_mutex[4];
pthread_mutex_t left_car_mutex[4];
pthread_mutex_t road_mutex[4];
pthread_mutex_t deadlock_mutex;
int get_counter(int x){
    pthread_mutex_lock(&counter_mutex[x]);
    int res = counter[x];
    pthread_mutex_unlock(&counter_mutex[x]);
    return res;
}
void increase_counter(int x){
    pthread_mutex_lock(&counter_mutex[x]);
    counter[x]++;
    pthread_mutex_unlock(&counter_mutex[x]);
}
int get_left(int x){
    pthread_mutex_lock(&left_car_mutex[x]);
    int res = left_car[x];
    pthread_mutex_unlock(&left_car_mutex[x]);
    return res;
}
void decrease_left_car(int x){
    pthread_mutex_lock(&left_car_mutex[x]);
    left_car[x]--;
    pthread_mutex_unlock(&left_car_mutex[x]);
}
bool can_go(int x){
    for(int i=0;i<4;i++)
        if(get_left(i) && get_counter(x) > get_counter(i))
            return false;
    return true;
}
bool deadlock(){
    bool cnt[4] = {false};
    for(int i=0;i<4;i++){
        if(road[i] != -1){
            if(cnt[road[i]]) return false;
            cnt[road[i]] = true;
        }else{
            return false;
        }
    }
    return true;
}
class Car{
    public:
        int id;
        char dir;
        vector<int> lock;
        Car(int i=0, char d='N', const vector<int>& l=vector<int>(2)): id(i), dir(d), lock(l){}
        bool Run(int cnt){
            while(true){
                while(can_go(id) == false);
                if(pthread_mutex_trylock(&road_mutex[lock[0]]) == 0){
                    road[lock[0]] = id;
#ifdef DEBUG
                    printf("%d: %c %d acquire lock_%d.\n", get_counter(id), dir, cnt, lock[0]);
#endif
                    increase_counter(id);
                    break;
                }else{
#ifdef DEBUG
                    printf("%d: %c %d acquire lock_%d (fail).\n", get_counter(id), dir, cnt, lock[0]);
#endif
                    increase_counter(id);
                }
            }
            while(true){
                while(can_go(id) == false);
                if(pthread_mutex_trylock(&road_mutex[lock[1]]) == 0){
                    road[lock[1]] = id;
#ifdef DEBUG
                    printf("%d: %c %d acquire lock_%d.\n", get_counter(id), dir, cnt, lock[1]);
#endif
                    increase_counter(id);
                    break;
                }else{
#ifdef DEBUG
                    printf("%d: %c %d acquire lock_%d (fail).\n", get_counter(id), dir, cnt, lock[1]);
#endif
                    pthread_mutex_lock(&deadlock_mutex);
                    bool dead = deadlock();
                    if(dead){
                        printf("A DEADLOCK HAPPENS at %d\n", get_counter(id));
                        while(can_go(id) == false);
                        road[lock[0]] = -1;
#ifdef DEBUG
                        printf("%d: %c %d unlock lock_%d.\n", get_counter(id), dir, cnt, lock[0]);
#endif
                        pthread_mutex_unlock(&road_mutex[lock[0]]);
                        increase_counter(id);
                        usleep(1);
                    }
                    pthread_mutex_unlock(&deadlock_mutex);
                    if(dead) return false;
                    increase_counter(id);
                }
            }
            printf("%c %d leaves at %d\n", dir, cnt, get_counter(id));
            while(can_go(id) == false);
            road[lock[0]] = -1;
#ifdef DEBUG
            printf("%d: %c %d unlock lock_%d.\n", get_counter(id), dir, cnt, lock[0]);
#endif
            pthread_mutex_unlock(&road_mutex[lock[0]]);
            increase_counter(id);
            while(can_go(id) == false);
            road[lock[1]] = -1;
#ifdef DEBUG
            printf("%d: %c %d unlock lock_%d.\n", get_counter(id), dir, cnt, lock[1]);
#endif
            pthread_mutex_unlock(&road_mutex[lock[1]]);
            increase_counter(id);
            decrease_left_car(id);
            return true;
        }
}car[4];
typedef pair<int,int> PI;
void* ThreadRunner(void *arg){
    int id = *(int*)arg;
    int i=1;
    while(get_left(id))
        car[id].Run(i);
    return  NULL;
}
int main(int argc, char **argv){
    if(argc < 5)return 0;
    deadlock_mutex = PTHREAD_MUTEX_INITIALIZER;
    for(int i=0;i<4;i++){
        counter_mutex[i] = PTHREAD_MUTEX_INITIALIZER;
        road_mutex[i] = PTHREAD_MUTEX_INITIALIZER;
        counter[i] = 1;
    }
    int times[] = {stoi(argv[1]), stoi(argv[2]), stoi(argv[3]), stoi(argv[4])};
    car[0] = Car(0, 'N', {0, 2});
    car[1] = Car(1, 'E', {1, 0});
    car[2] = Car(2, 'S', {3, 1});
    car[3] = Car(3, 'W', {2, 3});
    memcpy(left_car, times, sizeof(times));
    pthread_t tid[4];
    int arg[4];
    for(int i=0;i<4;i++){
        arg[i] = i;
        pthread_create(&tid[i], NULL, ThreadRunner, (void*)&arg[i]);
    }
    for(int i=0;i<4;i++)
        pthread_join(tid[i], NULL);
    return 0;
}
