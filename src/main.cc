/*******************************************************
                          main.cc
********************************************************/

#include <cstdlib>
#include <fstream>
#include <cassert>
#include "cache.h"
using namespace std;

Cache *processorArray[8];
int num_processors;

//Coherence Methods
int numShares(ulong addr) {
    int sharers = 0;
    for (int i = 0; i < num_processors; i++) {
        if (processorArray[i]->findLine(addr)) {
            sharers++;
        }
    }
    return sharers;
}

void doBusRds(ulong addr, int processor) {
    for (int i = 0; i < num_processors; i++) {
        if (i != processor) {
            processorArray[i]->busRd(addr);
        }
    }
}

void doBusWrs(ulong addr, int processor) {
    for (int i = 0; i < num_processors; i++) {
        if (i != processor) {
            processorArray[i]->busWr(addr);
        }
    }
}

void doBusRdXs(ulong addr, int processor) {
    for (int i = 0; i < num_processors; i++) {
        if (i != processor) {
            processorArray[i]->busRdX(addr);
        }
    }
}
//Coherence Methods

int main(int argc, char *argv[])
{

	FILE * pFile;

	if(argv[1] == NULL){
		 printf("input format: ");
		 printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
		 exit(0);
        }

	/*****uncomment the next five lines*****/
	int cache_size = atoi(argv[1]);
	int cache_assoc= atoi(argv[2]);
	int blk_size   = atoi(argv[3]);
	num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
	int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:Dragon*/
	char *fname =  (char *)malloc(20);
 	fname = argv[6];

	
	//****************************************************//
    printf("===== 506 Personal Information =====\n");
    printf("Iason Katsaros\n");
    printf("ikatsar\n");
    printf("ECE492 Students? NO\n");
	printf("===== 506 SMP Simulator configuration =====\n");
	//*******print out simulator configuration here*******//
	//****************************************************//
    cout << "L1_SIZE: " << cache_size << endl;
    cout << "L1_ASSOC: " << cache_assoc << endl;
    cout << "L1_BLOCKSIZE: " << blk_size << endl;
    cout << "NUMBER OF PROCESSORS: " << num_processors << endl;
    switch (protocol) {
        case 0:
            cout << "COHERENCE PROTOCOL: MSI" << endl;
            break;
        case 1:
            cout << "COHERENCE PROTOCOL: MESI" << endl;
            break;
        case 2:
            cout << "COHERENCE PROTOCOL: Dragon" << endl;
            break;
        default:
            cout << "PROTOCOL WAS NEITHER 0, 1, OR 2 - ENDING PROGRAM" << endl;
            exit(0);
    }
    cout << "TRACE FILE: " << fname << endl;

	//*********************************************//
	//*****create an array of caches here**********//

	int cache_counter = 0;
	for (cache_counter = 0; cache_counter < num_processors; cache_counter++) {
        switch (protocol) {
            case 0:
                //COHERENCE PROTOCOL: MSI
                processorArray[cache_counter] = new Msi(cache_size, cache_assoc, blk_size);
                break;
            case 1:
                //COHERENCE PROTOCOL: MESI
                processorArray[cache_counter] = new Mesi(cache_size, cache_assoc, blk_size);
                break;
            case 2:
                //COHERENCE PROTOCOL: Dragon
                processorArray[cache_counter] = new Dragon(cache_size, cache_assoc, blk_size);
                break;
            default:
                //unreachable, earlier if default was reached we exited program
                exit(0);
        }
	}
	//*********************************************//	

	pFile = fopen (fname,"r");
	if(pFile == 0)
	{   
		printf("Trace file problem\n");
		exit(0);
	}
	///******************************************************************//
	//**read trace file,line by line,each(processor#,operation,address)**//
	//*****propagate each request down through memory hierarchy**********//
	//*****by calling cachesArray[processor#]->Access(...)***************//
    ifstream fin(fname);
	string data_segment;
	int processor = 0;
	uchar rw = ' ';
	ulong addr = 0;
    Cache *cachePtr;
    ulong oldState;
    ulong newState;

    while (getline(fin, data_segment)) {
        processor = data_segment.at(0) - '0';
        rw = data_segment.at(2);
        addr = strtoul(data_segment.substr(4).c_str(), NULL, 16);

        cachePtr = processorArray[processor];
        oldState = cachePtr->Access(addr, rw, protocol);
        newState = cachePtr->findLine(addr)->getFlags();

        switch (protocol) {
            case 0:
                //COHERENCE PROTOCOL: MSI
                if (oldState == S) {
                    if (newState == M) {
                        //BusUpgr()
                        for (int i = 0; i < num_processors; i++) {
                            if (i != processor) {
                                if (processorArray[i]->findLine(addr)) {
                                    if (processorArray[i]->findLine(addr)->getFlags() == S) {
                                        processorArray[i]->findLine(addr)->setFlags(I);
                                        processorArray[i]->invalidations();
                                    }
                                }
                            }
                        }
                    } else if (newState == S) {
                        //BusRd()
                        for (int i = 0; i < num_processors; i++) {
                            if (i != processor) {
                                if (processorArray[i]->findLine(addr)) {
                                    if (processorArray[i]->findLine(addr)->getFlags() == M) {
                                        processorArray[i]->findLine(addr)->setFlags(S);
                                        processorArray[i]->flush();
                                    }
                                }
                            }
                        }
                    }
                } else if (oldState == I) {
                    if (newState == M) {
                        //BusRdX()
                        for (int i = 0; i < num_processors; i++) {
                            if (i != processor) {
                                if (processorArray[i]->findLine(addr)) {
                                    if (processorArray[i]->findLine(addr)->getFlags() == M) {
                                        processorArray[i]->findLine(addr)->setFlags(I);
                                        processorArray[i]->flush();
                                        processorArray[i]->invalidations();
                                        processorArray[i]->busRdX();
                                    } else if (processorArray[i]->findLine(addr)->getFlags() == S) {
                                        processorArray[i]->findLine(addr)->setFlags(I);
                                        processorArray[i]->invalidations();
                                        processorArray[i]->busRdX();
                                    }
                                }
                            }
                        }
                    } else if (newState == S) {
                        //BusRd()
                        for (int i = 0; i < num_processors; i++) {
                            if (i != processor) {
                                if (processorArray[i]->findLine(addr)) {
                                    if (processorArray[i]->findLine(addr)->getFlags() == M) {
                                        processorArray[i]->findLine(addr)->setFlags(S);
                                        processorArray[i]->flush();
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            case 1:
                //COHERENCE PROTOCOL: MESI

                break;
            case 2:
                //COHERENCE PROTOCOL: Dragon

                break;
            default:
                //unreachable, earlier if default was reached we exited program
                exit(0);
        }
    }

	///******************************************************************//
	fclose(pFile);

	//********************************//
	//print out all caches' statistics //
	int print_stats = 0;
	for (print_stats = 0; print_stats < num_processors; print_stats++) {
	    processorArray[print_stats]->printStats(print_stats, protocol);
	}
	//********************************//

	return 0;
}
