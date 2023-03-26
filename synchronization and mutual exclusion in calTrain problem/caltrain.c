#include <pthread.h>
#include "caltrain.h"

void station_init(struct station *station)
{
	// FILL ME IN
	pthread_mutex_init(&(station->lock), NULL);
	station->trainFreeSeats = 0;
	station->waitingPassengers = 0;
	station->passengerQueue = 0;
	pthread_cond_init(&(station->trainArrival), NULL);
	pthread_cond_init(&(station->trainDepart), NULL);
}

void station_load_train(struct station *station, int count)
{

	/*
	1. train arrives to station
	2. if there is no one waiting or no available seats train leaves
	3. train sets available seats
	4. train tells passenger threads waiting on its arrival
	5. train waits till its full or no other waiting passengers*/
	pthread_mutex_lock(&station->lock);
	if (count == 0 || station->waitingPassengers == 0)
	{
		pthread_mutex_unlock(&station->lock);
		return;
	}
	station->trainFreeSeats = count;
	pthread_cond_broadcast(&station->trainArrival);
	pthread_cond_wait(&station->trainDepart, &station->lock);
	station->trainFreeSeats = 0;
	pthread_mutex_unlock(&station->lock);
}

void station_wait_for_train(struct station *station)
{

	/*
	1. passenger increments waiting passengers
	3. check for available train seats
	2. wait for train arrival condition
	3. queue for train on its arrival and leave waiting
	*/
	pthread_mutex_lock(&station->lock);
	station->waitingPassengers++;
	while (station->trainFreeSeats == 0 || station->passengerQueue == station->trainFreeSeats)
	{
		pthread_cond_wait(&station->trainArrival, &station->lock);
	}
	station->passengerQueue++;
	station->waitingPassengers--;
	pthread_mutex_unlock(&station->lock);
}

void station_on_board(struct station *station)
{

	/*
	1. once the passenger is in train decrement waiting passengers
	2. increment on board passengers
	3. if no more free seats or waiting passengers tell the train to leave*/

	pthread_mutex_lock(&station->lock);
	station->passengerQueue--;
	station->trainFreeSeats--;
	if (station->trainFreeSeats == 0 || (station->passengerQueue == 0 && station->waitingPassengers == 0))
	{
		pthread_cond_signal(&station->trainDepart);
	}
	pthread_mutex_unlock(&station->lock);
}
