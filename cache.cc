// This is the code

#include <stdio.h>
#include <assert.h>
#include <string>
#include <inttypes.h>
#include <cstring>
#include <fstream>
#include <stdint.h>
#include <bitset>
#include <math.h>
#include <iostream>
#include <new>

using namespace std;

void decTobinary(int n);
void CacheRunCycle(char **argv);
void retire(int down, int left);
void direct();
void full();
void cacheinit();
void getcacheinfo();
int isValid();
void CacheIdxAndTag();
void RandomGenerate();

int ctable [512][512];
int key [512][512];

const char *ass_type[] = {"direct", "full"};
struct bits
{
  int valid [512][512];
} bit;

struct policies
{
  char *replacement;
  char *associativity;
  int   assnumber;
} policy;

struct cycle
{
  int idx;
  int tag;
  int max_idx;
  int number;
  int miss;
  int hit;
  long size;
  int blocksize;
  int linesize;
  int setNumberIndex;
  int NumberBlocks;
  int indexbit;
  int offset;
  float miss_rate;
  int setNumBlocks;
  int washit;
  int wasmiss;
  char binary[11];
} cache;

int main(int argc, char *argv[])
{
  const char *parameters[5] = {"-h", "-s", "-a", "-b", "-r"};
  if (argc < 2)
  {
    printf("In order to run the program you must add a tracefile\n");
  }

  else
  {
    for (int i = 0; i < argc; i++)
    {
      for (int j = 0; j < 5; j++)
      {
        if (strcmp(argv[i], parameters[j]) == 0)
        {
          switch (j)
          {
            case 0:
            puts("Cache simulator syntax");
            puts("./cache tracefile [-option] [option]");
            puts("------------------------------------");
            puts("Options");
            puts("-h Help");
            puts("-s Changes the cache size");
            puts("-a Changes the associativity (direct, full)");
            puts("-b Changes the block size (max of 1024)");
            puts("-r changes the replacement policy (LRU, RR)");
            break;
            case 1: cache.size = strtol(argv[i + 1], NULL, 10);
            break;
            case 2: policy.associativity = argv[i + 1];
            break;
            case 3: cache.blocksize = strtol(argv[i + 1], NULL, 10);
            break;
            case 4: policy.replacement = argv[i + 1];
            break;

          }
        }
      }
    }

    CacheRunCycle(argv);
  }
}

void decTobinary(int n)
{
// array to store cache.binary cache.number
  int binaryNum[32];

// counter for cache.binary array
  int i = 0;
  while (n > 0) {

  // storing remainder in cache.binary array
    binaryNum[i] = n % 2;
    n = n / 2;
    i++;
  }

// printing cache.binary array in reverse order
  for (int j = i - 1; j >= 0; j--)
    cout << binaryNum[j];
}

void CacheRunCycle(char **argv)
{
  ifstream trace;
  trace.open(argv[1]);
  int totalcycles = 0;
  cache.washit = 0;
  cache.wasmiss = 0;
  int count;
  cacheinit();

  while (trace >> cache.binary)
    { cache.number = (int)strtoumax(cache.binary, NULL, 16);

      CacheIdxAndTag();

      if (strcmp(policy.associativity, ass_type[0]) == 0)
      {
        if ( isValid() == 0 && count <= cache.NumberBlocks)
        {
          cache.miss++;
          cache.wasmiss++;
        }
        else
        {
          direct();
        }
      }
      else if (strcmp(policy.associativity, ass_type[1]) == 0)
      {
        if ( isValid() == 0 && count <= (cache.NumberBlocks*cache.setNumBlocks))
        {
          cache.miss++;
          cache.wasmiss++;
        }
        else
        {
          full();
        }
      }
      count++;
     // retire();
      totalcycles++;
    }

    cache.miss_rate = (float)cache.hit / cache.miss * 100;
    printf("miss rate is %.6f\n", cache.miss_rate);
    printf("hits %d\n", cache.hit);
    printf("misses %d\n", cache.miss);
    printf("total cycles %d\n", totalcycles);
    printf("replacement policy %s\n", policy.replacement);
    printf("associativity policy %s\n", policy.associativity);
  }

  void direct()
  {

    if (cache.tag == (ctable[cache.idx][0]))
    {
      cache.hit++;
      cache.washit++;
    }
    else
    {
      cache.miss++;
      cache.wasmiss++;

    }
  }

  void full()
  { 
    cache.max_idx = abs(~(cache.idx & 0) << cache.indexbit);
    for (int i = 0; i < cache.max_idx; i++)
    {
      for (int Blocks = 0; Blocks < cache.setNumBlocks; Blocks++)
      {
            if (ctable[i][Blocks] == cache.tag)
            { 
              cache.hit++;
              cache.washit++;
              retire(i,Blocks);
              return;
            }
            }
          }
                cache.miss++;
                cache.wasmiss++;
                retire(0,0);
                return;
            }
      

  void retire(int down, int left)
  {
    const char *policies [] = {"LRU", "RR"};
    if (policy.replacement != NULL)
    {
      if (strcmp(policy.replacement, policies[0]) == 0)
      {
        if (strcmp(policy.associativity, ass_type[0]) == 0)
        {
          if(cache.wasmiss == 1)
          {
            ctable[cache.idx][0] = cache.tag; 
          }
        }
        else if (strcmp(policy.associativity, ass_type[1]) == 0)
        {
          if (cache.washit == 1)
          {
            key[down][left] += 1;
          }
          else if (cache.wasmiss == 1)
          {
           // printf("reaches here");
            int leastused;
            leastused = key[0][0];
            int k = 0, l = 0;
            for (int i = 0; i < cache.max_idx; i++)
            {
              for (int j = 0; j < cache.NumberBlocks; j++)
              {
                if (key[i][j] < leastused)
                {
                  leastused = key[i][j];
                  k = i;
                  l = j;
                }
              }
            }

            ctable[k][l] = cache.tag;
            key[k][l] = 0;
          }
        }
      }

      if (strcmp(policy.replacement, policies[1]) == 0)
      {
        if (strcmp(policy.associativity, ass_type[0]) == 0)
        {
          if(cache.wasmiss > 0)
          {
            ctable[cache.idx][0] = cache.tag; 
          }
        }
        if (strcmp(policy.associativity, ass_type[1]) == 0)
        {
         if(cache.wasmiss > 0)
         {
           RandomGenerate();
         }
       }
     }
   }
 }

 void cacheinit()
 {
  cache.linesize = cache.blocksize / 32;
  cache.setNumBlocks = log2(cache.linesize);
  cache.NumberBlocks = cache.size / 32;
  cache.setNumberIndex = cache.NumberBlocks;  // fully associative =  NumberBlocks/
  cache.indexbit = log2(cache.setNumberIndex);


      printf("Cache line size %d\n", cache.linesize);
      printf("Cache setNumBlocks %d\n", cache.setNumBlocks);
      printf("Cache NumberBlocks %d\n", cache.NumberBlocks);
      printf("Cache setNumberIndex %d\n", cache.setNumberIndex);
      printf("bits for set index %d\n", cache.indexbit);
      printf("\n\n");

 }

    void CacheIdxAndTag()
    {
        cache.idx = (cache.number >> 2) & (cache.setNumberIndex - 1);
        cache.tag = cache.number >> (cache.indexbit + 2);
    }


    int isValid()
    {

      if (strcmp(policy.associativity, ass_type[0]) == 0)
      {
        if (bit.valid[cache.idx][0] == 0)
        {
          ctable[cache.idx][0] = cache.tag;
          bit.valid[cache.idx][0] = 1;
          return 0;
        }
      }
      else if (strcmp(policy.associativity, ass_type[1]) == 0)
      { 
        cache.max_idx = abs(~(cache.idx & 0) << cache.indexbit);


        for (int i = 0; i < cache.max_idx; i++)
        { 

            for (int Blocks = 0; Blocks < cache.setNumBlocks; Blocks++)
             { 
        
                if (bit.valid[i][Blocks] == 0)
                {  

                  ctable[i][Blocks] = cache.tag;
                  bit.valid[i][Blocks] = 1;
                  return 0;
                }
              }
            }
         }
       
      return 1;
    }

    void RandomGenerate()
    {
      time_t t;
      srand((unsigned int)time(NULL));

      int i, j;
      for (i = 0; i < cache.idx; ++i)
      {
        for (j = 0; j < cache.setNumBlocks; j++)
        {
          cache.idx = rand() % cache.max_idx;
          printf("%d", cache.idx);
          cache.setNumBlocks = rand() % cache.setNumBlocks;
          printf("%d", cache.setNumBlocks);
          ctable[cache.idx][cache.setNumBlocks] = cache.tag;
        }
      }
    }