#include "../src/common_func.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <iomanip>
#include "../src/others/Choose_Ske.h"
#include <cmath>
#include <chrono>

int Heavy_Thes=500;
int main()
{
    long double _ARE = 0, _AAE = 0;
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB\n", TOT_MEM);
    uint32_t totnum_packet = ReadTwoWindows();

    unordered_map<uint32_t, uint32_t> true_freqs[2];

    int traceindex = 0;
    int num_pkt1 = (int)traces[0].size();
    printf("num_pkt1: %d\n", num_pkt1);
	int num_pkt2 = (int)traces[1].size();
    printf("num_pkt2: %d\n", num_pkt2);

    long double Join_Ground_Truth = 0;
    long double Mn=Join_Ground_Truth;
    int d = 3;  //counts of hash function
	int w = TOT_MEM * 1024;  //   bits/counter_size/hash_counts
    int CHOOSE = 0;    //0:joinsketch, 1:fagms, 2:skim
	double t = 0.0;
    // vector<double>all;
    // all.clear();

	for (int i = 0; i < testcycles; ++i)
	{

		Sketch *bcm1,*bcm0;
		bcm0=Choose_Sketch(w,d, i * 123, CHOOSE);
		bcm1=Choose_Sketch(w,d, i * 123, CHOOSE);
		auto start = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < num_pkt1; ++j)
        {
            ++true_freqs[0][*((uint32_t *)(traces[0][j].key))];
            bcm0->Insert((const char *)(traces[0][j].key));
        }
		for (int j = 0; j < num_pkt2; ++j)
        {
            ++true_freqs[1][*((uint32_t *)(traces[1][j].key))];
            bcm1->Insert((const char *)(traces[1][j].key));
        }
		long double my=bcm0->Join(bcm1);
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = end - start;
		if (i == 0)
		{
			long double real_innerproduct = 0;
			for (auto it = true_freqs[0].begin(); it != true_freqs[0].end(); ++it)
			{
				real_innerproduct += (long double)it->second * (long double)true_freqs[1][it->first];
			}
			Join_Ground_Truth = real_innerproduct;
			Mn=Join_Ground_Truth;
		}

		_AAE+=abs(my-Join_Ground_Truth);
		_ARE+=1.0*abs(my-Join_Ground_Truth)/Join_Ground_Truth;
        t += diff.count();
	}
	_AAE/=testcycles;
	_ARE/=testcycles;
	t /= testcycles;
    cout << TOT_MEM << "," << _ARE << "," << t << endl;
}