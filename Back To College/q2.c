#define _POSIX_C_SOURCE 199309L //required for clock
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

typedef long long int ll;
typedef long double ld;

ld *prob;
ll n,m,o,s;

typedef struct company
{
	ll id;
	ll vaccines;
	ll batches_num;
	pthread_t thread;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
}company;

typedef struct vacczone
{
	ll id;
	ll vaccines;
	ll free;
	ll slot;
	ld probability;
	ll company;
	pthread_t thread;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
}zone;

typedef struct student
{
	ll id;
	ll round;
	ll test;
	ld probability;
	pthread_t thread;
	pthread_mutex_t mutex;
}student;

ll randomno(ll l,ll h)
{
	ll num;
	num = (rand()%(h-l+1))+l;
	return num;
}

ll randomprob(ld prob)
{
	ld r = (ld)rand() / RAND_MAX;
	return r > prob;
}

ll min(ll a, ll b)
{
	if(a<b)
		return a;
	else
		return b;
}

company Company[1000];
zone Zone[1000];
student Student[1000];

void *phar_company(void *inp)
{
	company* comp = (company*) inp;
	// printf("1\n");
	while(1)
	{
		ll w = randomno(2,5);
		ll r = randomno(1,5);
		ll p = randomno(10,20);
		comp->vaccines = p;
		comp->batches_num = r;
		printf("\033[0;33mPharmaceutical Company %lld is preparing %lld batches of Vaccines which have success probability %Lf\n",comp->id,r,prob[comp->id-1]);
		printf("\033[0m");
		sleep(w);
		printf("\033[0;33mPharmaceutical Company %lld has prepared %lld batches of Vaccines which have success probability %Lf\n",comp->id,r,prob[comp->id-1]);
		printf("\033[0m");
		pthread_mutex_lock(&(comp->mutex));
		while(1)
		{
			ll sum=0;
			pthread_cond_wait(&(comp->cond),&(comp->mutex));
			for(ll i=0;i<m;i++)
			{
				if(Zone[i].company==comp->id)
				{
					sum+=Zone[i].vaccines;
				}
			}
			if(sum==0)
			{
				printf("\033[0;31mAll the vaccines prepared by Pharmaceutical Company %lld are emptied. Resuming Production now\n", comp->id);
				pthread_cond_signal(&(comp->cond));
				break;
			}
		}
		pthread_mutex_unlock(&(comp->mutex));
		pthread_mutex_lock(&(comp->mutex));
		if(o<=0)
		{
			for(ll i=0;i<n;i++)
				pthread_mutex_unlock(&(Company[i].mutex));
			for(ll i=0;i<m;i++)
				pthread_mutex_unlock(&(Zone[i].mutex));
			for(ll i=0;i<s;i++)
				pthread_mutex_unlock(&(Student[i].mutex));
			break;
		}
		pthread_mutex_unlock(&(comp->mutex));
	}
}

void *vacc_zone(void *inp)
{
	zone *vz = (zone*)inp;
	while(1)
	{
		if(o<=0)
		{
			for(ll i=0;i<n;i++)
				pthread_mutex_unlock(&(Company[i].mutex));
			for(ll i=0;i<m;i++)
				pthread_mutex_unlock(&(Zone[i].mutex));
			for(ll i=0;i<s;i++)
				pthread_mutex_unlock(&(Student[i].mutex));
			break;
		}
		for(ll i=0;i<n;i++)
		{
			pthread_mutex_lock(&(Company[i].mutex));
			if(Company[i].batches_num==0)
			{
				// printf("\033[0;31mAll the vaccines prepared by Pharmaceutical Company %lld are emptied. Resuming Production now\n", Company[i].id);
				pthread_cond_signal(&(Company[i].cond));
			}
			else
			{
				printf("\033[0;36mPharmaceutical Company %lld is delivering vaccine batch to Vaccination Zone %lld which has success probability %Lf\n",Company[i].id,vz->id,prob[i]);
				printf("\033[0m");
				Company[i].batches_num--;
				vz->vaccines = Company[i].vaccines;
				vz->probability = prob[i];
				vz->company = Company[i].id;
				sleep(2);
				printf("\033[0;36mPharmaceutical Company %lld has delivered vaccines to Vaccination Zone %lld, resuming vaccinations now\n",Company[i].id,vz->id);
				printf("\033[0m");
				if(Company[i].batches_num==0)
				{
					// printf("\033[0;31mAll the vaccines prepared by Pharmaceutical Company %lld are emptied. Resuming Production now\n", Company[i].id);
					pthread_cond_signal(&(Company[i].cond));
				}
				pthread_mutex_unlock(&(Company[i].mutex));
				break;
			}
			pthread_cond_signal(&(Company[i].cond));
			pthread_mutex_unlock(&(Company[i].mutex));
		}
		pthread_mutex_lock(&(vz->mutex));
		printf("\033[0;32mVaccination Zone %lld is entering the Vaccination Phase\n",vz->id);
		printf("\033[0m");
		if(o<=0)
		{
			pthread_mutex_unlock(&(vz->mutex));
			for(ll i=0;i<n;i++)
				pthread_mutex_unlock(&(Company[i].mutex));
			for(ll i=0;i<m;i++)
				pthread_mutex_unlock(&(Zone[i].mutex));
			for(ll i=0;i<s;i++)
				pthread_mutex_unlock(&(Student[i].mutex));
			break;
		}
		pthread_mutex_unlock(&(vz->mutex));
		pthread_mutex_lock(&(vz->mutex));
		while(vz->vaccines>0)
		{
			vz->free = 1;
			ll given = 8;
			vz->slot = min(min(given,vz->vaccines),o);
			if(o==0)
			{
				for(ll i=0;i<n;i++)
					pthread_mutex_unlock(&(Company[i].mutex));
				for(ll i=0;i<m;i++)
					pthread_mutex_unlock(&(Zone[i].mutex));
				for(ll i=0;i<s;i++)
					pthread_mutex_unlock(&(Student[i].mutex));
				break;
			}
			printf("\033[0;32mVaccination Zone %lld is ready to vaccinate with %lld slots\n", vz->id,vz->slot);
			printf("\033[0m");
			pthread_cond_wait(&(vz->cond),&(vz->mutex));
			pthread_mutex_unlock(&(vz->mutex));
		}
		if(vz->vaccines<=0)
		{
			printf("\033[0;32mVaccination Zone %lld has ran out of vaccines\n", vz->id);
			printf("\033[0m");
			pthread_mutex_unlock(&(vz->mutex));
			continue;
		}
	}
}

void *student_slot(void *inp)
{
	ll time;
	student * sd = (student*)inp;
	while(1)
	{
		if(o<=0)
		{
			for(ll i=0;i<n;i++)
				pthread_mutex_unlock(&(Company[i].mutex));
			for(ll i=0;i<m;i++)
				pthread_mutex_unlock(&(Zone[i].mutex));
			for(ll i=0;i<s;i++)
				pthread_mutex_unlock(&(Student[i].mutex));
			break;
		}
		time = randomno(1,10);
		printf("\033[0;35mStudent %lld has arrived for %lld round of Vaccination\n", sd->id, sd->round);
		printf("\033[0m");
		sleep(time);
		printf("\033[0;35mStudent %lld is waiting to be allocated a slot on a Vaccination Zone\n", sd->id);
		printf("\033[0m");
		// ?ll slot_provided = 0;
		for(ll i=0;i<m;i++)
		{
			pthread_mutex_lock(&(Zone[i].mutex));
			if(Zone[i].slot != 0)
			{
				Zone[i].slot--;
				Zone[i].vaccines--;
				printf("\033[0;35mStudent %lld assigned a slot on the Vaccination Zone %lld and waiting to be Vaccinated\n", sd->id, Zone[i].id);
				printf("\033[0m");
				sleep(1);
				printf("\033[0;35mStudent %lld on Vaccination Zone %lld has been vaccinated which has a success probability %Lf\n", sd->id, Zone[i].id, Zone[i].probability);
				printf("\033[0m");
				sd->probability = Zone[i].probability;
				if(Zone[i].slot==0)
					pthread_cond_signal(&(Zone[i].cond));
				pthread_mutex_unlock(&(Zone[i].mutex));
				break;
			}
			pthread_cond_signal(&(Zone[i].cond));
			pthread_mutex_unlock(&(Zone[i].mutex));
		}
		pthread_mutex_lock(&(sd->mutex));
		sd->test = randomprob(sd->probability);
		if(sd->test==1)
		{
			o--;
			printf("\033[0;35mStudent %lld tested positive for antibodies\n", sd->id);
			printf("\033[0m");
			pthread_mutex_unlock(&(sd->mutex));
			break;
		}
		else
		{
			if(sd->round==3)
			{
				printf("\033[0;31mStudent %lld tested negative for antibodies 3 times, now it's time to go home :(\n", sd->id);
				printf("\033[0m");
				pthread_mutex_unlock(&(sd->mutex));
				break;
			}
			sd->round++;
			printf("\033[0;35mStudent %lld tested negative for antibodies\n", sd->id);
			printf("\033[0m");
		}
		pthread_mutex_unlock(&(sd->mutex));
	}
	return NULL;
}

int main()
{
	srand(time(0));

	//---------------------------------------------------------------------------------
	//                             TAKING INPUTS
	//---------------------------------------------------------------------------------
	printf("Enter the no. of Companies, Vaccination Zones, Students\n");
	scanf("%lld%lld%lld",&n,&m,&o);

	printf("Enter success rate of each Vaccination\n");
	prob = malloc(sizeof(ld)*(n+1));
	for(ll i=0;i<n;i++)
	{
		scanf("%Lf",&prob[i]);
		//printf("%Lf\n",prob[i]);
	}
	s = o;
	if(n>0 && m>0 && s>0)
	{
		//---------------------------------------------------------------------------------
		//                             INITIATING MUTEX
		//--------------------------------------------------------------------------------- 
		for(ll i=0;i<n;i++)
		{
			Company[i].id = i+1;
			pthread_mutex_init(&(Company[i].mutex), NULL);
			pthread_create(&Company[i].thread, NULL, phar_company, &Company[i]);
		}
		sleep(7);
		for(ll i=0;i<m;i++)
		{
			Zone[i].id = i+1;
			pthread_mutex_init(&(Zone[i].mutex), NULL);
			pthread_create(&Zone[i].thread, NULL, vacc_zone, &Zone[i]);
		}
		sleep(10);
		for(ll i=0;i<s;i++)
		{
			Student[i].id = i+1;
			Student[i].round = 1;
			pthread_mutex_init(&(Student[i].mutex), NULL);
			pthread_create(&Student[i].thread, NULL, student_slot, &Student[i]);
		}
		for(ll i=0;i<s;i++)
		{
			pthread_join(Student[i].thread,NULL);
		}
	}
	// //---------------------------------------------------------------------------------
	// //                             DESTROY MUTEX
	// //---------------------------------------------------------------------------------
	printf("\033[0;31mSTIMULATION OVER\n");
	for(ll i=0;i<n;i++)
		pthread_mutex_destroy(&(Company[i].mutex));
	for(ll i=0;i<m;i++)
		pthread_mutex_destroy(&(Zone[i].mutex));
	for(ll i=0;i<s;i++)
		pthread_mutex_destroy(&(Student[i].mutex));

	return 0;
}
