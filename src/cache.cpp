/*Questions:
- check with the prefectch logic 
- check how should we utilize the inclusive variable
*/

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
const char *studentName = "Stuart Boynton - Chengcheng Zhang";
const char *studentID = "A15389925 - A16513041";
const char *email = "sjboynto@ucsd.edu - chz009@ucsd.edu";

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

uint32_t l2cacheSets;      // Number of sets in the L2$
uint32_t l2cacheAssoc;     // Associativity of the L2$
uint32_t l2cacheBlocksize; // Blocksize of the L2$
uint32_t l2cacheHitTime;   // Hit Time of the L2$
uint32_t inclusive;        // Indicates if the L2 is inclusive

uint32_t prefetch; // Indicate if prefetching is enabled

uint32_t memspeed; // Latency of Main Memory

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;      // I$ references
uint64_t icacheMisses;    // I$ misses
uint64_t icachePenalties; // I$ penalties

uint64_t dcacheRefs;      // D$ references
uint64_t dcacheMisses;    // D$ misses
uint64_t dcachePenalties; // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

cacheLine **icache;
cacheLine **dcache;
cacheLine **l2cache;
static uint32_t i_prev_addr;
static uint32_t i_stride;
static int i_count;
static uint32_t d_prev_addr;
static uint32_t d_stride;
static int d_count;

//------------------------------------//
//          Cache Functions           //
//------------------------------------//

// Initialize the Cache Hierarchy
//
void init_cache()
{

  // Initialize cache stats
  icacheRefs = 0;
  icacheMisses = 0;
  icachePenalties = 0;
  dcacheRefs = 0;
  dcacheMisses = 0;
  dcachePenalties = 0;
  l2cacheRefs = 0;
  l2cacheMisses = 0;
  l2cachePenalties = 0;

  //
  // TODO: Initialize Cache Simulator Data Structures
  //

  icache = new cacheLine *[icacheSets];
  for (int i = 0; i < icacheSets; i++)
  {
    icache[i] = new cacheLine[icacheAssoc];
    for (int j = 0; j < icacheAssoc; j++)
    {
      icache[i][j].valid = false;
      icache[i][j].tag = 0;
      icache[i][j].lru = j;
    }
  }

  dcache = new cacheLine *[dcacheSets];
  for (int i = 0; i < dcacheSets; i++)
  {
    dcache[i] = new cacheLine[dcacheAssoc];
    for (int j = 0; j < dcacheAssoc; j++)
    {
      dcache[i][j].valid = false;
      dcache[i][j].tag = 0;
      dcache[i][j].lru = j;
    }
  }

  l2cache = new cacheLine *[l2cacheSets];
  for (int i = 0; i < l2cacheSets; i++)
  {
    l2cache[i] = new cacheLine[l2cacheAssoc];
    for (int j = 0; j < l2cacheAssoc; j++)
    {
      l2cache[i][j].valid = false;
      l2cache[i][j].tag = 0;
      l2cache[i][j].lru = j;
    }
  }
}

// Clean Up the Cache Hierarchy
//
void clean_cache()
{
  
  for (int i = 0; i < icacheSets; i++)
  {
    delete[] icache[i];
  }
  delete icache;

  for (int i = 0; i < dcacheSets; i++)
  {
    delete[] dcache[i];
  }
  delete dcache;

  for (int i = 0; i < l2cacheSets; i++)
  {
    delete[] l2cache[i];
  }
  delete l2cache;
}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
icache_access(uint32_t addr)
{
  int index = (addr / icacheBlocksize) % icacheSets;
  int tag = addr / icacheBlocksize / icacheSets;
  icacheRefs++;

  for (int i = 0; i < icacheAssoc; i++)
  {
    if (icache[index][i].tag == tag && icache[index][i].valid)
    {
      update_lru(i, icache[index], icacheAssoc);
      return icacheHitTime;
    }
  }

  icacheMisses++;
  uint32_t penalty = l2cache_access(addr);
  icachePenalties += penalty;
  uint32_t evictInd = get_lru(icache[index], icacheAssoc);

  icache[index][evictInd].tag = tag;
  icache[index][evictInd].valid = true;
  update_lru(evictInd, icache[index], icacheAssoc);

  return icacheHitTime + penalty;
}

uint32_t get_lru(cacheLine *cacheSet, uint32_t cacheAssoc)
{
  for (int i = 0; i < cacheAssoc; i++)
  {
    if (cacheSet[i].lru == 0)
    {
      return i;
    }
  }
  return -1;
}

void update_lru(int mru, cacheLine *cacheSet, uint32_t cacheAssoc)
{ // mru  CacheLine** cache- most recently used
  for (int i = 0; i < cacheAssoc; i++)
  {
    if (cacheSet[i].lru > cacheSet[mru].lru)
    {
      cacheSet[i].lru = cacheSet[i].lru - 1;
    }
  }
  cacheSet[mru].lru = cacheAssoc - 1;
}

bool l2cache_contains(uint32_t tag) {
  for (int i = 0; i < l2cacheSets; i++) {
    for (int j = 0; j < l2cacheAssoc; j++) {
      if (l2cache[i][j].tag == tag && l2cache[i][j].valid) {
        return true;
      }
    }
  }
  return false;
}

// Check if a block with a given address is present in the cache
bool cache_contains(cacheLine** cache, int cacheSets, int cacheAssoc, int cacheBlocksize, uint32_t addr) {
  int index = (addr / cacheBlocksize) % cacheSets;
  int tag = addr / cacheBlocksize / cacheSets;

  for (int i = 0; i < cacheAssoc; i++) {
    if (cache[index][i].tag == tag && cache[index][i].valid) {
      return true;
    }
  }
  return false;
}

// Invalidate a block with a given address in the cache
void cache_evict(cacheLine** cache, int cacheSets, int cacheAssoc, int cacheBlocksize, uint32_t addr) {
  int index = (addr / cacheBlocksize) % cacheSets;
  int tag = addr / cacheBlocksize / cacheSets;

  for (int i = 0; i < cacheAssoc; i++) {
    if (cache[index][i].tag == tag && cache[index][i].valid) {
      cache[index][i].valid = false;
      return;
    }
  }
}

// Perform a memory access through the dcache interface for the address 'addr'
//  Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
  int index = (addr / dcacheBlocksize) % (dcacheSets);
  int tag = addr / dcacheBlocksize / dcacheSets;
  uint32_t penalty;
  dcacheRefs++;
  
  for (int i = 0; i < dcacheAssoc; i++)
  {
    if (dcache[index][i].tag == tag && dcache[index][i].valid)
    {
      update_lru(i, dcache[index], dcacheAssoc);
      return dcacheHitTime;
    }
  }

  dcacheMisses++;
  penalty = l2cache_access(addr);
  dcachePenalties += penalty;
  uint32_t evictInd = get_lru(dcache[index], dcacheAssoc);

  dcache[index][evictInd].tag = tag;
  dcache[index][evictInd].valid = true;
  update_lru(evictInd, dcache[index], dcacheAssoc);

  return dcacheHitTime + penalty;
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
l2cache_access(uint32_t addr)
{
  int index = (addr / l2cacheBlocksize) % (l2cacheSets);
  int tag = addr / l2cacheBlocksize / l2cacheSets;

  l2cacheRefs++;

  for (int i = 0; i < l2cacheAssoc; i++)
  {
    if (l2cache[index][i].tag == tag && l2cache[index][i].valid)
    {
      update_lru(i, l2cache[index], l2cacheAssoc);
      return l2cacheHitTime;
    }
  }
  l2cacheMisses++;
  u_int32_t penalty = memspeed;
  l2cachePenalties += penalty;

  
  uint32_t evictInd = get_lru(l2cache[index], l2cacheAssoc);
  if (inclusive) {
    uint32_t evictedAddr = l2cache[index][evictInd].tag * l2cacheBlocksize * l2cacheSets + index * l2cacheBlocksize;
    if (cache_contains(icache, icacheSets, icacheAssoc, icacheBlocksize, evictedAddr)) {
        cache_evict(icache, icacheSets, icacheAssoc, icacheBlocksize, evictedAddr);
    }
    if (cache_contains(dcache, dcacheSets, dcacheAssoc, dcacheBlocksize, evictedAddr)) {
        cache_evict(dcache, dcacheSets, dcacheAssoc, dcacheBlocksize, evictedAddr);
    }
  }
  l2cache[index][evictInd].tag = tag;
  l2cache[index][evictInd].valid = true;
  update_lru(evictInd, l2cache[index], l2cacheAssoc);

  return l2cacheHitTime + penalty;
}

// Predict an address to prefetch on icache with the information of last icache access:
// 'pc':     Program Counter of the instruction of last icache access
// 'addr':   Accessed Address of last icache access
// 'r_or_w': Read/Write of last icache access
uint32_t
icache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w)
{

  if (i_count == 0)
  {
    i_prev_addr = addr;
    i_count = 1;
  }
  else
  {
    uint32_t new_stride = addr - i_prev_addr;
    if (new_stride == i_stride)
    {
      i_count++;
      if (i_count >= 2)
      {
        if (i_stride < icacheBlocksize)
        {
          return addr + icacheBlocksize;
        }
        printf("Stride: %d\n", i_stride);
        // else
        return addr + i_stride;
      }
    }
    else
    {
      i_stride = new_stride;
      i_count = 1;
    }
    i_prev_addr = addr;
  }
  return addr + icacheBlocksize;
}

// Predict an address to prefetch on dcache with the information of last dcache access:
// 'pc':     Program Counter of the instruction of last dcache access
// 'addr':   Accessed Address of last dcache access
// 'r_or_w': Read/Write of last dcache access
uint32_t
dcache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w)
{

  if (d_count == 0)
  {
    d_prev_addr = addr;
    d_count = 1;
  }
  else
  {
    uint32_t new_stride = addr - d_prev_addr;
    if (new_stride == d_stride)
    {
      d_count++;
      if (d_count >= 2)
      {
        if (d_stride < dcacheBlocksize)
        {
          return addr + dcacheBlocksize;
        }
        // else
        return addr + d_stride;
      }
    }
    else
    {
      d_stride = new_stride;
      d_count = 1;
    }
    d_prev_addr = addr;
  }
  return addr + dcacheBlocksize;
}

// Perform a prefetch operation to I$ for the address 'addr'
void icache_prefetch(uint32_t addr)
{
  int index = (addr / icacheBlocksize) % (icacheSets);
  int tag = addr / icacheBlocksize / icacheSets;

  for (int i = 0; i < icacheAssoc; i++)
  {
    if (icache[index][i].tag == tag)
    {
      update_lru(i, icache[index], icacheAssoc);
      icache[index][i].valid = true;
      return;
    }
  }

  uint32_t evictInd = get_lru(icache[index], icacheAssoc);
  icache[index][evictInd].tag = tag;
  icache[index][evictInd].valid = true;
  update_lru(evictInd, icache[index], icacheAssoc);
}

// Perform a prefetch operation to D$ for the address 'addr'
void dcache_prefetch(uint32_t addr)
{
  int index = (addr / dcacheBlocksize) % (dcacheSets);
  int tag = addr / dcacheBlocksize / dcacheSets;

  for (int i = 0; i < dcacheAssoc; i++)
  {
    if (dcache[index][i].tag == tag)
    {
      update_lru(i, dcache[index], dcacheAssoc);
      dcache[index][i].valid = true;
      return;
    }
  }

  uint32_t evictInd = get_lru(dcache[index], dcacheAssoc);
  dcache[index][evictInd].tag = tag;
  dcache[index][evictInd].valid = true;
  update_lru(evictInd, dcache[index], dcacheAssoc);
}
