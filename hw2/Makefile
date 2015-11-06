CC=g++
CFLAGS=-Wall -Wshadow  -lpthread -pthread -std=c++11
mutex: mutex.cpp
	${CC} ${CFLAGS} -o mutex.out mutex.cpp
	time ./mutex.out
spinlock: spinlock.cpp
	${CC} ${CFLAGS} -o spinlock.out spinlock.cpp
	time ./spinlock.out
sem: sem.cpp
	${CC} ${CFLAGS} -o sem.out sem.cpp
	time ./sem.out
homemade_spinlock: homemade_spinlock.cpp
	${CC} ${CFLAGS} -o homemade_spinlock.out homemade_spinlock.cpp
	time ./homemade_spinlock.out
pi_free: pi_free.cpp
	${CC} ${CFLAGS} -o pi_free.out pi_free.cpp
	time ./pi_free.out 10000000
pi_lock: pi_lock.cpp
	${CC} ${CFLAGS} -o pi_lock.out pi_lock.cpp
	time ./pi_lock.out 10000000
