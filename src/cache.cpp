/*Questions:


*/


/*Questions:

- What are penalties?
- How does main work with the traces? How are the variables filled in.
- Type casted some variables in main due to errors, do you prefer we change the data type in cache.cpp/hpp?

- Segmentation Fault

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
  //
  // TODO: Implement I$
  //
  int index = (addr / icacheBlocksize) % (icacheSets); // * icacheAssoc);
  int tag = addr / icacheBlocksize / icacheSets;       // / icacheAssoc;

  icacheRefs++;

  for (int i = 0; i < icacheAssoc; i++)
  {
    if (icache[index][i].tag == tag && icache[index][i].valid)
    {
      updateLRU(i, icache[index], icacheAssoc);
      return icacheHitTime;
    }
  }

  icacheMisses++;

  uint32_t penalty = icacheHitTime + l2cache_access(addr); // question

  icachePenalties += penalty;
  uint32_t evictInd = getLRU(icache[index], icacheAssoc);
  icache[index][evictInd].tag = tag;
  icache[index][evictInd].valid = true;
  updateLRU(evictInd, icache[index], icacheAssoc);

  // printf("Before icache_access return");

  return penalty;
}

uint32_t getLRU(cacheLine *cacheSet, uint32_t cacheAssoc)
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

void updateLRU(int mru, cacheLine *cacheSet, uint32_t cacheAssoc)
{ // mru  CacheLine** cache- most recently used
  for (int i = 0; i < cacheAssoc; i++)
  {
    if (cacheSet[i].lru > mru)
    {
      cacheSet[i].lru = cacheSet[i].lru - 1;
    }
  }
  cacheSet[mru].lru = cacheAssoc - 1;
}

// Perform a memory access through the dcache interface for the address 'addr'
//  Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
  int index = (addr / dcacheBlocksize) % (dcacheSets); // * dcacheAssoc);
  int tag = addr / dcacheBlocksize / dcacheSets;       // / dcacheAssoc;

  dcacheRefs++;
  // printf("Before dcache_access for loop");

  for (int i = 0; i < dcacheAssoc; i++)
  {
    if (dcache[index][i].tag == tag && dcache[index][i].valid)
    {
      updateLRU(i, dcache[index], dcacheAssoc);
      return dcacheHitTime;
    }
  }
  dcacheMisses++;
  uint32_t penalty = dcacheHitTime + l2cache_access(addr);
  dcachePenalties += penalty;
  uint32_t evictInd = getLRU(dcache[index], dcacheAssoc);
  dcache[index][evictInd].tag = tag;
  dcache[index][evictInd].valid = true;
  updateLRU(evictInd, dcache[index], dcacheAssoc);

  return penalty;
  // return 1;
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
l2cache_access(uint32_t addr)
{
  int index = (addr / l2cacheBlocksize) % (l2cacheSets); // * l2cacheAssoc);
  int tag = addr / l2cacheBlocksize / l2cacheSets;       // / l2cacheAssoc;

  l2cacheRefs++;

  // printf("Before l2cache_access for loop\n");
  // printf("addr: %x\n", addr);
  // printf("index: %d \n", index);
  // exit(0);

  for (int i = 0; i < l2cacheAssoc; i++)
  {
    if (l2cache[index][i].tag == tag && l2cache[index][i].valid)
    {
      updateLRU(i, l2cache[index], l2cacheAssoc);
      return l2cacheHitTime;
    }
  }

  l2cacheMisses++;
  u_int32_t penalty = l2cacheHitTime + memspeed;
  l2cachePenalties += penalty;
  uint32_t evictInd = getLRU(l2cache[index], l2cacheAssoc);
  l2cache[index][evictInd].tag = tag;
  l2cache[index][evictInd].valid = true;
  updateLRU(evictInd, l2cache[index], l2cacheAssoc);

  return penalty;
  // return 1;
}

// Predict an address to prefetch on icache with the information of last icache access:
// 'pc':     Program Counter of the instruction of last icache access
// 'addr':   Accessed Address of last icache access
// 'r_or_w': Read/Write of last icache access
uint32_t
icache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w)
{
  static uint32_t prev_addr = 0;
  static uint32_t stride = 0;
  static int count = 0;

  if (count == 0)
  {
    prev_addr = addr;
  }
  else
  {
    uint32_t new_stride = addr - prev_addr;
    if (new_stride == stride)
    {
      count++;
      if (count >= 3)
      { // tuning parameters
        return addr + stride;
      }
    }
    else
    {
      stride = new_stride;
      count = 1;
    }
    prev_addr = addr;
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
  return addr + dcacheBlocksize; // Next line prefetching
  //
  // TODO: Implement a better prefetching strategy
  //
}

// Perform a prefetch operation to I$ for the address 'addr'
void icache_prefetch(uint32_t addr)
{
  int index = (addr / icacheBlocksize) % (icacheSets * icacheAssoc);
  int tag = addr / icacheBlocksize / icacheSets / icacheAssoc;

  for (int i = 0; i < icacheAssoc; i++)
  {
    if (icache[index][i].tag == tag)
    {
      updateLRU(i, icache[index], icacheAssoc);
      icache[index][i].valid = true;
      return;
    }
  }

  uint32_t evictInd = getLRU(icache[index], icacheAssoc);
  icache[index][evictInd].tag = tag;
  icache[index][evictInd].valid = true;
  updateLRU(evictInd, icache[index], icacheAssoc);
}

// Perform a prefetch operation to D$ for the address 'addr'
void dcache_prefetch(uint32_t addr)
{
  int index = (addr / dcacheBlocksize) % (dcacheSets * dcacheAssoc);
  int tag = addr / dcacheBlocksize / dcacheSets / dcacheAssoc;

  for (int i = 0; i < dcacheAssoc; i++)
  {
    if (dcache[index][i].tag == tag)
    {
      updateLRU(i, dcache[index], dcacheAssoc);
      dcache[index][i].valid = true;
      return;
    }
  }

  uint32_t evictInd = getLRU(dcache[index], dcacheAssoc);
  dcache[index][evictInd].tag = tag;
  dcache[index][evictInd].valid = true;
  updateLRU(evictInd, dcache[index], dcacheAssoc);
}
