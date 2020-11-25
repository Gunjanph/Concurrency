#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
typedef long long int ll;
typedef long double ld;

typedef struct arg{
     int l;
     int r;
     int* arr;
}arg;

void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

int * shareMem(size_t size)
{
     key_t mem_key = IPC_PRIVATE;
     int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
     return (int*)shmat(shm_id, NULL, 0);
}

void merge(int *arr, ll low, ll mid, ll high)
{
     ll n1,n2;
     int *L = malloc(sizeof(int)*(high-low+1));
     int *R = malloc(sizeof(int)*(high-low+1));
     n1 = mid-low+1;
     n2 = high-mid;

     for(ll i=0;i<n1;i++)
          L[i]=arr[low+i];
     for(ll i=0;i<n2;i++)
          R[i]=arr[mid+1+i];

     ll i=0,j=0,k=low;

     while(i<n1 && j<n2)
     {
          if(L[i]<=R[j])
          {
               arr[k]=L[i];
               i++;
          }
          else
          {
               arr[k]=R[j];
               j++;
          }
          k++;
     }
     while(i<n1)
     {
          arr[k]=L[i];
          i++;
          k++;
     }
     while(j<n2)
     {
          arr[k]=R[j];
          j++;
          k++;
     }
}

void selectionSort(int *arr, ll n)
{
     ll min=0;
     for(ll i=0;i<n-1;i++)
     {
          min = i;
          for(ll j=i+1;j<n;j++)
          {
               if(arr[j]<arr[min])
                    min=j;
          }
          swap(&arr[min],&arr[i]);
     }
}
void normal_mergeSort(int *arr, int low, int high)
{
     ll length;
     length = high-low+1;
     if(length < 5)
          selectionSort(arr+low,length);
     else
     {
          /* pi is partitioning index, arr[p] is now
          at right place */
          int pi = low + (high-low)/2;

          // Separately sort elements before
          // partition and after partition
          normal_mergeSort(arr, low, pi - 1);
          normal_mergeSort(arr, pi + 1, high);
          merge(arr,low,pi,high);
     }
}

void mergeSort(int *arr, ll low, ll high)
{
     ll length;
     length = high-low+1;
     if(length < 5)
     {
          
          selectionSort(arr+low,length);
     }
     else
     {
          /* pi is partitioning index, arr[p] is now
          at right place */
          pid_t pid1,pid2,pi;
          pi = low + (high-low)/2;
          pid1=fork();
          if(pid1<0)
          {
               perror("Left Child Proc. not created");
               _exit(-1);
          }
          else if(pid1==0)
          {
               mergeSort(arr, low, pi);
               _exit(1);
          }
          else
          {
               int status;
               pid2=fork();
               if(pid2<0)
               {
                    perror("Right Child Proc. not created");
                    _exit(-1);
               }
               else if(pid2==0)
               {
                    mergeSort(arr, pi + 1, high);
                    _exit(1);
               }
               else
               {
                    waitpid(pid1, &status, 0);
                    waitpid(pid2, &status, 0);
                    merge(arr,low,pi,high);
               }
          }
          return;
          // Separately sort elements before
          // partition and after partition
     }
}


void *threaded_mergeSort(void* a)
{
     //note that we are passing a struct to the threads for simplicity.
     arg *args = (arg*) a;

     int l = args->l;
     int r = args->r;
     int *arr = args->arr;
     ll length = r-l+1;
     if(l>r) 
          return NULL;
     if(length<5)
          selectionSort(arr+l,length);
     else
     {
     int ind=l+(r-l)/2;
     //sort left half array
     arg a1;
     a1.l = l;
     a1.r = ind;
     a1.arr = arr;
     pthread_t tid1;
     pthread_create(&tid1, NULL, threaded_mergeSort, &a1);

     //sort right half array
     arg a2;
     a2.l = ind+1;
     a2.r = r;
     a2.arr = arr;
     pthread_t tid2;
     pthread_create(&tid2, NULL, threaded_mergeSort, &a2);

     //wait for the two halves to get sorted
     pthread_join(tid1, NULL);
     pthread_join(tid2, NULL);
     merge(arr,l,ind,r);
     }
}

void runSorts(ll n)
{
     struct timespec ts;

     //getting shared memory
     int *arr = shareMem(sizeof(int)*(n+1));
     for(ll i=0;i<n;i++) 
          scanf("%d", arr+i);

     int brr[n+1],crr[n+1];
     for(ll i=0;i<n;i++) 
     {
          brr[i] = arr[i];
          crr[i] = arr[i];
     }

     printf("Running \033[0;36mconcurrent_mergesort\033[0m for n = %lld\n", n);
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     ld st = ts.tv_nsec/(1e9)+ts.tv_sec;

     //multiprocess mergesort
     mergeSort(arr, 0, n-1);
     for(int i=0; i<n; i++)
     {
          printf("%d ",arr[i]);
     }
     printf("\n");
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     ld en = ts.tv_nsec/(1e9)+ts.tv_sec;
     printf("\033[0;33mtime = %Lf\n", en - st);
     printf("\033[0m\n");
     ld t1 = en-st;

     pthread_t tid;
     arg a;
     a.l = 0;
     a.r = n-1;
     a.arr = brr;
     printf("Running \033[0;36mthreaded_concurrent_mergesort\033[0m for n = %lld\n", n);
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     st = ts.tv_nsec/(1e9)+ts.tv_sec;

     //multithreaded mergesort
     pthread_create(&tid, NULL, threaded_mergeSort, &a);
     pthread_join(tid, NULL);
     for(ll i=0; i<n; i++)
     {
          printf("%d ",a.arr[i]);
     }
     printf("\n");
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     en = ts.tv_nsec/(1e9)+ts.tv_sec;
     printf("\033[0;33mtime = %Lf\n", en - st);
     printf("\033[0m\n");
     ld t2 = en-st;

     printf("Running \033[0;36mnormal_mergesort\033[0m for n = %lld\n", n);
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     st = ts.tv_nsec/(1e9)+ts.tv_sec;

     // normal mergesort
     normal_mergeSort(crr, 0, n-1);
     for(ll i=0; i<n; i++)
     {
          printf("%d ",crr[i]);
     }
     printf("\n");
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     en = ts.tv_nsec/(1e9)+ts.tv_sec;
     printf("\033[0;33mtime = %Lf\n", en - st);
     printf("\033[0m\n");
     ld t3 = en - st;

     printf("normal_mergesort ran:\n\t\033[0;31m[ %Lf ]\033[0m times faster than concurrent_mergesort\n\t\033[0;31m[ %Lf ] \033[0mtimes faster than threaded_concurrent_mergesort\n", t1/t3, t2/t3);
     shmdt(arr);
     return;
}

int main()
{

     ll n;
     scanf("%lld", &n);
     runSorts(n);
     return 0;
}