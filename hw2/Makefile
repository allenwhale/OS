CC=g++
CFLAGS=-Wall -Wshadow  -lpthread -pthread -std=c++11
all: mutex.out spinlock.out sem.out homemade_spinlock.out pi_free.out pi_lock.out crossroad.out
	@echo "done"
mutex.out: mutex.cpp
	${CC} ${CFLAGS} -o mutex.out mutex.cpp
	#time ./mutex.out
spinlock.out: spinlock.cpp
	${CC} ${CFLAGS} -o spinlock.out spinlock.cpp
	#time ./spinlock.out
sem.out: sem.cpp
	${CC} ${CFLAGS} -o sem.out sem.cpp
	#time ./sem.out
homemade_spinlock.out: homemade_spinlock.cpp
	${CC} ${CFLAGS} -o homemade_spinlock.out homemade_spinlock.cpp
	#time ./homemade_spinlock.out
pi_free.out: pi_free.cpp
	${CC} ${CFLAGS} -o pi_free.out pi_free.cpp
	#time ./pi_free.out 10000000
pi_lock.out: pi_lock.cpp
	${CC} ${CFLAGS} -DTHREAD_NUM=3 -o pi_lock.out pi_lock.cpp
	#time ./pi_lock.out 1000000000
crossroad.out: crossroad.cpp
	${CC} ${CFLAGS} -o crossroad.out crossroad.cpp
clean:
	rm *.out
