#include "./src/common_func.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <iomanip>
#include "./src/others/Choose_Ske.h"
#include <cmath>

int Heavy_Thes=500;
long double _ARE = 0, _AAE = 0;
int main()
{
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB\n", TOT_MEM);
    uint32_t totnum_packet = myReadTraces();

    unordered_map<uint32_t, uint32_t> true_freqs[2];

    int traceindex = 0;
    int num_pkt = (int)traces[traceindex].size();
    printf("num_pkt: %d\n", num_pkt);

    long double Join_Ground_Truth = 0;
    long double Mn=Join_Ground_Truth;
    int d = 3;  //counts of hash function
	int w = TOT_MEM * 1024;  //   bits/counter_size/hash_counts
    int CHOOSE = 1;    //0:joinsketch, 1:fagms, 2:skim
    vector<double>all;
    all.clear();

	for (int i = 0; i < testcycles; ++i)
	{

		Sketch *bcm1,*bcm0;
		bcm0=Choose_Sketch(w,d, i * 123, CHOOSE);
		bcm1=Choose_Sketch(w,d, i * 123, CHOOSE);
        for (int j = 0; j < num_pkt; ++j)
        {
            if(j%2==0){
                ++true_freqs[0][*((uint32_t *)(traces[traceindex][j].key))];
                bcm0->Insert((const char *)(traces[traceindex][j].key));
            }
            else{
                ++true_freqs[1][*((uint32_t *)(traces[traceindex][j].key))];
                bcm1->Insert((const char *)(traces[traceindex][j].key));
            }
        }
		long double my=bcm0->Join(bcm1);

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
	}
	_AAE/=testcycles;
	_ARE/=testcycles;
    cout << "fagms," << _ARE << "," << _AAE << ","  << TOT_MEM <<endl;
}