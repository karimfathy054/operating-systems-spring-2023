#include <pthread.h>

struct station {
	pthread_mutex_t lock;
	int trainFreeSeats;
	int waitingPassengers;
	int passengerQueue;
	pthread_cond_t trainArrival;
	pthread_cond_t trainDepart;
};

void station_init(struct station *station);

void station_load_train(struct station *station, int count);

void station_wait_for_train(struct station *station);

void station_on_board(struct station *station);