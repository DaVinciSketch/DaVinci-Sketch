#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <string>
#include "../src/Hashpipe/hashpipe.h"
#include <fstream>
#include <iomanip>
#include <cstring>
#include <functional>
#include <typeinfo>
#include <set>
#include "../src/common_func.h"
using namespace std;

// #define COMPARE_UNORDEREDMAPS

#define START_FILE_NO 1
#define END_FILE_NO 1
int total_packet_nums[END_FILE_NO];
int total_flow_nums[END_FILE_NO];

#define INIT_SEED 813

typedef vector<SRCIP_TUPLE> TRACE;

void run_hp(int endfile = 0, const std::string &str = NULL){
	HashPipe *hashpipe = NULL;
	double ave_HH = 0.0;
    double ave_HH_are = 0.0;
    double t = 0.0;
    unordered_map<uint32_t, uint32_t> true_freq;

    for (int times = 0; times < TIMES; times++) {
        hashpipe = new HashPipe(TOT_MEM * 1024);
        true_freq.clear();

        auto start = std::chrono::high_resolution_clock::now();
        for(int datafileCnt = 0; datafileCnt <= endfile; ++datafileCnt)
        {
            int packet_cnt = (int)traces[datafileCnt].size() / 2;
            //printf("datafileCnt = %d\n", datafileCnt);
            for(int i = 0; i < packet_cnt; ++i)
            {
                hashpipe->insert((uint8_t *)(traces[datafileCnt][i].key));
                true_freq[*((uint32_t *)(traces[datafileCnt][i].key))]++;
            }
        }

        unordered_map<uint32_t, int> HH_true;
        for (unordered_map<uint32_t, uint32_t>::iterator it = true_freq.begin(); it != true_freq.end(); ++it)
        {
            uint8_t key[4] = {0};                   // srcIP-flowkey
            uint32_t temp_first = htonl(it->first); // convert uint32_t -> uint8_t * 4 array
            for (int i = 0; i < 4; ++i)
            {
                key[i] = ((uint8_t *)&temp_first)[3 - i];
            }
            if (it->second > HH_THRESHOLD)
                HH_true.insert(make_pair(it->first, it->second));
        }

        /*-*-*-* End of count-query (ARE, AAE) *-*-*-*/
        vector<pair<uint32_t, int>> hashpipe_tmp;
        unordered_map<uint32_t, int> hashpipe_hh;
        hashpipe->get_heavy_hitters(HH_THRESHOLD, hashpipe_tmp);

        auto end = std::chrono::high_resolution_clock::now();

        for (auto f : hashpipe_tmp)
        {
            hashpipe_hh.insert(f);
        }

        double HH_precision = 0;
        double HH_are = 0;
        int HH_PR = 0;
        int HH_PR_denom = 0;
        int HH_RR = 0;
        int HH_RR_denom = 0;
        unordered_map<uint32_t, int>::iterator itr;
        for (itr = HH_true.begin(); itr != HH_true.end(); ++itr)
        {
            HH_PR_denom += 1;
            HH_PR += hashpipe_hh.find(itr->first) != hashpipe_hh.end();
        }
        for (itr = hashpipe_hh.begin(); itr != hashpipe_hh.end(); ++itr)
        {
            HH_RR_denom += 1;
            HH_RR += HH_true.find(itr->first) != HH_true.end();
        }
        for (itr = HH_true.begin(); itr != HH_true.end(); ++itr)
        {
            uint32_t key = itr->first;
            HH_are += std::abs((double)((int)true_freq[key] - (int)hashpipe_hh[key])) / (double)true_freq[key];
        }
        HH_are /= HH_true.size();
        HH_precision = (2 * (double(HH_PR) / double(HH_PR_denom)) * (double(HH_RR) / double(HH_RR_denom))) / ((double(HH_PR) / double(HH_PR_denom)) + (double(HH_RR) / double(HH_RR_denom)));

        ave_HH += HH_precision;
        ave_HH_are += HH_are;
        //printf("average HH F1 score : %3.5f\n", ave_HH);
        //auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        //cout << "diff: " << diff.count() << endl;
        t += diff.count();

        //fflush(stdout);
        delete hashpipe;
    }
    ave_HH /= TIMES;
    t /= TIMES;
    printf("Hashpipe average HH F1 score : %3.5f\n", ave_HH);
    cout << TOT_MEM << "," << ave_HH << "," << t << endl;
}


int main()
{
	ReadNTraces(0);

	unordered_map<uint32_t, uint32_t> total_true_freqs;

	for(int traceindex = 0; traceindex < END_FILE_NO; ++traceindex){
		int current_packet_num = traces[traceindex].size();
		if(traceindex == 0){
			total_packet_nums[traceindex] = current_packet_num;
		}
		else{
			total_packet_nums[traceindex] = total_packet_nums[traceindex - 1] + current_packet_num;
		}
		for (int i = 0; i < current_packet_num; ++i)
		{
			++total_true_freqs[*((uint32_t *)(traces[traceindex][i].key))];
		}
		total_flow_nums[traceindex] = total_true_freqs.size();
	}

	for(int traceindex = 0; traceindex < END_FILE_NO; ++traceindex){
		cout << "Total packet num for first " << traceindex + 1 << " files: " << total_packet_nums[traceindex] << endl;
		cout << "Total flow num for first " << traceindex + 1 << " files: " << total_flow_nums[traceindex] << endl;
	}
    int endfile = 0;
	run_hp(endfile, "Original Hashpipe");

	
}	