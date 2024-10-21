#ifndef _CMSKETCH_H
#define _CMSKETCH_H

#include <sstream>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <vector>
#include "../Common/BOBHash32.h"
#include "../common_func.h"

using namespace std;

class CMSketch
{
private:
	int memory_in_bytes = 0;

	int w = 0;
	int* counters[CM_DEPTH] = {NULL};
	BOBHash32* hash[CM_DEPTH] = {NULL};

public:
    std::set<std::vector<uint8_t> > HH_candidates;
	string name;

	CMSketch(){}
	CMSketch(int memory_in_bytes)
	{	initial(memory_in_bytes);	}
	~CMSketch(){clear();}

	void initial(int memory_in_bytes)
	{
		this->memory_in_bytes = memory_in_bytes;
		w = memory_in_bytes / 4 / CM_DEPTH;
        int init = INIT;
		for(int i = 0; i < CM_DEPTH; ++i){
			counters[i] = new int[w];
			memset(counters[i], 0, 4 * w); // 4*w bytes allocate with filling 0 value.

			hash[i] = new BOBHash32(i + init);
		}


		stringstream name_buffer;
        name_buffer << "CM@" << memory_in_bytes << "@" << CM_DEPTH;
        name = name_buffer.str();
	}
	void clear()
	{
		for(int i = 0; i < CM_DEPTH; ++i)
			delete[] counters[i];

		for (int i = 0; i < CM_DEPTH; ++i)
			delete hash[i];
	}

	void print_basic_info()
    {
        printf("CM sketch\n");
        printf("\tCounters: %d\n", w);
        printf("\tMemory: %.6lfMB\n", w * CM_DEPTH * 4.0 / 1024 / 1024);
        int total_pck = 0;
        for (int dep = 0; dep < CM_DEPTH; ++dep)
            for (int i = 0; i < w; ++i)
                total_pck += counters[dep][i];
            printf("\tTotal_packet at depth %d : %10d\n", CM_DEPTH, total_pck);
    }

    void insert(uint8_t * key, int f = 1)
    {
        std::vector<int> meta_indicator(CM_DEPTH, 0);
        for (int i = 0; i < CM_DEPTH; i++) {
            int index = (hash[i]->run((const char *)key, 4)) % w;
            // int index = (hash[i]->run((const char *)key, 13));
            counters[i][index] += f;
            if (counters[i][index] > HH_THRESHOLD)
                meta_indicator[i] = 1;
        }
        if (std::accumulate(meta_indicator.begin(), meta_indicator.end(), 0) == CM_DEPTH)
            heavy_insert(key);
    }

    void heavy_insert(uint8_t * key)
    {
        std::vector<uint8_t> temp_key(4);
        temp_key.assign(key, key + 4);
        HH_candidates.insert(temp_key);
        return;
    }

	int query(uint8_t * key)
    {
        int ret = 1 << 30; // |flow| < 0100,0000,0000,0000 (16 bits = int)
        for (int i = 0; i < CM_DEPTH; i++) {
        	int index = (hash[i]->run((const char *)key, 4)) % w;
            int tmp = counters[i][index];
            ret = min(ret, tmp);
        }
        return ret;
    }
    
    int get_cardinality()
    {
        int empty = 0;
        for (int i = 0; i < w; ++i)
        {
            if (counters[0][i] == 0)
                empty += 1;
        }
        return (int)(w * std::log(double(w) / double(empty)));
    }

    // w: entry_num
    // CM_DEPTH: array_num
    long double InnerProduct(CMSketch* sketch2)
    {
        long double innerProduct = 0;
        int array_num = CM_DEPTH;
        int entry_num = w;
        long double res[array_num];
        //printf("array_num: %d\n", CM_DEPTH); //3
        //printf("entry_num: %d\n", w); // 51200
        for (int i = 0; i < array_num; i++)
        {
            long k = 0;
            for (int j = 0; j < w; j++)
            {
                k += 1ll * (counters[i][j]) * (sketch2->counters[i][j]);
                //outfile1 << (counters[i][j]) << " " << (sketch2->counters[i][j]) << endl;
                
            }
            //printf("k[%d]: %Ld\n", i, k);
            res[i] = k;
            //printf("res[%d]: %Lf\n", i, res[i]);
        }
        // int d = CM_DEPTH;
        // sort(res, res + d);
		// if (d % 2 == 0)
		// 	return ((res[d / 2] + res[d / 2 - 1]) / 2);
		// else
		// 	return (res[d / 2]);
        long double re = 0;
        for (int i = 0; i < array_num; i++)
        {
            //printf("res[%d]: %Lf\n", i, res[i]);
            re += res[i];
        }
        innerProduct = 1.0 * re / CM_DEPTH;
        return innerProduct;
    }
    
    void Union(CMSketch &sketch2)
    {
        for (int i = 0; i < CM_DEPTH; i++)
        {
            for (int j = 0; j < w; j++)
            {
                //counters[i][j] += sketch2.counters[i][j];
                sketch2.counters[i][j] += counters[i][j];
            }
        }
        return;
    }
};


#endif
