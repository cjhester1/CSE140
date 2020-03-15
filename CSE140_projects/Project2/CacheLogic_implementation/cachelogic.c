/*
 
 I recieved help on this from my tutor sundeep kumar sundeep@sundeepkumar.com
 Sundeep has tutored many students and he mentioned he has worked with many UC let alone
 UC Merced Students.
 
 I looked at this page for reference and observing on github.com https://github.com/chrisdesoto/CSE-140-Project2/blob/master/cachelogic.c
 I also looked at this page for referencing and observing https://github.com/DLohmann/mips-cache-simulator/blob/master/cachelogic.c

 Works cited:
 
 https://practice.geeksforgeeks.org/problems/difference-between-lru-and-lfu
 
 */



#include "tips.h"

/* The following two functions are defined in util.c */

/* finds the highest 1 bit, and returns its position, else 0xFFFFFFFF */
unsigned int uint_log2(word w); 

/* return random int from 0..x-1 */
int randomint( int x );
int fetch_lru (int index);

/*
  This function allows the lfu information to be displayed

    assoc_index - the cache unit that contains the block to be modified
    block_index - the index of the block to be modified

  returns a string representation of the lfu information
 */
char* lfu_to_string(int assoc_index, int block_index)
{
  /* Buffer to print lfu information -- increase size as needed. */
  static char buffer[9];
  sprintf(buffer, "%u", cache[assoc_index].block[block_index].accessCount);

  return buffer;
}

/*
  This function allows the lru information to be displayed

    assoc_index - the cache unit that contains the block to be modified
    block_index - the index of the block to be modified

  returns a string representation of the lru information
 */
char* lru_to_string(int assoc_index, int block_index)
{
  /* Buffer to print lru information -- increase size as needed. */
  static char buffer[9];
  sprintf(buffer, "%u", cache[assoc_index].block[block_index].lru.value);

  return buffer;
}

/*
  This function initializes the lfu information

    assoc_index - the cache unit that contains the block to be modified
    block_number - the index of the block to be modified

*/
void init_lfu(int assoc_index, int block_index)
{
  cache[assoc_index].block[block_index].accessCount = 0;
}

/*
  This function initializes the lru information

    assoc_index - the cache unit that contains the block to be modified
    block_number - the index of the block to be modified

*/
void init_lru(int assoc_index, int block_index)
{
  cache[assoc_index].block[block_index].lru.value = 0;
}

/*
  This is the primary function you are filling out,
  You are free to add helper functions if you need them

  @param addr 32-bit byte address
  @param data a pointer to a SINGLE word (32-bits of data)
  @param we   if we == READ, then data used to return
              information back to CPU

              if we == WRITE, then data used to
              update Cache/DRAM
*/

/*
 
 LRU is a cache eviction algorithm called least recently used cache.
 
 It uses a hash table to cache the entries and a double linked list to keep track of the access order. If an entry is inserted, updated or accessed, it gets removed and re-linked before the head node. We need to find the LRU first because when the cache reaches maximum size the LRU will  be taken from the cache
 
 */

int fetch_lru(int init_index)
{
    int head = 0;
    int block = 0;
    for (int i = 0; i < assoc; i++)
    {
        if (cache[init_index].block[i].lru.value > head)
        {
            head = cache[init_index].block[i].lru.value;
            block = i;
        }
    }
    return block;
}
    /*LFU is a cache eviction algorithm called least frequently used cache. With LFU we want to find Least Frequently Used. Here we return the Least frequently used block*/
    int fetch_lfu (int init_index)
    {
        unsigned int min = 0xFFFFFFFF;
        unsigned int block = 0;
        for (int i = 0; i < assoc; i++)
        {
            if (cache[init_index].block[i].accessCount < min)
            {
                block = i;
                min = cache[init_index].block[i].accessCount;
            }
        }
        return block;
        
    }

void accessMemory(address addr, word* data, WriteEnable we)
{
    /* Declare variables here */
    
    
    /*
     1 byte = 8 bits
     1 word = 4 bytes
     1/2 word = 2 bytes
     double_word = 8 bytes
     OCT_WORD = 16 bytes
     */
    
    
    TransferUnit tu;
    int off_bits = 0;
     int index_bits;
    address gg;
    int asc = assoc;
    switch (block_size)
        {
        case 0x0:
            tu = BYTE_SIZE;
            break;
        case 0x2://2
            tu = HALF_WORD_SIZE;
            break;
        case 0x4://4
            tu = WORD_SIZE;
            break;
        case 0x8://8
            tu = DOUBLEWORD_SIZE;
            break;
        case 0x10://16
            tu = QUADWORD_SIZE;
            break;
        case 0x20://32
            tu = OCTWORD_SIZE;
            break;
        }
    
    off_bits = uint_log2(block_size);
    index_bits = uint_log2(set_count);
    int tag_bits = 32 - (off_bits + index_bits);
    
    unsigned int offset = addr << (tag_bits + index_bits);
    offset = offset >> (tag_bits);
    unsigned int index;
    index = addr << (tag_bits);
    index = index >> (tag_bits + off_bits);
    unsigned int tag = addr >> (off_bits + index_bits);
    
    
    
  /* handle the case of no cache at all - leave this in */
  if(assoc == 0)
  {
    accessDRAM(addr, (byte*)data, WORD_SIZE, we);
    return;
  }

  /*
  You need to read/write between memory (via the accessDRAM() function) and
  the cache (via the cache[] global structure defined in tips.h)

  Remember to read tips.h for all the global variables that tell you the
  cache parameters

  The same code should handle random, LFU, and LRU policies. Test the policy
  variable (see tips.h) to decide which policy to execute. The LRU policy
  should be written such that no two blocks (when their valid bit is VALID)
  will ever be a candidate for replacement. In the case of a tie in the
  least number of accesses for LFU, you use the LRU information to determine
  which block to replace.

  Your cache should be able to support write-through mode (any writes to
  the cache get immediately copied to main memory also) and write-back mode
  (and writes to the cache only gets copied to main memory when the block
  is kicked out of the cache.

  Also, cache should do allocate-on-write. This means, a write operation
  will bring in an entire block if the block is not already in the cache.

  To properly work with the GUI, the code needs to tell the GUI code
  when to redraw and when to flash things. Descriptions of the animation
  functions can be found in tips.h
  */

  /* Start adding code here */

    int key = 0;
    while (key < assoc)
    {
        if (cache[index].block[key].tag == tag && cache[index].block[key].valid == 1)
        {
            gg = key;
            highlight_offset(index, gg, offset, HIT);
            break;
        }
        key++;
    }
    if (we == READ)
    {
        if (asc != assoc)
        {
            memcpy((void *)data, (void *)cache[index].block[gg].data + offset, block_size);
        }
        else {
            accessDRAM(addr, (void*)data, tu, READ);
            int d_lru;
            
            switch (policy)//replacementpolicy
            {
                //This  code segment handle LRU policies.
                case LRU:
                    d_lru = fetch_lru(index);
                
                    if (cache[index].block[d_lru].dirty == DIRTY)
                    {
                       
                        gg = cache[index].block[d_lru].tag << ((index_bits + off_bits) + (index << off_bits));
                        accessDRAM(gg, (cache[index].block[d_lru].data), tu, WRITE);
                    }
                    memcpy((void *)cache[index].block[d_lru].data, (void *)data, block_size);
                    cache[index].block[d_lru].dirty = VIRGIN;
                    cache[index].block[d_lru].valid = VALID;
                    cache[index].block[d_lru].tag = tag;
                    cache[index].block[d_lru].accessCount = 0;
                    cache[index].block[d_lru].lru.value = 1;
                    break;
                // This code segment handles LFU policies.
                case LFU:
                    d_lru = fetch_lfu(index);
                    if (cache[index].block[d_lru].dirty == DIRTY)
                    {

                        gg = cache[index].block[d_lru].tag << ((index_bits + off_bits) + (index << off_bits));
                        accessDRAM(gg, (cache[index].block[d_lru].data), tu, WRITE);
                    }
                    memcpy((void *)cache[index].block[d_lru].data, (void *)data, block_size);
                    cache[index].block[d_lru].dirty = VIRGIN;
                    cache[index].block[d_lru].valid = VALID;
                    cache[index].block[d_lru].tag = tag;
                    cache[index].block[d_lru].accessCount = 0;
                    cache[index].block[d_lru].lru.value = 1;
                    break;
                  
                // This code segment handles Random policies.
                case RANDOM:
                    
                    d_lru = randomint(assoc);
                    
                    if (cache[index].block[d_lru].dirty == DIRTY)
                    {
                        gg = cache[index].block[d_lru].tag << ((index_bits + off_bits) + (index << off_bits));
                        accessDRAM(gg, (cache[index].block[d_lru].data), tu, WRITE);
                        
                    }
                    
                    memcpy((void *)cache[index].block[d_lru].data, (void *)data, block_size);
                    cache[index].block[d_lru].dirty = VIRGIN;
                    cache[index].block[d_lru].valid = VALID;
                    cache[index].block[d_lru].tag = tag;
                    cache[index].block[d_lru].accessCount = 0;
                    cache[index].block[d_lru].lru.value = 1;
                    break;
            }
            
            highlight_block(index, d_lru);
            highlight_offset(index, d_lru, offset, MISS);
            }
            
        }

        else {
            int  e_lru;
            switch (memory_sync_policy)//Memorysyncpolicy
            {
                case WRITE_BACK:
                    if (policy == LRU)
                    {
                        e_lru = fetch_lru(index);
                    }
                    else if(policy==RANDOM)
                    {
                        e_lru = randomint(assoc);//returns a random integer from 0 to ...x-1
                        
                    }
                    else
                    {
                        e_lru = fetch_lfu(index);
                        
                    }
                    if (cache[index].block[e_lru].dirty == DIRTY)
                    {
                        gg = cache[index].block[e_lru].tag << ((index_bits + off_bits) + (index << off_bits));
                        accessDRAM(gg, (cache[index].block[e_lru].data), tu, WRITE);
                    }
                    memcpy((void *)cache[index].block[e_lru].data, (void *)data, block_size);
                    cache[index].block[e_lru].dirty = VIRGIN;
                    cache[index].block[e_lru].valid = VALID;
                    cache[index].block[e_lru].tag = tag;
                    cache[index].block[e_lru].accessCount = 0;
                    cache[index].block[e_lru].lru.value = 1;
                    
                    break;
                    
                case WRITE_THROUGH:
                    if (policy == LRU) {
                        e_lru = fetch_lru(index);
                    }
                    
                    else if(policy==RANDOM) {
                        e_lru = randomint(assoc);
                        
                    }
                    else{
                        e_lru = fetch_lfu(index);
                        
                    }
                    if (cache[index].block[e_lru].dirty == DIRTY)
                    {
                        gg = cache[index].block[e_lru].tag << ((index_bits + off_bits) + (index << off_bits));
                        accessDRAM(gg, (cache[index].block[e_lru].data), tu, WRITE);
                    }
                    
                    accessDRAM(addr, (void *)cache[index].block[e_lru].data, tu, WRITE);
                    memcpy((void *)cache[index].block[e_lru].data, (void *)data, block_size);
                    cache[index].block[e_lru].dirty = VIRGIN;
                    cache[index].block[e_lru].valid = VALID;
                    cache[index].block[e_lru].tag = tag;
                    cache[index].block[e_lru].accessCount = 0;
                    cache[index].block[e_lru].lru.value = 1;
                    break;
            }
        }
}
