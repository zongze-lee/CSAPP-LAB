#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cachelab.h"

int Hits = 0;
int Misses = 0;
int Evicts = 0;
int verbose = 0;
char t[100];

typedef struct Cache1{
	int s;
	int E;
	int b;
	int** line;
	int** cnt;
}Cache;

void print_help();
void initCache(int argc, char *argv[]);
void useCache();
void freeCache();
void update(unsigned address);

Cache* cache = NULL;

int main(int argc, char *argv[]) {
	initCache(argc, argv);
	useCache();
	freeCache();
    	printSummary(Hits, Misses, Evicts);
	return 0;
}

void print_help() {
    	printf("** A Cache Simulator by Deconx\n");
    	printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    	printf("Options:\n");
    	printf("-h         Print this help message.\n");
    	printf("-v         Optional verbose flag.\n");
    	printf("-s <num>   Number of set index bits.\n");
    	printf("-E <num>   Number of lines per set.\n");
    	printf("-b <num>   Number of block offset bits.\n");
    	printf("-t <file>  Trace file.\n\n\n");
    	printf("Examples:\n");
    	printf("linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    	printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void initCache(int argc, char *argv[]) {
	cache = (Cache*)malloc(sizeof(Cache));
	char opt = 0;
	while (-1 != (opt = getopt(argc, argv, "hvs:E:b:t:"))) {
		switch (opt)
		{
		case 'h':
		    print_help();
		    exit(0);
		case 'v':
		    verbose = 1;
		    break;
		case 's':
		    cache->s = atoi(optarg);
		    break;
		case 'E':
		    cache->E = atoi(optarg);
		    break;
		case 'b':
		    cache->b = atoi(optarg);
		    break;
		case 't':
		    strcpy(t, optarg);
		    break;
		default:
		    print_help();
		    exit(-1);
		}
	}
	
	int i1 = 1 << cache->s;
	int j1 = cache->E;
	
	cache->line = (int**)malloc(sizeof(int*) * i1);
	cache->cnt = (int**)malloc(sizeof(int*) * i1);
	for (int i = 0 ; i < i1 ; i++) {
		cache->line[i] = (int*)malloc(sizeof(int) * j1);
		cache->cnt[i] = (int*)malloc(sizeof(int) * j1);
		for (int j = 0 ; j < j1 ; j++) {
			cache->line[i][j] = 0;
			cache->cnt[i][j] = 0;
		} 
	}
}

void useCache() {
	FILE *fp = fopen(t, "r");
    	if (fp == NULL)
    	{
        	exit(-1);
    	}
	char command;
	unsigned address;
	int size;
    	while (fscanf(fp, " %c %x,%d", &command, &address, &size) > 0) {
        	switch (command) {
        		case 'M':
        			if (verbose) {
					printf("M %x,%d ", address, size); 
				}
            			update(address);
            			update(address);
            			if (verbose) {
					printf("\n"); 
				}
            			break;
        		case 'L':
        			if (verbose) {
					printf("L %x,%d ", address, size); 
				}
            			update(address);
            			if (verbose) {
					printf("\n"); 
				}
            			break;
        		case 'S':
        			if (verbose) {
					printf("S %x,%d ", address, size); 
				}
            			update(address);
            			if (verbose) {
					printf("\n"); 
				}
            			break;
        	}
    	}
    	fclose(fp);
}

void update(unsigned address) {
	unsigned t = (address >> (cache->s + cache->b)) & ~((1 << 31) >> (cache->s + cache->b - 1));
	unsigned si = (address >> cache->b) & ((1 << cache->s) - 1);
	int i = 0;
	
	while ((cache->line[si][i] & (1 << 31)) && (i < (cache->E))) {
		if (!((cache->line[si][i] ^ (1 << 31)) ^ t)) {
			Hits++;
			int j = 0;
			int cnt1 = cache->cnt[si][i];
			while ((cache->line[si][j] & (1 << 31)) && (j < (cache->E))) {
				if (cache->cnt[si][j] < cnt1) {
					cache->cnt[si][j]++;
				} 
				j++;
			}
			cache->cnt[si][i] = 0;
			if (verbose) {
				printf("hit "); 
			}
			return;
		} 
		i++;
	}
	
	Misses++;
	if (verbose) {
		printf("miss "); 
	}
	if (i ^ (cache->E)) {
		cache->line[si][i] = (1 << 31) | t;
		for (int j = 0 ; j < i ; j++) {
			cache->cnt[si][j]++;
		}
	} else {
		Evicts++;
		int cntm = 0;
		int flag = 0;
		for (int j = 0 ; j < i ; j++) {
			if (cache->cnt[si][j] > cntm) {
				cntm = cache->cnt[si][j];
				flag = j;
			}
			cache->cnt[si][j]++;
		}
		cache->line[si][flag] = (1 << 31) | t;
		cache->cnt[si][flag] = 0;
		if (verbose) {
			printf("eviction "); 
		}
	}
	return;
}

void freeCache() {
	for (int i = 0 ; i < cache->s ; i++) {
		free(cache->line[i]);
		free(cache->cnt[i]);
	}
	free(cache->line);
	free(cache);
}
