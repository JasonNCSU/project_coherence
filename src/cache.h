/*******************************************************
                          cache.h
********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;

/****add new states, based on the protocol****/
enum{
	INVALID = 0,
	VALID,
	DIRTY,
	M,
	S,
	I,
	E,
	O,
};

class cacheLine 
{
protected:
   ulong tag;
   ulong Flags;   // 0:invalid, 1:valid, 2:dirty 
   ulong seq; 
 
public:
   cacheLine()            { tag = 0; Flags = 0; }
   ulong getTag()         { return tag; }
   ulong getFlags()			{ return Flags;}
   ulong getSeq()         { return seq; }
   void setSeq(ulong Seq)			{ seq = Seq;}
   void setFlags(ulong flags)			{  Flags = flags;}
   void setTag(ulong a)   { tag = a; }
   void invalidate()      { tag = 0; Flags = INVALID; }//useful function
   bool isValid()         { return ((Flags) != INVALID); }
};

class Cache
{
protected:
   ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   ulong reads,readMisses,writes,writeMisses,writeBacks;

   //******///
   //add coherence counters here///
   ulong numCacheTransfers, numMemoryTransactions, numInterventions, numInvalidaitons, numFlushes, numBusRdX;
   //******///

   cacheLine **cache;
   ulong calcTag(ulong addr)     { return (addr >> (log2Blk) );}
   ulong calcIndex(ulong addr)  { return ((addr >> log2Blk) & tagMask);}
   ulong calcAddr4Tag(ulong tag)   { return (tag << (log2Blk));}
   
public:
    ulong currentCycle;
    bool busReads, busReadXs, busRdFlush;
     
    Cache(int,int,int);
   ~Cache() { delete cache;}
   
   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine * findLine(ulong addr);
   cacheLine * getLRU(ulong);
   
   ulong getRM(){return readMisses;} ulong getWM(){return writeMisses;} 
   ulong getReads(){return reads;}ulong getWrites(){return writes;}
   ulong getWB(){return writeBacks;}
   
   void writeBack(ulong)   {writeBacks++;}
   ulong Access(ulong,uchar,ulong);
   void printStats(int processor_num, int protocol);
   void updateLRU(cacheLine *);

   //******///
   //add other functions to handle bus transactions///
   virtual void prRd(ulong) = 0;
   virtual void prWr(ulong) = 0;
   virtual void flush() {return;};
   virtual void invalidations() {return;};
   virtual void memTransaction() {return;};
   virtual void busRd(ulong) {return;};
   virtual void busUpgr(ulong) {return;};
   virtual void busUpdate(ulong) {return;};
   virtual void busRdX(ulong) {return;};
   virtual void busWr(ulong) {return;};
   //******///

};

class Msi : public Cache {
public:
    Msi(int s, int a, int b):
        Cache(s, a, b) {};
    ~Msi() {};

    //state machine calls
    void prRd(ulong);
    void prWr(ulong);
    void flush();
    void invalidations();
    void memTransaction();
    void busRd(ulong);
    void busRdX(ulong);
    //state machine calls
};

class Mesi : public Cache {
public:
    Mesi(int s, int a, int b):
            Cache(s, a, b) {};
    ~Mesi() {};

    //state machine calls
    void prRd(ulong);
    void prWr(ulong);
    void flush();
    void busRd(ulong);
    void busRdX(ulong);
    void busWr(ulong);
    //state machine calls
};

class Dragon : public Cache {
public:
    Dragon(int s, int a, int b):
            Cache(s, a, b) {};
    ~Dragon() {};

    //state machine calls
    void prRd(ulong);
    void prWr(ulong);
    void flush();
    void busRd(ulong);
    void busUpdate(ulong);
    //state machine calls
};

#endif
