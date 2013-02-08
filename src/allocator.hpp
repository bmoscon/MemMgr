/*
 * allocator.hpp
 *
 *
 * Custom Allocator for MemMgr
 *
 *
 * Copyright (C) 2013  Bryant Moscon - bmoscon@gmail.com
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions, and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution, and in the same 
 *    place and form as other copyright,
 *    license and disclaimer information.
 *
 * 3. The end-user documentation included with the redistribution, if any, must 
 *    include the following acknowledgment: "This product includes software 
 *    developed by Bryant Moscon (http://www.bryantmoscon.org/)", in the same 
 *    place and form as other third-party acknowledgments. Alternately, this 
 *    acknowledgment may appear in the software itself, in the same form and 
 *    location as other such third-party acknowledgments.
 *
 * 4. Except as contained in this notice, the name of the author, Bryant Moscon,
 *    shall not be used in advertising or otherwise to promote the sale, use or 
 *    other dealings in this Software without prior written authorization from 
 *    the author.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */


/* Each piece of allocated memory has a MCB at the start of it, followed by the requested 
 * amount of space. The pointer returned to user is incremented past the MCB. The MCB
 * contains info like size of the following memory and if the memory is "free" for use.
 *
 * When user requests allocation, walk the list (we have pointer to start and we know
 * the size to increment to reach the next MCB, and so on) looking for open blocks. If one 
 * is found (i.e. size is >= requested size and the free bit is set), mark it as used, 
 * and return it.
 *
 * If block is not found, allocated a new block at the end of the list, set MCB accordingly, and
 * return to user, keeping track of the last returned address. 
 */
 

#ifndef __MEM_ALLOCATOR__
#define __MEM_ALLOCATOR__

#include <cassert>

#include <stdint.h>
#include <unistd.h>

class Allocator {
public:
  
  Allocator() 
  {
    /* calling sbrk with increment size 0 is used to find current location of
     * program break (end of process data segment). 
     */
    last_addr_ = sbrk(0);
    mem_start_ = last_addr_; 
  }
    

  void free(void *block)
  {
    mcb_st *block_mcb;
    
    // pointer arithmetic to extract MCB from block. Once found, mark "free"
    block_mcb = (mcb_st *)((uint8_t *)block - sizeof(mcb_st));
    block_mcb->free = true;
  }


  void *alloc(uint32_t size)
  {
    void *curr = NULL;
    void *ret = NULL;
    mcb_st *mcb;
    
    assert(size =! 0);
    
    curr = mem_start_;
    size += sizeof(mcb_st);

    while (curr != last_addr_) {
      // get the MCB so we can test for open blocks and size
      mcb = (mcb_st *)curr;
      
      if ((mcb->free) && (mcb->size >= size)) {
	mcb->free = false;
	ret = curr;
	
	ret = (uint8_t *)ret + sizeof(mcb_st);
	
	return (ret);
      }

      curr = (uint8_t *)curr + mcb->size;
    }
    
    // no free allocated blocks, so allocated a new one
    sbrk(size);
    
    // set the return address to the last address pointer and then increment last_addr_ past this
    // newly allocated block
    ret = last_addr_;
    last_addr_ = (uint8_t *)last_addr_ + size;
    
    // set MCB for the block
    mcb = (mcb_st *)ret;
    mcb->free = false;
    mcb->size = size;
    
    // move pointer past the MCB
    ret = (uint8_t *)ret + sizeof(mcb_st);

    return (ret);
  }
  


protected:
  typedef struct mcb_st {
    bool free;
    uint32_t size;
    
    mcb_st() : size(0), free(false) {}
    mcb_st(uint32_t s) : size(s), free(false) {}
  } mcb_st;


  void *mem_start_;
  void *last_addr_;

};


#endif
