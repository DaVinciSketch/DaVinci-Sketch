#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <string>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <functional>
#include <typeinfo>
#include <set>
#include "../src/FCMelastic/FCMelastic.h"

// #include "common/BOBHash32.h"
#include "../src/common_func.h"
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;
using namespace std;

// #define COMPARE_UNORDEREDMAPS

#define START_FILE_NO 1
#define END_FILE_NO 1
int total_packet_nums[END_FILE_NO];
int total_flow_nums[END_FILE_NO];

#define HEAVY_MEM (150 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
// #define TOT_MEM_IN_BYTES (600 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)
#define ES_MEM_IN_BYTES (600 * 1024)
#define INIT_SEED 813

typedef vector<SRCIP_TUPLE> TRACE;

int print_dat_num = 13;
int taskno = 0;


void run_fcm(int endfile = 0, const std::string &str = NULL){
	
	FCMPlus *fcm = NULL;
	unordered_map<uint32_t, uint32_t> true_freq;
	double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0, ave_WMRD = 0.0, ave_entr_RE = 0.0;
	double ave_HH_are = 0.0;
    double t = 0.0;
	for (int times = 0; times < TIMES; times++) {
		vector<int> true_dist(1000000);
		fcm = new FCMPlus();
		true_freq.clear();
        
        auto start = std::chrono::high_resolution_clock::now();
		for(int datafileCnt = 0; datafileCnt <= endfile; ++datafileCnt)
		{
			int packet_cnt = (int)traces[datafileCnt].size() / 2;
			for(int i = 0; i < packet_cnt; ++i)
			{
				fcm->insert((uint8_t*)(traces[datafileCnt][i].key));
				//string str((const char*)(traces[datafileCnt][i].key), 4);
				true_freq[*((uint32_t*)(traces[datafileCnt][i].key))]++;
			}
		}
			
		//cout << "Insert FCMPlus over!" << endl;
		double ARE = 0;
		double AAE = 0;
		set<uint32_t> HH_true;
		for (unordered_map<uint32_t, uint32_t>::iterator it = true_freq.begin(); it != true_freq.end(); ++it)
		{
			uint8_t key[4] = {0};                   // srcIP-flowkey
			uint32_t temp_first = htonl(it->first); // convert uint32_t -> uint8_t * 4 array
			for (int i = 0; i < 4; ++i)
			{
				key[i] = ((uint8_t *)&temp_first)[3 - i];
			}
			if (it->second >= true_dist.size())
				true_dist.resize(it->second + 1, 0);
			true_dist[it->second] += 1;
			uint32_t est_val = fcm->query(key);
			int dist = std::abs((int)it->second - (int)est_val);
			ARE += dist * 1.0 / (it->second);
			AAE += dist * 1.0;
			if (it->second > HH_THRESHOLD)
				HH_true.insert(it->first);
		}
        auto end = std::chrono::high_resolution_clock::now();

		ARE /= (int)true_freq.size();
		AAE /= (int)true_freq.size();
		//printf("ARE : %.8lf, AAE : %.8lf\n", ARE, AAE);
		aveare += ARE;
		aveaae += AAE;
		//fflush(stdout);
        //auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        t += diff.count();

		// 清理资源
		delete fcm;
	}
	aveare /= TIMES;
    aveaae /= TIMES;
    t /= TIMES;
	printf("FCMPlus average ARE : %.8lf, average AAE : %.8lf\n", aveare, aveaae);
	cout << TOT_MEM << "," << aveare << "," << t << endl;
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


	for(int endfile = 0; endfile < END_FILE_NO; endfile++){
		run_fcm(endfile, "Original FCMPLUS");
	}
	return 0;
}	