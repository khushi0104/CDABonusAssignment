#include <stdio.h>
#include <stdlib.h>

#define CACHE_SIZE 32
#define NUM_TRACES 1000

typedef struct {
    unsigned long tag;
    int valid;
    int lruCounter;
} CacheLine;

void directMapped(CacheLine* cache, unsigned long address, int* hits, int* accesses) {
    int index = address % CACHE_SIZE;
    if (cache[index].valid && cache[index].tag == address) {
        (*hits)++;
    } else {
        cache[index].valid = 1;
        cache[index].tag = address;
    }
    (*accesses)++;

}

void setAssociative(CacheLine* cache, int numWays, unsigned long address, int* hits, int* accesses) {
    int setSize = CACHE_SIZE / numWays;
    int setIndex = address % setSize;
    int hit = 0;

    for (int i = 0; i < numWays; i++) {
        int index = setIndex * numWays + i;
        if (cache[index].valid && cache[index].tag == address) {
            hit = 1;
            cache[index].lruCounter = 0;
        } else if (cache[index].valid) {
            cache[index].lruCounter++;
        }
    }

    if (hit) {
        (*hits)++;
    } else {
        int maxCounter = 0;
        int maxIndex = setIndex * numWays;
        for (int i = 1; i < numWays; i++) {
            int index = setIndex * numWays + i;
            if (cache[index].lruCounter > maxCounter) {
                maxCounter = cache[index].lruCounter;
                maxIndex = index;
            }
        }
        cache[maxIndex].valid = 1;
        cache[maxIndex].tag = address;
    }
    (*accesses)++;

    
}

void randomReplacement(CacheLine* cache, unsigned long address, int* hits, int* accesses) {
    int index = rand() % CACHE_SIZE;
    if (cache[index].valid && cache[index].tag == address) {
        (*hits)++;
    } else {
        cache[index].valid = 1;
        cache[index].tag = address;
    }
    (*accesses)++;
}

void simulateCache(int numWays, int useLRU) {
    CacheLine* cache = (CacheLine*)malloc(CACHE_SIZE * sizeof(CacheLine));
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache[i].valid = 0;
        cache[i].lruCounter = 0;
    }

    FILE* file = fopen("traces.txt", "r");
    if (file == NULL) {
        printf("Failed to open traces.txt\n");
        return;
    }

    unsigned long address;
    int hits = 0;
    int accesses = 0;

    while (fscanf(file, "%lx", &address) != EOF) {
        if (numWays == 1) {
            directMapped(cache, address, &hits, &accesses);
        }
        else {
            if (useLRU) {
                setAssociative(cache, numWays, address, &hits, &accesses);
            } 
            else {
                randomReplacement(cache, address, &hits, &accesses);
            }
        }

        if (useLRU) {
            // Update LRU counters
            for (int i = 0; i < CACHE_SIZE; i++) {
                if (cache[i].valid) {
                    cache[i].lruCounter++;
                }
            }
        }

        // Reset LRU counter for the accessed cache line
        if (numWays > 1 && useLRU) {
            int setIndex = (address >> 2) % (CACHE_SIZE / numWays);
            int tag = (address >> (2 + (CACHE_SIZE / numWays))) % (CACHE_SIZE / numWays);
            int lineIndex = setIndex * numWays + tag;
            cache[lineIndex].lruCounter = 0;
        }
        
    }

    fclose(file);

    float hitRate = (float)hits / accesses * 100;
    printf("Number of hits: %d\n", hits);
    printf("Number of total accesses: %d\n", accesses);
    printf("Hit rate: %.2f%%\n", hitRate);

    free(cache);

}

int main() {
    // Test direct-mapped cache
    printf("Direct-Mapped Cache:\n");
    simulateCache(1, 0);

    // Test 2-way set-associative cache with LRU replacement policy
    printf("\n2-Way Set-Associative Cache (LRU):\n");
    simulateCache(2, 1);

    // Test 4-way set-associative cache with LRU replacement policy
    printf("\n4-Way Set-Associative Cache (LRU):\n");
    simulateCache(4, 1);

    // Test fully associative cache with random replacement policy
    printf("\nFully Associative Cache (Random Replacement):\n");
    simulateCache(CACHE_SIZE, 0);

    return 0;
}






