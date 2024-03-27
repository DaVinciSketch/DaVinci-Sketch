#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <string>
#include "./src/ElasticSketch/ElasticSketch.h"
#include <fstream>
#include <iomanip>
#include <cstring>
#include <functional>

// #include "common/BOBHash32.h"
#include "./src/ElasticRadar/ERSketch.h"
#include "./src/common_func.h"
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;
using namespace std;

#define COMPARE_UNORDEREDMAPS

#define START_FILE_NO 1
#define END_FILE_NO 11
int total_packet_nums[END_FILE_NO];
int total_flow_nums[END_FILE_NO];

#define HEAVY_MEM (150 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
// #define TOT_MEM_IN_BYTES (600 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)
#define ES_MEM_IN_BYTES (600 * 1024)
#define INIT_SEED 813

// struct FIVE_TUPLE{	char key[13];	};
// typedef vector<FIVE_TUPLE> TRACE;
typedef vector<SRCIP_TUPLE> TRACE;
// TRACE traces[END_FILE_NO - START_FILE_NO + 1];
int print_dat_num = 13;
int taskno = 0;
// int incluingpacketnum = 2472727;
std::ofstream outfile("outputs/sketchtasksoutput.csv");
std::ofstream outfile_set_ops("outputs/setopstasksoutput_union.csv");

void ReadInNTraces(const char *trace_prefix, int n)
{
    if (n < 0 || n > 10) {
        printf("Invalid value of n. It should be between 0 and 10.\n");
        return;
    }

    int printednum = 0;
    for (int datafileCnt = 0; datafileCnt <= n; ++datafileCnt)
    {
        char datafileName[100];
        sprintf(datafileName, "%s%d.dat", trace_prefix, datafileCnt);
        FILE *fin = fopen(datafileName, "rb");

        if (fin == NULL) {
            printf("Failed to open file %s\n", datafileName);
            continue;
        }

        SRCIP_TUPLE tmp_five_tuple;
        traces[datafileCnt].clear();
        while (fread(&tmp_five_tuple, 1, 13, fin) == 13)
        {
            traces[datafileCnt].push_back(tmp_five_tuple);
        }
        fclose(fin);

        printf("Successfully read in %s, %ld packets\n", datafileName, traces[datafileCnt].size());
    }
    printf("\n");
}

void ReadInTraces(const char *trace_prefix)
{
	int printednum = 0;
	for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt)
	{
		char datafileName[100];
		sprintf(datafileName, "%s%d.dat", trace_prefix, datafileCnt - 1);
		FILE *fin = fopen(datafileName, "rb");

		SRCIP_TUPLE tmp_five_tuple;
		traces[datafileCnt - 1].clear();
		while(fread(&tmp_five_tuple, 1, 13, fin) == 13)
		{
			traces[datafileCnt - 1].push_back(tmp_five_tuple);
		}
		fclose(fin);

		printf("Successfully read in %s, %ld packets\n", datafileName, traces[datafileCnt - 1].size());
	}
	printf("\n");
}

void runSetExperiments(std::function<void(int, bool, int, int, bool, int, const std::string&)> run_func, const std::string& taskName) {

    for(int endfile = 5;endfile<END_FILE_NO;endfile++){
		cout << "Running " << taskName << " on first " << endfile << " files" << endl;

		for (int i = 1; i <= 20; i++) {
			run_func(2, true, 3, i * (TOT_MEM_IN_BYTES - HEAVY_MEM), false, endfile, taskName);
		}

		for (int i = -2; i < 3; i++) {
			run_func(2, true, 3, static_cast<int>(pow(10, i) * (TOT_MEM_IN_BYTES - HEAVY_MEM)), false, endfile, taskName);
		}

		for (int i = 1; i <= 20; i++) {
			run_func(2, true, 3, i * (TOT_MEM_IN_BYTES - HEAVY_MEM) / (3 * 8), false, endfile, taskName);
		}

		for (int i = -2; i < 3; i++) {
			run_func(2, true, 3, static_cast<int>(pow(10, i) * (TOT_MEM_IN_BYTES - HEAVY_MEM) / (3 * 8)), false, endfile, taskName);
		}
	}
}

void run_union(int _fermatcount = 2, bool _addundecodec = 1, int array_num = 3, int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM)/(3 * 8), bool _fing = 0, int endfile = 0, const std::string &str = NULL){
	FLCSketch<bucket_num> *sketches[2];
    FLCSketch<bucket_num> *flcsketch = NULL;
	unordered_map<uint32_t, uint32_t> true_freqs[2];
	uint32_t init_seed = INIT_SEED;
    sketches[0] = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, _fermatcount, _fing, init_seed);
    sketches[1] = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, _fermatcount, _fing, init_seed);
    flcsketch = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, _fermatcount, _fing, init_seed);
    true_freqs[0].clear();
    true_freqs[1].clear();

	vector<int> true_dist(1);

	for(int traceindex = 0; traceindex <= endfile; ++traceindex){
		int num_pkt = (int)traces[traceindex].size();
		printf("num_pkt: %d\n", num_pkt);
		for (int i = 0; i < num_pkt; ++i)
		{
			if(i%2==0){
				++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
				sketches[0]->insert((const char *)(traces[traceindex][i].key), 1);
			}
			else{
				++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
				sketches[1]->insert((const char *)(traces[traceindex][i].key), 1);
			}
		}
	}
	printf("Insertion finished\n");
    printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());
	// FLCSketch<bucket_num> sketch_result = Union<bucket_num>(*sketches[0], *sketches[1], init_seed);
	Union<bucket_num>(*sketches[0], *sketches[1], *flcsketch, init_seed);
	// flcsketch = &sketch_result;
	flcsketch->decode();
	unordered_map<uint32_t, int32_t> real_result;
    for (const auto &elem : true_freqs[0])
    {
        if (true_freqs[1].find(elem.first) == true_freqs[1].end())
        {
            real_result[elem.first] = elem.second;
        }
        else
        {
            real_result[elem.first] = elem.second + true_freqs[1][elem.first];
        }
    }
    for (const auto &elem : true_freqs[1])
    {
        if (true_freqs[0].find(elem.first) == true_freqs[0].end())
        {
            real_result[elem.first] = elem.second;
        }
    }
	double totalARE = 0.0;
    int count = 0;
    for (const auto& elem : real_result) {
        int trueFreq = elem.second;
        if(trueFreq == 0) continue;
        int estimatedFreq = flcsketch->query((char*)&(elem.first), true);
        double are = fabs((double)(trueFreq - estimatedFreq) / trueFreq);
        // cout << are << endl;
        totalARE += are;
        count++;
    }
    double averageARE = totalARE / count;

	outfile_set_ops << total_packet_nums[endfile] << "," << total_flow_nums[endfile] << "," << array_num << "," << entry_num << "," << array_num*entry_num*8 << "," << averageARE << "," << str.c_str() << endl;
	delete sketches[0];
    delete sketches[1];
	delete flcsketch;
}

void run_distribution(int _fermatcount = 2, bool _addundecodec = 1, int array_num = 3, int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM)/(3 * 8), bool _fing = 0, int endfile = 0, const std::string &str = NULL){
	FLCSketch<bucket_num> *sketch;
	uint32_t init_seed = INIT_SEED;
	unordered_map<uint32_t, uint32_t> true_freq;
	true_freq.clear();
	sketch = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, _fermatcount, _fing, init_seed);
	vector<int> true_dist(1);
	for(int traceindex = 0; traceindex <= endfile; ++traceindex){
		int num_pkt = (int)traces[traceindex].size();
		printf("num_pkt: %d\n", num_pkt);
		for (int i = 0; i < num_pkt; ++i)
		{
			++true_freq[*((uint32_t *)(traces[traceindex][i].key))];
			sketch->insert((const char *)(traces[traceindex][i].key), 1);
		}
	
	}
	printf("Insertion finished\n");
	vector<double> real_distribution(10, 0);
	for(auto it = true_freq.begin(); it != true_freq.end(); ++it)
    {
        if(real_distribution.size() < it->second + 1)
            real_distribution.resize(it->second + 1);
        real_distribution[it->second]++;
    }

	int index = 0; //calculate according to which array
	vector<double> dist;//[array_num];
	sketch->get_distribution(dist, index);
	
	int minnum = std::min(dist.size(), real_distribution.size());
	long double are = 0;
	for(int i = 0; i < minnum; ++i)
	{
		if(real_distribution[i] != 0)     
			are += std::abs(dist[i] - real_distribution[i]) / real_distribution[i];
		else
			are += dist[i];
	}
	outfile_set_ops << total_packet_nums[endfile] << "," << total_flow_nums[endfile] << "," << array_num << "," << entry_num << "," << array_num*entry_num*8 << "," << are / minnum << "," << str.c_str() << endl;
	// cout << index << "th array ARE: " << are / minnum << endl;
	delete sketch;
}


void run_innerproduct(int _fermatcount = 2, bool _addundecodec = 1, int array_num = 3, int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM)/(3 * 8), bool _fing = 0, int endfile = 0, const std::string &str = NULL){
	FLCSketch<bucket_num> *sketches[2];
	unordered_map<uint32_t, uint32_t> true_freqs[2];
	uint32_t init_seed = INIT_SEED;
    sketches[0] = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, _fermatcount, _fing, init_seed);
    sketches[1] = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, _fermatcount, _fing, init_seed);
    true_freqs[0].clear();
    true_freqs[1].clear();
	vector<int> true_dist(1);
	for(int traceindex = 0; traceindex <= endfile; ++traceindex){
		int num_pkt = (int)traces[traceindex].size();
		printf("num_pkt: %d\n", num_pkt);
		for (int i = 0; i < num_pkt; ++i)
		{
			if(i%2==0){
				++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
				sketches[0]->insert((const char *)(traces[traceindex][i].key), 1);
			}
			else{
				++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
				sketches[1]->insert((const char *)(traces[traceindex][i].key), 1);
			}
		}
	}

	long double calculated_innerproduct = InnerProduct<bucket_num>(*sketches[0], *sketches[1]);
	long double real_innerproduct = 0;
	for (auto it = true_freqs[0].begin(); it != true_freqs[0].end(); ++it)
    {
        real_innerproduct += (long double)it->second * (long double)true_freqs[1][it->first];
    }
	outfile_set_ops << total_packet_nums[endfile] << "," << total_flow_nums[endfile] << "," << array_num << "," << entry_num << "," << array_num*entry_num*8 << "," << (real_innerproduct - calculated_innerproduct) / real_innerproduct << "," << str.c_str() << endl;
	delete sketches[0];
    delete sketches[1];

}


void run_elastic(int _num_heavy_bucket = BUCKET_NUM, int _light_mem_byte = TOT_MEM_IN_BYTES - BUCKET_NUM*8*8, const std::string &str = NULL){
	
	ElasticSketch<BUCKET_NUM, TOT_MEM_IN_BYTES> *elastic = NULL;
	for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt)
	{
		unordered_map<string, int> Real_Freq;
		cout << "Gonna init ElasticSketch!" << endl;
		elastic = new ElasticSketch<BUCKET_NUM, TOT_MEM_IN_BYTES>(_num_heavy_bucket, _light_mem_byte);
		cout << "Init ElasticSketch over!" << endl;

		auto start_insert = std::chrono::high_resolution_clock::now();
		int packet_cnt = (int)traces[datafileCnt - 1].size() / 2;
		for(int i = 0; i < packet_cnt; ++i)
		{
			elastic->insert((uint8_t*)(traces[datafileCnt - 1][i].key));
			string str((const char*)(traces[datafileCnt - 1][i].key), 4);
			Real_Freq[str]++;
		}
		auto start_query = std::chrono::high_resolution_clock::now();
		
		cout << "Insert ElasticSketch over!" << endl;

		double ARE = 0;
		for(unordered_map<string, int>::iterator it = Real_Freq.begin(); it != Real_Freq.end(); ++it)
		{
			uint8_t key[4];
			memcpy(key, (it->first).c_str(), 4);
			int est_val = elastic->query(key);
			int dist = std::abs(it->second - est_val);
			ARE += dist * 1.0 / (it->second);
		}
		auto end_query = std::chrono::high_resolution_clock::now();
		ARE /= (int)Real_Freq.size();
		
		printf("%s\n", str.c_str());
		auto insert_duration = std::chrono::duration_cast<std::chrono::microseconds>(start_query - start_insert).count();
		
		auto query_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_query - start_query).count();

		
		printf("Elastic Sketch %d.dat:\t ARE=%.3lf\t", datafileCnt - 1, ARE);
		printf("Insert time: %d\t\t\t\t Query time %d\n", insert_duration, query_duration);
		printf("-------------------------------------------------------------------------------\n");
		delete elastic;
		Real_Freq.clear();
		outfile << "ElasticSketch,-,-,-,1," << _light_mem_byte << "," << _light_mem_byte << ",-,-," << ARE << "," << insert_duration << ",," << query_duration << endl;
	}

	taskno++;
}
void run_flc(int _fermatcount = 2, bool _addundecodec = 1, int array_num = 3, int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM)/(3 * 8), bool _fing = 0, const std::string &str = NULL){
	static constexpr int bucket_num = BUCKET_NUM;
	FLCSketch<bucket_num> *flcsketch = NULL;

	for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt)
	{
		unordered_map<string, int> Real_Freq;
		flcsketch = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, _fermatcount, _fing, INIT);

		int packet_cnt = (int)traces[datafileCnt - 1].size() / 2;
		auto start_insert = std::chrono::high_resolution_clock::now();
		for(int i = 0; i < packet_cnt; ++i)
		{
			flcsketch->insert(reinterpret_cast<const char*>(traces[datafileCnt - 1][i].key));
			string str((const char*)(traces[datafileCnt - 1][i].key), 4);
			Real_Freq[str]++;
		}
		cout<<"Real_Freq.size()"<<Real_Freq.size()<<endl;
		// printf("After Insert\n");
		auto start_decode = std::chrono::high_resolution_clock::now();
		int decodedNum = flcsketch->decode();

		auto start_query = std::chrono::high_resolution_clock::now();
		double ARE = 0;
	    // outfile << "Key,Real Value,Est Value,Heavy Part Query,Decoded Result,CountMin Query\n";
        // double preARE = 0;
		for(unordered_map<string, int>::iterator it = Real_Freq.begin(); it != Real_Freq.end(); ++it)
		{
			uint8_t key[4];
			memcpy(key, (it->first).c_str(), 4);
			int est_val = flcsketch->query(reinterpret_cast<const char*>(key), _addundecodec);

			int dist = std::abs(it->second - est_val);
			ARE += dist * 1.0 / (it->second);
		}
		
		auto end_query = std::chrono::high_resolution_clock::now();
		#ifdef COMPARE_UNORDEREDMAPS
				std::ofstream outFile("outputs/output_.csv");
				outFile << "key,heavyCnt,decodedCnt,insertedCnt,realCnt,realCnt-heavyCnt-insertedCnt\n";

				// 首先处理只存在于一个字典中的键
				for (const auto& elem : flcsketch->Eleresult) {
					if (flcsketch->fermatEle->insertedflows.find(elem.first) == flcsketch->fermatEle->insertedflows.end()) {
						// 如果键仅存在于 Eleresult 中
						int heavycnt = (int)GetCounterVal(flcsketch->heavy_part->query((uint8_t*)(&(elem.first))));
						string str((const char*)(&(elem.first)), 4);
						int real_cnt = Real_Freq[str];
						outFile << (uint32_t)elem.first << "," << heavycnt << "," << elem.second << ",," << real_cnt << "," << real_cnt - heavycnt - elem.second << ",";

						uint32_t key = elem.first; // 指定您想要输出的键
						// 检查 key 是否存在
						if (flcsketch->insert_tracking.find(key) != flcsketch->insert_tracking.end()) {
							// 遍历对应 key 的 vector<pair<int, int>>
							for (const auto& elem : flcsketch->insert_tracking[key]) {
								outFile << "(" << elem.first << ";" << elem.second << ")";
							}
						}
						outFile <<"\n";

					}
				}

				for (const auto& elem : flcsketch->fermatEle->insertedflows) {
					if (flcsketch->Eleresult.find(elem.first) == flcsketch->Eleresult.end()) {
						// 如果键仅存在于 EleFermatInserted 中
						int heavycnt = (int)GetCounterVal(flcsketch->heavy_part->query((uint8_t*)(&(elem.first))));
						string str((const char*)(&(elem.first)), 4);
						int real_cnt = Real_Freq[str];
						outFile << (uint32_t)elem.first << "," << heavycnt << ",," << elem.second << "," << real_cnt << "," << real_cnt - heavycnt << ",";

						uint32_t key = elem.first; // 指定您想要输出的键
						// 检查 key 是否存在
						if (flcsketch->insert_tracking.find(key) != flcsketch->insert_tracking.end()) {
							// 遍历对应 key 的 vector<pair<int, int>>
							for (const auto& elem : flcsketch->insert_tracking[key]) {
								outFile << "(" << elem.first << ";" << elem.second << ")";
							}
						}
						outFile <<"\n";
					}
				}

				// 然后处理两个字典中都有的键
				bool flag = 0;
				int decode_cor = 0;
				int decode_fal = 0;
				for (const auto& elem : flcsketch->Eleresult) {
					auto it = flcsketch->fermatEle->insertedflows.find(elem.first);
					if (it != flcsketch->fermatEle->insertedflows.end()) {
						int dis = flcsketch->Eleresult[elem.first] - flcsketch->fermatEle->insertedflows[elem.first];
						if(dis){
							flag = 1;
							//cout<<" Different in two unorderedmaps: " << dis << endl;
						}
						int heavycnt = (int)GetCounterVal(flcsketch->heavy_part->query((uint8_t*)(&(elem.first))));
						string str((const char*)(&(elem.first)), 4);
						int real_cnt = Real_Freq[str];
						// 如果键同时存在于两个字典中
						if(real_cnt - heavycnt - flcsketch->Eleresult[elem.first] == 0){
							decode_cor ++;
							// continue;
						}
						else{
							decode_fal ++;
						}
						outFile << (uint32_t)elem.first << "," << heavycnt << "," << elem.second << "," << it->second << "," << real_cnt << "," << real_cnt - heavycnt - flcsketch->Eleresult[elem.first] << ",";

						uint32_t key = elem.first; // 指定想要输出的键
						// 检查 key 是否存在
						if (flcsketch->insert_tracking.find(key) != flcsketch->insert_tracking.end()) {
							// 遍历对应 key 的 vector<pair<int, int>>
							for (const auto& elem : flcsketch->insert_tracking[key]) {
								outFile << "(" << elem.first << ";" << elem.second << ")";
							}
						}
						outFile <<"\n";
					}
				}
				cout << "Decoded correct num: " << decode_cor << " Decoded false num: " << decode_fal << endl;
				

				if(!flag){
					cout<<"These two unordered maps are always the same!\n";
				}
				outFile.close();
		#endif

		auto insert_duration = std::chrono::duration_cast<std::chrono::microseconds>(start_decode - start_insert).count();
		auto decode_duration = std::chrono::duration_cast<std::chrono::microseconds>(start_query - start_decode).count();
		auto query_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_query - start_query).count();

		// printf("Real_Freq.size(): %d\n",(int)Real_Freq.size());
		ARE /= (int)Real_Freq.size();
		int cnt_fermat_inserted = flcsketch->fermatEle->insertedflows.size();
		double decodesrate = 100 * decodedNum / cnt_fermat_inserted;
		cout << "decodedNum: " << decodedNum << ", cnt_fermat_inserted: " << cnt_fermat_inserted << endl;
		outfile << "Radar,";
		if(_fermatcount == 1) outfile << "id/cnt+-" << ",";
		else if(_fermatcount == 2){
			outfile << "id+cnt+-" << ",";
		}
		else outfile << "no" << ",";
		if(_addundecodec) outfile << "yes" << ",";
		else outfile << "no" << ",";
		
		if(_fing) outfile << "yes" << ',' << array_num << ',';
		else outfile << "no" << ',' << array_num << ',';
		// if(entry_num == (TOT_MEM_IN_BYTES - HEAVY_MEM)) outfile << "Same Entry Num" << ',';
		// else outfile << "Same Mem" << ',';
		outfile << entry_num << "," << array_num*entry_num*8 << ",";
		outfile << decodedNum << ',' << decodesrate << '%' << ',' << ARE << ',' << insert_duration << ',' << decode_duration << ',' << query_duration << endl;


// #define HEAVY_HITTER_THRESHOLD(total_packet) (total_packet * 1 / 1000)
		printf("%s\n", str.c_str());
		printf("FLCSketch %d.dat:\t ARE=%.3lf\t", datafileCnt - 1, ARE);
		printf("Insert time: %d\t Decode time: %d\t Query time %d\n", insert_duration, decode_duration, query_duration);
		printf("-------------------------------------------------------------------------------\n");


		delete flcsketch;
		// delete elastic;
		Real_Freq.clear();
	}
	taskno ++;
}
void two_counters_diff(bool _fermatcount = 1, bool _addundecodec = 1, int array_num = 3, int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM)/(3 * 8), bool _fing = 0, const std::string &str = NULL){
	// static constexpr int bucket_num = 100000;
	static constexpr int bucket_num = BUCKET_NUM;
	// ElasticSketch<BUCKET_NUM, TOT_MEM_IN_BYTES> *elastic = NULL;
	FLCSketch<bucket_num> *flcsketch_sketch = NULL;
	FLCSketch<bucket_num> *flcsketch_count = NULL;
	std::ofstream outfile_diff("outputs/two_counters_diff.csv");

	for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt)
	{
		unordered_map<string, int> Real_Freq;
		// elastic = new ElasticSketch<BUCKET_NUM, TOT_MEM_IN_BYTES>();
		flcsketch_sketch = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, 0, _fing, 813);
		flcsketch_count = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, 1, _fing, 813);

		int packet_cnt = (int)traces[datafileCnt - 1].size();
        
		for(int i = 0; i < packet_cnt; ++i)
		{
			flcsketch_sketch->insert(reinterpret_cast<const char*>(traces[datafileCnt - 1][i].key));
			flcsketch_count->insert(reinterpret_cast<const char*>(traces[datafileCnt - 1][i].key));

			string str((const char*)(traces[datafileCnt - 1][i].key), 4);
			Real_Freq[str]++;
		}
		cout<<"Real_Freq.size()"<<Real_Freq.size()<<endl;
		outfile_diff << "array" << ",entry" << ",sketchid" << ",countid" << ",iddiff" << ",sketchcounters" << ",countcounters" << ",countersdiff" <<endl;
		int id_diffcnt = 0;
		int count_diffcnt = 0;
		for(int i = 0;i<array_num;i++){
			for(int j=0;j<entry_num;j++){
				int countid = flcsketch_count->fermatEle->get_id(i, j);
				int sketchid = flcsketch_sketch->fermatEle->get_id(i, j);
				int countcounters = flcsketch_count->fermatEle->get_counter(i, j);
				int sketchcounters = flcsketch_sketch->fermatEle->get_counter(i, j);
				if(sketchid - countid == 0 && sketchcounters - countcounters == 0) {
					if(j < 100){
						outfile_diff << i << "," << j << ","\
				<< sketchid << "," << countid << "," << sketchid - countid << ","\
				<< sketchcounters << "," <<countcounters << "," << sketchcounters - countcounters << endl;

					}
					continue;
				}

				outfile_diff << i << "," << j << ","\
				<< sketchid << "," << countid << "," << sketchid - countid << ","\
				<< sketchcounters << "," <<countcounters << "," << sketchcounters - countcounters << endl;

				if(sketchid - countid != 0){
					id_diffcnt ++;
					// cout<< i << " " << j << "id different!" << sketchid << "," << countid << "," << sketchid - countid <<endl;
				}
				if(sketchcounters - countcounters != 0){
					count_diffcnt ++;
					// cout<< i << " " << j << "counter different!" << sketchcounters << "," << countcounters << "," << sketchcounters - countcounters <<endl;
				}
			}
		}
		cout<< "id diff count: " << id_diffcnt << " count diff count: " << count_diffcnt <<endl;
		outfile_diff.close();
		delete flcsketch_count;
		delete flcsketch_sketch;
		Real_Freq.clear();

		return;

	}
	taskno ++;
}


int main()
{
	ReadInNTraces("/home/FLC/ElasticSketchCode/data/", 10);
	outfile << "PacketNum,FlowNum,Type,UsingFermatCount,AddUndecoded,FingerPrint,ArrayNum,LightEntryNum,LightTotalMem,DecodedNum,DecodedRate,ARE,InsertTime,DecodeTime,QueryTime\n";
	outfile_set_ops << "PacketNum,FlowNum,ArrayNum,LightEntryNum,LightTotalMem,ARE,Task\n";

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


	runSetExperiments(run_union, "Union");
	// runSetExperiments(run_difference, "Difference");
	// runSetExperiments(run_distribution, "Distribution");
	return 0;
	

	// for(int endfile = 0; endfile < END_FILE_NO; endfile++){
	// 	total_packet_nums += traces[endfile].size();
	// 	cout << "[INFO] Start Set Operartion tasks!" << endl;
	// 	for(int i = 1;i<=20;i++){
	// 		run_union(2, true, 3, i*(TOT_MEM_IN_BYTES - HE1AVY_MEM), false, endfile, "3 arrays and Same Entry Num per Array");	
	// 	}
	// 	for(int i = -2;i<3;i++){
	// 		run_union(2, true, 3, (int)((pow(10, i))*(TOT_MEM_IN_BYTES - HEAVY_MEM)), false, endfile, "3 arrays and Same Entry Num per Array");
	// 	}
	// 	for(int i = 1;i<=20;i++){
	// 		run_union(2, true, 3, i*(TOT_MEM_IN_BYTES - HEAVY_MEM)/(3*8), false, endfile, "3 arrays and Same Entry Num per Array");
	// 	}
	// 	for(int i = -2;i<3;i++){
	// 		run_union(2, true, 3, (int)((pow(10, i))*(TOT_MEM_IN_BYTES - HEAVY_MEM)/(3*8)), false, endfile, "3 arrays and Same Entry Num per Array");
	// 	}


	// 	cout << "[INFO] Start Sketch Operartion tasks!" << endl;
	// 	for(int i = 1;i<=20;i++){
	// 		run_flc(2, true, 3, i*(TOT_MEM_IN_BYTES - HEAVY_MEM), false, "No finger 3 arrays and Same Entry Num per Array"); //Same single array's num of entry
	// 		run_flc(true, true, 3, i*(TOT_MEM_IN_BYTES - HEAVY_MEM), false, "No finger 3 arrays and Same Entry Num per Array"); //Same single array's num of entry
	// 		// run_flc(false, true, 3, i*(TOT_MEM_IN_BYTES - HEAVY_MEM), false, "No finger 3 arrays and Same Entry Num per Array"); //Same single array's num of entry
			
	// 		run_elastic(BUCKET_NUM, i*(TOT_MEM_IN_BYTES - HEAVY_MEM), "Original Elastic Sketch"); //Original Elastic Sketch
			
	// 	}
	// 	for(int i = -2;i<3;i++){
	// 		run_flc(2, true, 3, (int)((pow(10, i))*(TOT_MEM_IN_BYTES - HEAVY_MEM)), false, "No finger 3 arrays and Entry Num per Array"); //Same single array's num of entry
	// 		run_flc(true, true, 3, (int)((pow(10, i))*(TOT_MEM_IN_BYTES - HEAVY_MEM)), false, "No finger 3 arrays and Entry Num per Array"); //Same single array's num of entry
	// 		// run_flc(false, true, 3, (int)((pow(10, i))*(TOT_MEM_IN_BYTES - HEAVY_MEM)), false, "No finger 3 arrays and Entry Num per Array"); //Same single array's num of entry
			
	// 		run_elastic(BUCKET_NUM, (int)((pow(10, i))*(TOT_MEM_IN_BYTES - HEAVY_MEM)), "Original Elastic Sketch"); //Original Elastic Sketch
	// 	}
	// 	for(int i = 1;i<=20;i++){
	// 		run_flc(2, true, 3, i*(TOT_MEM_IN_BYTES - HEAVY_MEM)/(3*8), false, "No finger 3 arrays and Same Mem"); //Same single array's num of entry
	// 		run_flc(true, true, 3, i*(TOT_MEM_IN_BYTES - HEAVY_MEM)/(3*8), false, "No finger 3 arrays and Same Mem"); //Same single array's num of entry
	// 		// run_flc(false, true, 3, i*(TOT_MEM_IN_BYTES - HEAVY_MEM)/(3*8), false, "No finger 3 arrays and Same Mem"); //Same single array's num of entry
	// 		run_elastic(BUCKET_NUM, i*(TOT_MEM_IN_BYTES - HEAVY_MEM), "Original Elastic Sketch"); //Original Elastic Sketch
	// 	}
	// 	for(int i = -2;i<3;i++){
	// 		run_flc(2, true, 3, (int)((pow(10, i))*(TOT_MEM_IN_BYTES - HEAVY_MEM)/(3*8)), false, "No finger 3 arrays and Same Mem"); //Same single array's num of entry
	// 		run_flc(true, true, 3, (int)((pow(10, i))*(TOT_MEM_IN_BYTES - HEAVY_MEM)/(3*8)), false, "No finger 3 arrays and Same Mem"); //Same single array's num of entry
	// 		// run_flc(false, true, 3, (int)((pow(10, i))*(TOT_MEM_IN_BYTES - HEAVY_MEM)/(3*8)), false, "No finger 3 arrays and Same Mem"); //Same single array's num of entry
	// 		run_elastic(BUCKET_NUM, (int)((pow(10, i))*(TOT_MEM_IN_BYTES - HEAVY_MEM)), "Original Elastic Sketch"); //Original Elastic Sketch
	// 	}
	// }

	outfile.close();
	
}	