/*******************************************************
                          cache.cc
********************************************************/

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <iomanip>
#include "cache.h"
using namespace std;

Cache::Cache(int s,int a,int b )
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;

   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  
   //*******************//
   //initialize your counters here//
    numCacheTransfers = numMemoryTransactions = 0;
    numInterventions = numInvalidaitons = 0;
    numFlushes = numBusRdX = 0;
    busReads = busReadXs = busRdFlush = false;
   //*******************//
 
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
		tagMask <<= 1;
        tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++) 
      {
	   cache[i][j].invalidate();
      }
   }      
   
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
ulong Cache::Access(ulong addr,uchar op, ulong protocol)
{
	/*per cache global counter to maintain LRU order
			among cache ways, updated on every cache access*/

	
	cacheLine * line = findLine(addr);

	ulong oldState;

	if(line == NULL)/*miss*/
	{
		if(op == 'w') writeMisses++;
		else readMisses++;

		cacheLine *newline = fillLine(addr);
		if(op == 'r') {
            if (protocol == 0) {
                newline->setFlags(S);
                oldState = I;
            } else if (protocol == 1) {
                newline->setFlags(S);
            } else {
                newline->setFlags(S);
            }
		} else if(op == 'w') {
		    if (protocol == 0) {
   		        newline->setFlags(M);
                oldState = I;
   		    } else if (protocol == 1) {
                newline->setFlags(M);
   		    } else {
                newline->setFlags(M);
   		    }
   		}
		
	}
	else
	{
	    oldState = line->getFlags();
		/**since it's a hit, update LRU and update dirty flag**/
		updateLRU(line);
		if(op == 'w') {
            if (protocol == 0) {
                line->setFlags(M);
            } else if (protocol == 1) {
                line->setFlags(M);
            } else {
                line->setFlags(M);
            }
        }
	}

	return oldState;
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache[i][j].isValid())
	        if(cache[i][j].getTag() == tag)
		{
		     pos = j; break; 
		}
   if(pos == assoc)
	return NULL;
   else
	return &(cache[i][pos]); 
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) return &(cache[i][j]);     
   }   
   for(j=0;j<assoc;j++)
   {
	 if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
   } 
   assert(victim != assoc);
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   if(victim->getFlags() == DIRTY || victim->getFlags() == M) writeBack(addr);
    	
   tag = calcTag(addr);   
   victim->setTag(tag);
   victim->setFlags(VALID);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

void Cache::printStats(int processor_num, int protocol)
{
    double miss_rate = 0;

    if (reads != 0 || writes != 0) {
        miss_rate = 100 * (double) (readMisses + writeMisses) / (double) (reads + writes);
    }

	cout << "============ Simulation results (Cache " << processor_num << ") ============" << endl;
	/****print out the rest of statistics here.****/
	cout << "01. number of reads:				" << reads << endl;
    cout << "02. number of read misses:			" << readMisses << endl;
    cout << "03. number of writes:				" << writes << endl;
    cout << "04. number of write misses:			" << writeMisses << endl;
    cout << setprecision(2) << fixed << "05. total miss rate:				" << miss_rate << "%" << endl;
    cout << "06. number of writebacks:			" << writeBacks << endl;
    cout << "07. number of cache-to-cache transfers:		" << numCacheTransfers << endl;
    cout << "08. number of memory transactions:		" << numMemoryTransactions << endl;
    cout << "09. number of interventions:			" << numInterventions << endl;
    cout << "10. number of invalidations:			" << numInvalidaitons << endl;
    cout << "11. number of flushes:				"<< numFlushes << endl;
    cout << "12. number of BusRdX:				" << numBusRdX << endl;
    /****follow the ouput file format**************/
}

//MSI Functions
void Msi::prRd(ulong addr) {
    currentCycle++;
    reads++;
    cacheLine *line = findLine(addr);

    if (!line) {
        readMisses++;
        cacheLine *newline = fillLine(addr);
        memTransaction();
        newline->setFlags(S);
        busReads = true;
    } else if (line->getFlags() != I) {
        updateLRU(line);
    } else if (line->getFlags() == I) { //might have to undo this part
        updateLRU(line);
        line->setFlags(S);
        busReads = true;
    }
}
void Msi::prWr(ulong addr) {
    currentCycle++;
    writes++;
    cacheLine *line = findLine(addr);

    if (!line) {
        writeMisses++;
        cacheLine *newline = fillLine(addr);
        newline->setFlags(M);
        busReadXs = true;
    } else if (line->getFlags() == I) {
        updateLRU(line);
        writeMisses++;
        line->setFlags(M);
        busReadXs = true;
    } else {
        updateLRU(line);
        if (line->getFlags() == S) {
            line->setFlags(M);
            busReadXs = true;
        }
    }
}
void Msi::flush() {
    numFlushes++;
}
void Msi::invalidations() {
    numInvalidaitons++;
}
void Msi::memTransaction() {
    numMemoryTransactions++;
}
void Msi::busRd(ulong addr) {
    cacheLine *line = findLine(addr);
    ulong state;

    if (line) {
        state = line->getFlags();
        if (state == M) {
            line->setFlags(S);
            memTransaction();
            flush();
            writeBack(addr);
            busRdFlush = true;
        }
    }
}
void Msi::busUpgr(ulong addr) {

}
void Msi::busRdX(ulong addr) {
    cacheLine *line = findLine(addr);
    ulong state;
    if (line) {
        state = line->getFlags();
        if (state == S) {
            line->setFlags(I);
            invalidations();
            memTransaction();
            numBusRdX++;
        } else if (state == M) {
            line->setFlags(I);
            invalidations();
            memTransaction();
            flush();
            writeBack(addr);
            numBusRdX++;
        }
    }
}
//MSI Functions

//MESI Functions
void Mesi::prRd(ulong addr) {

}
void Mesi::prWr(ulong addr) {

}
void Mesi::flush() {

}
void Mesi::busRd(ulong addr) {

}
void Mesi::busRdX(ulong addr) {

}
void Mesi::busWr(ulong addr) {

}
//MESI Functions

//Dragon Functions
void Dragon::prRd(ulong addr) {

}
void Dragon::prWr(ulong addr) {

}
void Dragon::flush() {

}
void Dragon::busRd(ulong addr) {

}
void Dragon::busUpdate(ulong addr) {

}
//Dragon Functions