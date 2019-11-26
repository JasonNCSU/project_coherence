/*******************************************************
                          main.cc
********************************************************/

#include <cstdlib>
#include <fstream>
#include <cassert>
#include "cache.h"
using namespace std;

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
	int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
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
	Cache *processorArray = (Cache *)malloc(num_processors * sizeof(Cache));
	int cache_counter = 0;
	for (cache_counter = 0; cache_counter < num_processors; cache_counter++) {
        Cache cache(cache_size, cache_assoc, blk_size);
        processorArray[cache_counter] = cache;
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
    while (getline(fin, data_segment)) {
        processor = data_segment.at(0) - '0';
        rw = data_segment.at(2);
        addr = strtoul(data_segment.substr(4).c_str(), NULL, 16);

        processorArray[processor].Access(addr, rw);

        switch (protocol) {
            case 0:
                //COHERENCE PROTOCOL: MSI
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
	    processorArray[print_stats].printStats(print_stats);
	}
	//********************************//

	return 0;
}
