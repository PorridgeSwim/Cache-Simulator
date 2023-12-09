//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.hpp"

//
// TODO:Student Information
//
const char *studentName = "You Zhou";
const char *studentID   = "A59024699";
const char *email       = "yoz023@ucsd.edu";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;      // Number of sets in the I$
uint32_t icacheAssoc;     // Associativity of the I$
uint32_t icacheBlocksize; // Blocksize of the I$
uint32_t icacheHitTime;   // Hit Time of the I$

uint32_t dcacheSets;      // Number of sets in the D$
uint32_t dcacheAssoc;     // Associativity of the D$
uint32_t dcacheBlocksize; // Blocksize of the D$
uint32_t dcacheHitTime;   // Hit Time of the D$

uint32_t l2cacheSets;     // Number of sets in the L2$
uint32_t l2cacheAssoc;    // Associativity of the L2$
uint32_t l2cacheBlocksize;// Blocksize of the L2$
uint32_t l2cacheHitTime;  // Hit Time of the L2$
uint32_t inclusive;       // Indicates if the L2 is inclusive

uint32_t prefetch;        // Indicate if prefetching is enabled
uint32_t memspeed;        // Latency of Main Memory

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

uint64_t compulsory_miss;  // Compulsory misses on all caches
uint64_t other_miss;       // Other misses (Conflict / Capacity miss) on all caches

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

//
//TODO: Add your Cache data structures here
//
uint32_t **icache;
uint32_t **icacheLRU;
uint32_t **icacheValid;
uint32_t icacheCounter = 0;
uint32_t icacheBlockoffset;
uint32_t **dcache;
uint32_t **dcacheLRU;
uint32_t **dcacheValid;
uint32_t dcacheCounter = 0;
uint32_t dcacheBlockoffset;
uint32_t **l2cache;
uint32_t **l2cacheLRU;
uint32_t **l2cacheValid;
uint32_t l2cacheCounter = 0;
uint32_t l2cacheBlockoffset;
uint32_t icacheReadAddr = 0;
uint32_t icacheWriteAddr = 0;
uint32_t icacheReadPc = 0;
uint32_t icacheWritePc = 0;
uint32_t dcacheReadAddr = 0;
uint32_t dcacheWriteAddr = 0;
uint32_t dcacheReadPc = 0;
uint32_t dcacheWritePc = 0;
uint32_t *icacheReadTable;
uint32_t *icacheWriteTable;
uint32_t *dcacheReadTable;
uint32_t *dcacheWriteTable;
uint32_t SHTentries = 1 << 15;


uint32_t log2(uint32_t num)
{
  uint32_t i = 1;
  while (num >> i > 1) {
    i++;
  }
  return i;
}
//------------------------------------//
//          Cache Functions           //
//------------------------------------//

// Initialize the Cache Hierarchy
//
void
init_cache()
{
  int i, j;
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;
  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;
  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;

  compulsory_miss = 0;
  other_miss = 0;
  
  //
  //TODO: Initialize Cache Simulator Data Structures
  //
  icache = (uint32_t **)malloc(icacheSets * sizeof(uint32_t *));
  icacheLRU = (uint32_t **)malloc(icacheSets * sizeof(uint32_t *));
  icacheValid = (uint32_t **)malloc(icacheSets * sizeof(uint32_t *));
  for (i = 0; i < icacheSets; i++) {
    icache[i] = (uint32_t *)malloc(icacheAssoc * sizeof(uint32_t));
    icacheLRU[i] = (uint32_t *)malloc(icacheAssoc * sizeof(uint32_t));
    icacheValid[i] = (uint32_t *)malloc(icacheAssoc * sizeof(uint32_t));
    for (j = 0; j < icacheAssoc; j++) {
      icacheLRU[i][j] = 0;
      icacheValid[i][j] = 0;
    }
  }
  icacheBlockoffset = log2(icacheBlocksize);

  dcache = (uint32_t **)malloc(dcacheSets * sizeof(uint32_t *));
  dcacheLRU = (uint32_t **)malloc(dcacheSets * sizeof(uint32_t *));
  dcacheValid = (uint32_t **)malloc(dcacheSets * sizeof(uint32_t *));
  for (i = 0; i < dcacheSets; i++) {
    dcache[i] = (uint32_t *)malloc(dcacheAssoc * sizeof(uint32_t));
    dcacheLRU[i] = (uint32_t *)malloc(dcacheAssoc * sizeof(uint32_t));
    dcacheValid[i] = (uint32_t *)malloc(dcacheAssoc * sizeof(uint32_t));
    for (j = 0; j < dcacheAssoc; j++) {
      dcacheLRU[i][j] = 0;
      dcacheValid[i][j] = 0;
    }
  }
  dcacheBlockoffset = log2(dcacheBlocksize);

  l2cache = (uint32_t **)malloc(l2cacheSets * sizeof(uint32_t *));
  l2cacheLRU = (uint32_t **)malloc(l2cacheSets * sizeof(uint32_t *));
  l2cacheValid = (uint32_t **)malloc(l2cacheSets * sizeof(uint32_t *));
  for (i = 0; i < l2cacheSets; i++) {
    l2cache[i] = (uint32_t *)malloc(l2cacheAssoc * sizeof(uint32_t));
    l2cacheLRU[i] = (uint32_t *)malloc(l2cacheAssoc * sizeof(uint32_t));
    l2cacheValid[i] = (uint32_t *)malloc(l2cacheAssoc * sizeof(uint32_t));
    for (j = 0; j < l2cacheAssoc; j++) {
      l2cacheLRU[i][j] = 0;
      l2cacheValid[i][j] = 0;
    }
  }
  l2cacheBlockoffset = log2(l2cacheBlocksize);

  if (prefetch) {
    icacheReadTable = (uint32_t *)malloc(SHTentries * sizeof(uint32_t));
    icacheWriteTable = (uint32_t *)malloc(SHTentries * sizeof(uint32_t));
    dcacheReadTable = (uint32_t *)malloc(SHTentries * sizeof(uint32_t));
    dcacheWriteTable = (uint32_t *)malloc(SHTentries * sizeof(uint32_t));
    for (i = 0; i < SHTentries; i++) {
      icacheReadTable[i] = icacheBlocksize;
      icacheWriteTable[i] = icacheBlocksize;
      dcacheReadTable[i] = dcacheBlocksize;
      dcacheWriteTable[i] = dcacheBlocksize;
    }
  }
}

// Clean Up the Cache Hierarchy
//
void
clean_cache()
{
  //
  //TODO: Clean Up Cache Simulator Data Structures
  //
}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
icache_access(uint32_t addr)
{
  //
  //TODO: Implement I$
  //
  int i;
  uint32_t index = (addr >> icacheBlockoffset) & (icacheSets - 1);
  uint32_t total_offset = icacheBlockoffset + log2(icacheSets);
  uint32_t tag = addr >> total_offset;
  bool isEmpty = false;
  uint32_t empty_index = 0;
  uint32_t min_counter = icacheCounter + 1;
  uint32_t LRU_index = 0;
  uint32_t penalty = 0;

  icacheRefs++;
  icacheCounter++;


  for (i = 0; i < icacheAssoc; i++) {
    if (icacheValid[index][i] == 1 && (icache[index][i] >> total_offset) == tag) {
      icacheLRU[index][i] = icacheCounter;
      return icacheHitTime;
    }
    if (icacheValid[index][i] == 0) {
      isEmpty = true;
      empty_index = i;
    }
    if (icacheLRU[index][i] < min_counter) {
      min_counter = icacheLRU[index][i];
      LRU_index = i;
    }
  }

  icacheMisses++;
  penalty = l2cache_access(addr);
  icachePenalties += penalty;

  if (isEmpty) {
    icache[index][empty_index] = addr;
    icacheValid[index][empty_index] = 1;
    icacheLRU[index][empty_index] = icacheCounter;
  } else {
    icache[index][LRU_index] = addr;
    icacheLRU[index][LRU_index] = icacheCounter;
  }

  return icacheHitTime + penalty;
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
  //
  //TODO: Implement D$
  //
  int i;
  uint32_t index = (addr >> dcacheBlockoffset) & (dcacheSets - 1);
  uint32_t total_offset = dcacheBlockoffset + log2(dcacheSets);
  uint32_t tag = addr >> total_offset;
  bool isEmpty = false;
  uint32_t empty_index = 0;
  uint32_t min_counter = dcacheCounter + 1;
  uint32_t LRU_index = 0;
  uint32_t penalty = 0;

  dcacheRefs++;
  dcacheCounter++;
  for (i = 0; i < dcacheAssoc; i++) {
    if (dcacheValid[index][i] == 1 && (dcache[index][i] >> total_offset) == tag) {
      dcacheLRU[index][i] = dcacheCounter;
      return dcacheHitTime;
    }
    if (dcacheValid[index][i] == 0) {
      isEmpty = true;
      empty_index = i;
    }
    if (dcacheLRU[index][i] < min_counter) {
      min_counter = dcacheLRU[index][i];
      LRU_index = i;
    }
  }

  dcacheMisses++;
  penalty = l2cache_access(addr);
  dcachePenalties += penalty;

  if (isEmpty) {
    dcache[index][empty_index] = addr;
    dcacheValid[index][empty_index] = 1;
    dcacheLRU[index][empty_index] = dcacheCounter;
  } else {
    dcache[index][LRU_index] = addr;
    dcacheLRU[index][LRU_index] = dcacheCounter;
  }
  
  return dcacheHitTime + penalty;
}


// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
// [access time, ]
uint32_t
l2cache_access(uint32_t addr)
{
  //
  //TODO: Implement L2$
  //
  int i;
  uint32_t index = (addr >> l2cacheBlockoffset) & (l2cacheSets - 1);
  uint32_t total_offset = l2cacheBlockoffset + log2(l2cacheSets);
  uint32_t tag = addr >> total_offset;
  bool isEmpty = false;
  uint32_t empty_index = 0;
  uint32_t min_counter = icacheCounter + 1;
  uint32_t LRU_index = 0;
  uint32_t penalty = 0;

  l2cacheRefs++;
  l2cacheCounter++;
  for (i = 0; i < l2cacheAssoc; i++) {
    if (l2cacheValid[index][i] == 1 && (l2cache[index][i] >> total_offset) == tag) {
      l2cacheLRU[index][i] = l2cacheCounter;
      return l2cacheHitTime;
    }
    if (l2cacheValid[index][i] == 0) {
      isEmpty = true;
      empty_index = i;
    }
    if (l2cacheLRU[index][i] < min_counter) {
      min_counter = l2cacheLRU[index][i];
      LRU_index = i;
    }
  }

  l2cacheMisses++;
  l2cachePenalties += memspeed;

  if (isEmpty) {
    l2cache[index][empty_index] = addr;
    l2cacheValid[index][empty_index] = 1;
    l2cacheLRU[index][empty_index] = l2cacheCounter;
  } else {
    l2cache[index][LRU_index] = addr;
    l2cacheLRU[index][LRU_index] = l2cacheCounter;
  }

  return l2cacheHitTime + memspeed;
}

// Predict an address to prefetch on icache with the information of last icache access:
// 'pc':     Program Counter of the instruction of last icache access
// 'addr':   Accessed Address of last icache access
// 'r_or_w': Read/Write of last icache access
uint32_t
icache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w)
{
  //
  //TODO: Implement a better prefetching strategy
  //
  if (r_or_w == 'R') {
    icacheReadTable[icacheReadPc & (SHTentries - 1)] = addr - icacheReadAddr;
    icacheReadAddr = addr;
    icacheReadPc = pc;
    return addr + icacheReadTable[(pc) & (SHTentries - 1)];
  } else {
    icacheWriteTable[icacheWritePc & (SHTentries - 1)] = addr - icacheWriteAddr;
    icacheWriteAddr = addr;
    icacheWritePc = pc;
    return addr + icacheWriteTable[pc & (SHTentries - 1)];
  }
}

// Predict an address to prefetch on dcache with the information of last dcache access:
// 'pc':     Program Counter of the instruction of last dcache access
// 'addr':   Accessed Address of last dcache access
// 'r_or_w': Read/Write of last dcache access
uint32_t
dcache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w)
{
  //
  //TODO: Implement a better prefetching strategy
  //
  if (r_or_w == 'R') {
    dcacheReadTable[(dcacheReadAddr ^ dcacheReadPc) & (SHTentries - 1)] = addr - dcacheReadAddr;
    dcacheReadAddr = addr;
    dcacheReadPc = pc;
    return addr + dcacheReadTable[(addr ^ pc) & (SHTentries - 1)];
  } else {
    dcacheWriteTable[(dcacheWriteAddr ^ dcacheWritePc) & (SHTentries - 1)] = addr - dcacheWriteAddr;
    dcacheWriteAddr = addr;
    dcacheWritePc = pc;
    return addr + dcacheWriteTable[(addr ^ pc) & (SHTentries - 1)];
  }
}

// Perform a prefetch operation to I$ for the address 'addr'
void
icache_prefetch(uint32_t addr)
{
  //
  //TODO: Implement I$ prefetch operation
  //
  int i;
  uint32_t index = (addr >> icacheBlockoffset) & (icacheSets - 1);
  uint32_t total_offset = icacheBlockoffset + log2(icacheSets);
  uint32_t tag = addr >> total_offset;
  bool isEmpty = false;
  uint32_t empty_index = 0;
  uint32_t min_counter = icacheCounter + 1;
  uint32_t LRU_index = 0;

  icacheCounter++;

  for (i = 0; i < icacheAssoc; i++) {
    if (icacheValid[index][i] == 1 && (icache[index][i] >> total_offset) == tag) {
      icacheLRU[index][i] = icacheCounter;
      return;
    }
    if (icacheValid[index][i] == 0) {
      isEmpty = true;
      empty_index = i;
    }
    if (icacheLRU[index][i] < min_counter) {
      min_counter = icacheLRU[index][i];
      LRU_index = i;
    }
  }

  if (isEmpty) {
    icache[index][empty_index] = addr;
    icacheValid[index][empty_index] = 1;
    icacheLRU[index][empty_index] = icacheCounter;
  } else {
    icache[index][LRU_index] = addr;
    icacheLRU[index][LRU_index] = icacheCounter;
  }
}

// Perform a prefetch operation to D$ for the address 'addr'
void
dcache_prefetch(uint32_t addr)
{
  //
  //TODO: Implement D$ prefetch operation
  //
  int i;
  uint32_t index = (addr >> dcacheBlockoffset) & (dcacheSets - 1);
  uint32_t total_offset = dcacheBlockoffset + log2(dcacheSets);
  uint32_t tag = addr >> total_offset;
  bool isEmpty = false;
  uint32_t empty_index = 0;
  uint32_t min_counter = dcacheCounter + 1;
  uint32_t LRU_index = 0;

  dcacheCounter++;
  for (i = 0; i < dcacheAssoc; i++) {
    if (dcacheValid[index][i] == 1 && (dcache[index][i] >> total_offset) == tag) {
      dcacheLRU[index][i] = dcacheCounter;
      return;
    }
    if (dcacheValid[index][i] == 0) {
      isEmpty = true;
      empty_index = i;
    }
    if (dcacheLRU[index][i] < min_counter) {
      min_counter = dcacheLRU[index][i];
      LRU_index = i;
    }
  }

  if (isEmpty) {
    dcache[index][empty_index] = addr;
    dcacheValid[index][empty_index] = 1;
    dcacheLRU[index][empty_index] = dcacheCounter;
  } else {
    dcache[index][LRU_index] = addr;
    dcacheLRU[index][LRU_index] = dcacheCounter;
  }
}
