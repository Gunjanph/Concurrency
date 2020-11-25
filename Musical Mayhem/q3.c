#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <inttypes.h>
#include <math.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>
#include <errno.h>

typedef long long int ll;

typedef struct musician
{
	ll id;
	char name[1000];
	char instrument;
	ll arrival_time;
	pthread_t thread;
}Musician;

typedef struct Stage
{
	ll id;
	ll time;
	ll sings;
	ll empty;
	char type;
	char name[1000];
	char instrument;
	char singer_name[1000];
	sem_t perform;
	pthread_mutex_t mutex;
}Stage;


ll randomno(ll l,ll h)
{
	ll num;
	num = (rand()%(h-l+1))+l;
	return num;
}

ll k,a,e,c,t1,t2,t;
Musician musician[1000];
Stage stage[1000];

sem_t asem;
sem_t esem;
sem_t bsem;
sem_t ssem;
sem_t csem;

struct timespec ts;

void tshirt(Musician *musician)
{
	sem_wait(&csem);
	printf("\033[0;35m%s is collecting t-shirt\n",musician->name);
	printf("\033[0m");
	sleep(2);
	printf("\033[0;33m%s left after Performing :,)\n",musician->name);
	sem_post(&csem);
}

void *Acoustic(void *inp)
{
	Musician *acoustic = (Musician*)inp;
	sleep(acoustic->arrival_time);
	printf("\033[0;32m%s plays %c arrived at time t = %lld\n",acoustic->name,acoustic->instrument,acoustic->arrival_time);
	printf("\033[0m");
	while(1)
	{
		if((sem_timedwait(&asem,&ts) == -1) && errno == ETIMEDOUT)
		{
			printf("\033[0;31m%s plays %c left due to impatience :,(\n",acoustic->name,acoustic->instrument);
			printf("\033[0m");
			return NULL;
		}
		else
		{
			for(ll i=0;i<(a+e);i++)
			{
				pthread_mutex_lock(&(stage[i].mutex));
				if((stage[i].empty == 0) && (stage[i].type == 'a'))
				{
					stage[i].empty = 1;
					strcpy(stage[i].name, acoustic->name);
					stage[i].instrument = acoustic->instrument;
					stage[i].time = randomno(t1,t2);
					printf("\033[0;36m%s plays %c at acoustic stage number %lld for %lld sec\n",acoustic->name,acoustic->instrument,stage[i].id,stage[i].time);
					printf("\033[0m");
					pthread_mutex_unlock(&(stage[i].mutex));
					sleep(stage[i].time);
					if(stage[i].sings == 1)
					{
						sleep(2);
						sem_post(&stage[i].perform);
					}
					pthread_mutex_lock(&(stage[i].mutex));
					stage[i].empty = 0;
					pthread_mutex_unlock(&(stage[i].mutex));
					tshirt(acoustic);
					return NULL;
				}
				else
					pthread_mutex_unlock(&(stage[i].mutex));
			}
		}
	}
	return NULL;
}

void *Electric(void *inp)
{
	Musician *electric = (Musician*)inp;
	sleep(electric->arrival_time);
	printf("\033[0;32m%s plays %c arrived at time t = %lld\n",electric->name,electric->instrument,electric->arrival_time);
	printf("\033[0m");
	while(1)
	{
		if(sem_timedwait(&esem,&ts) == -1 && errno == ETIMEDOUT)
		{	
			printf("\033[0;31m%s plays %c left due to impatience :,(\n",electric->name,electric->instrument);
			printf("\033[0m");
			return NULL;
		}
		else
		{
			// printf("%lld\n",electric->id);
			for(ll i=0;i<(a+e);i++)
			{
				pthread_mutex_lock(&(stage[i].mutex));
				// printf("%lld\n",electric->id);
				if((stage[i].empty == 0) && (stage[i].type == 'e'))
				{
					stage[i].empty = 1;
					strcpy(stage[i].name, electric->name);
					stage[i].instrument = electric->instrument;
					stage[i].time = randomno(t1,t2);
					printf("\033[0;36m%s plays %c at electric stage number %lld for %lld sec\n",electric->name,electric->instrument,stage[i].id,stage[i].time);
					printf("\033[0m");
					pthread_mutex_unlock(&(stage[i].mutex));
					sleep(stage[i].time);
					if(stage[i].sings == 1)
					{
						sleep(2);
						sem_post(&stage[i].perform);
					}
					pthread_mutex_lock(&(stage[i].mutex));
					stage[i].empty = 0;
					pthread_mutex_unlock(&(stage[i].mutex));
					tshirt(electric);
					return NULL;
				}
				else
					pthread_mutex_unlock(&(stage[i].mutex));
			}
		}
	}
	return NULL;
}

void *Both(void *inp)
{
	Musician *both = (Musician*)inp;
	sleep(both->arrival_time);
	printf("\033[0;32m%s plays %c arrived at time t = %lld\n",both->name,both->instrument,both->arrival_time);
	printf("\033[0m");
	while(1)
	{
		if(sem_timedwait(&bsem,&ts) == -1 && errno == ETIMEDOUT)
		{
			printf("\033[0;31m%s plays %c left due to impatience :,(\n",both->name,both->instrument);
			printf("\033[0m");
			return NULL;
		}
		else
		{
			for(ll i=0;i<(a+e);i++)
			{
				pthread_mutex_lock(&(stage[i].mutex));
				if((stage[i].empty == 0))
				{
					stage[i].empty = 1;
					strcpy(stage[i].name, both->name);
					stage[i].instrument = both->instrument;
					stage[i].time = randomno(t1,t2);
					if(stage[i].type == 'e')
					{
						printf("\033[0;36m%s plays %c at electric stage number %lld for %lld sec\n",both->name,both->instrument,stage[i].id,stage[i].time);
						printf("\033[0m");
					}
					else
					{
						printf("\033[0;36m%s plays %c at acoustic stage number %lld for %lld sec\n",both->name,both->instrument,stage[i].id,stage[i].time);
						printf("\033[0m");
					}
					pthread_mutex_unlock(&(stage[i].mutex));
					sleep(stage[i].time);
					if(stage[i].sings == 1)
					{
						sleep(2);
						sem_post(&stage[i].perform);
					}
					pthread_mutex_lock(&(stage[i].mutex));
					stage[i].empty = 0;
					pthread_mutex_unlock(&(stage[i].mutex));
					tshirt(both);
					return NULL;
				}
				else
					pthread_mutex_unlock(&(stage[i].mutex));
			}
		}
	}
	return NULL;
}

void *Singer(void *inp)
{
	Musician *singer = (Musician*)inp;
	sleep(singer->arrival_time);
	printf("\033[0;32m%s plays %c arrived at time t = %lld\n",singer->name,singer->instrument,singer->arrival_time);
	printf("\033[0m");
	while(1)
	{
		if(sem_timedwait(&ssem,&ts) == -1 && errno == ETIMEDOUT)
		{
			printf("\033[0;31m%s plays %c left due to impatience :,(\n",singer->name,singer->instrument);
			printf("\033[0m");
			return NULL;
		}
		else
		{
			for(ll i=0;i<(a+e);i++)
			{
				pthread_mutex_lock(&(stage[i].mutex));
				if((stage[i].sings == 0))
				{
					stage[i].sings = 1;
					pthread_mutex_unlock(&(stage[i].mutex));
					strcpy(stage[i].singer_name, singer->name); 
					if(stage[i].empty == 1)
					{
						if(stage[i].type == 'e')
						{
							printf("\033[0;34m%s joined %s's performance at electric stage number %lld extended by 2 sec\n",singer->name,stage[i].name,stage[i].id);
							printf("\033[0m");
						}
						else
						{
							printf("\033[0;34m%s joined %s's performance at acoustic stage number %lld extended by 2 sec\n",singer->name,stage[i].name,stage[i].id);
							printf("\033[0m");
						}
						sem_wait(&stage[i].perform);
						pthread_mutex_lock(&(stage[i].mutex));
						stage[i].sings=0;
						pthread_mutex_unlock(&(stage[i].mutex));
						tshirt(singer);
					}
					else
					{
						stage[i].empty = 1;
						stage[i].time = randomno(t1,t2);
						pthread_mutex_unlock(&(stage[i].mutex));
						if(stage[i].type == 'e')
						{
							printf("\033[0;36m%s starts solo performance at electric stage number %lld for %lld sec\n",singer->name,stage[i].id,stage[i].time);
							printf("\033[0m");
						}
						else
						{
							printf("\033[0;36m%s starts solo performance at acoustic stage number %lld for %lld sec\n",singer->name,stage[i].id,stage[i].time);
							printf("\033[0m");
						}
						sleep(stage[i].time);
						pthread_mutex_lock(&(stage[i].mutex));
						stage[i].sings=0;
						stage[i].empty=0;
						pthread_mutex_unlock(&(stage[i].mutex));
						tshirt(singer);
					}
					return NULL;
				}
				else
					pthread_mutex_unlock(&(stage[i].mutex));
			}
		}
	}
	return NULL;
}

int main()
{
	srand(time(0));
	printf("Enter the no. of Musicians, Acoustic and Electric Stages, Coordinators, Min and Max Duration, Waiting Time\n");
	scanf("%lld%lld%lld%lld%lld%lld%lld",&k,&a,&e,&c,&t1,&t2,&t);

	char arr[1000];
	char brr;
	ll crr;

	for(ll i=0;i<k;i++)
	{ 
		scanf("%s %c %lld",arr,&brr,&crr);
		strcpy(musician[i].name, arr);
		musician[i].instrument = brr;
		musician[i].arrival_time = crr;
		// printf("%s %c %lld", musician[i].name,musician[i].instrument,musician[i].arrival_time);
	}

	for(ll i=0;i<(a+e);i++)
	{
		stage[i].id = i+1;
		stage[i].empty = 0;
		stage[i].sings = 0;
		if(i<a)
			stage[i].type = 'a';
		else
			stage[i].type = 'e';
		pthread_mutex_init(&(stage[i].mutex), NULL);
		sem_init(&stage[i].perform,0,0);
	}

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += t;

	sem_init(&asem,0,a);
	sem_init(&esem,0,e);
	sem_init(&bsem,0,a+e);
	sem_init(&ssem,0,a+e);
	sem_init(&csem,0,c);

	for(ll i=0;i<k;i++)
	{
		musician[i].id = i+1;
		if(musician[i].instrument=='v')
			pthread_create(&musician[i].thread, NULL, Acoustic, &musician[i]);
		else if(musician[i].instrument=='b')
			pthread_create(&musician[i].thread, NULL, Electric, &musician[i]);
		else if(musician[i].instrument=='p' || musician[i].instrument=='g')
			pthread_create(&musician[i].thread, NULL, Both, &musician[i]);
		else
			pthread_create(&musician[i].thread, NULL, Singer, &musician[i]);
	}
	for(ll i=0;i<k;i++)
		pthread_join(musician[i].thread,NULL);

	printf("\033[0;31mFINISHED\n");

	sem_destroy(&asem);
	sem_destroy(&esem);
	sem_destroy(&bsem);
	sem_destroy(&ssem);
	sem_destroy(&csem);

	for(ll i=0;i<a+e;i++)
	{
		pthread_mutex_destroy(&(stage[i].mutex));
		sem_destroy(&stage[i].perform);
	}

	return 0;
}
