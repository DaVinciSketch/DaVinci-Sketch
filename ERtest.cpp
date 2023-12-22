#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <string>
#include "ElasticSketch/ElasticSketch.h"
#include <fstream>
#include <iomanip>
#include <cstring>
// #include "Fermat/fermat.h"
// #include "Fermat/fermat_count.h"
#include "ERSketch.h"
// #include "ERSketch_Count.h"
#include "common/BOBHash32.h"
using namespace std;

#define COMPARE_UNORDEREDMAPS

#define START_FILE_NO 1
#define END_FILE_NO 1

#define HEAVY_MEM (150 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
// #define TOT_MEM_IN_BYTES (600 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)

struct FIVE_TUPLE{	char key[13];	};
typedef vector<FIVE_TUPLE> TRACE;
TRACE traces[END_FILE_NO - START_FILE_NO + 1];
int print_dat_num = 13;
int taskno = 0;
// int incluingpacketnum = 2472727;
std::ofstream outfile("outputs/alltasksoutput.csv");
void ReadInTraces(const char *trace_prefix)
{
	int printednum = 0;
	for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt)
	{
		char datafileName[100];
		sprintf(datafileName, "%s%d.dat", trace_prefix, datafileCnt - 1);
		FILE *fin = fopen(datafileName, "rb");

		FIVE_TUPLE tmp_five_tuple;
		traces[datafileCnt - 1].clear();
		while(fread(&tmp_five_tuple, 1, 13, fin) == 13)
		{
			traces[datafileCnt - 1].push_back(tmp_five_tuple);
			// if(printednum < print_dat_num){z
			// 	for (int i = 0; i < 13; ++i) {
			// 		printf("%02x ", (unsigned char)tmp_five_tuple.key[i]);
			// 	}
			// 	printf("\n");
			// 	printednum++;
			// }
		}
		fclose(fin);

		printf("Successfully read in %s, %ld packets\n", datafileName, traces[datafileCnt - 1].size());
	}
	printf("\n");
}

void run_elastic(const std::string &str = NULL){
	
	ElasticSketch<BUCKET_NUM, TOT_MEM_IN_BYTES> *elastic = NULL;
	for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt)
	{
		unordered_map<string, int> Real_Freq;
		elastic = new ElasticSketch<BUCKET_NUM, TOT_MEM_IN_BYTES>();

		auto start_insert = std::chrono::high_resolution_clock::now();
		int packet_cnt = (int)traces[datafileCnt - 1].size();
		for(int i = 0; i < packet_cnt; ++i)
		{
			elastic->insert((uint8_t*)(traces[datafileCnt - 1][i].key));
			// elastic->quick_insert((uint8_t*)(traces[datafileCnt - 1][i].key));

			string str((const char*)(traces[datafileCnt - 1][i].key), 4);
			Real_Freq[str]++;
		}
		auto start_query = std::chrono::high_resolution_clock::now();

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
		outfile << ",,,,,,," << ARE << "," << insert_duration << ",," << query_duration << endl;
	}

	taskno++;
}

void run_flc(bool _fermatcount = 1, bool _addundecodec = 1, int array_num = 3, int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM)/(3 * 8), bool _fing = 0, const std::string &str = NULL){
	// static constexpr int bucket_num = 100000;
	static constexpr int bucket_num = BUCKET_NUM;
	// ElasticSketch<BUCKET_NUM, TOT_MEM_IN_BYTES> *elastic = NULL;
	FLCSketch<bucket_num> *flcsketch = NULL;

	for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt)
	{
		unordered_map<string, int> Real_Freq;
		// elastic = new ElasticSketch<BUCKET_NUM, TOT_MEM_IN_BYTES>();
		flcsketch = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, _fermatcount, _fing, INIT);

		int packet_cnt = (int)traces[datafileCnt - 1].size();
        // packet_cnt = incluingpacketnum;
		// printf("Before Insert\n");
		auto start_insert = std::chrono::high_resolution_clock::now();
		for(int i = 0; i < packet_cnt; ++i)
		{
			// elastic->insert((uint8_t*)(traces[datafileCnt - 1][i].key));
			flcsketch->insert(reinterpret_cast<const char*>(traces[datafileCnt - 1][i].key));
			// elastic->quick_insert((uint8_t*)(traces[datafileCnt - 1][i].key));
			// if(!_fing) printf("After FLC Insert\n");

			string str((const char*)(traces[datafileCnt - 1][i].key), 4);
			Real_Freq[str]++;
		}
		cout<<"Real_Freq.size()"<<Real_Freq.size()<<endl;
		// printf("After Insert\n");
		auto start_decode = std::chrono::high_resolution_clock::now();
		int decodedNum = flcsketch->decode();

		auto start_query = std::chrono::high_resolution_clock::now();
		// printf("Decode over!\n");

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

			//Test partly
			// 将key转换为十六进制字符串
			// std::stringstream ss;
			// ss << std::hex;
			// for (int i = 0; i < 4; ++i) {
			// 	ss << std::setw(2) << std::setfill('0') << (int)key[i];
			// }
			// std::string key_hex = ss.str();

			// int heavy_part_query = (int)GetCounterVal(flcsketch->heavy_part->query(key));

			// outfile << key_hex << ',' << it->second << ',' << est_val << ',' << heavy_part_query << ',';
			// int result_val;
			// if (flcsketch->Eleresult.count(*(uint32_t *)key) > 0) {
			// 	result_val = flcsketch->Eleresult[*(uint32_t *)key];
			// 	outfile << result_val << ',' << 0 <<endl;
			// } else {
			// 	result_val = flcsketch->fermatEle->CountMin_query(reinterpret_cast<const char*>(key));
			// 	outfile << 0 << ',' << result_val << endl;
			// }
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
						// if(real_cnt - heavycnt - elem.second == 0){
						// 	continue;
						// }
						outFile << elem.first << "," << heavycnt << "," << elem.second << ",," << real_cnt << "," << real_cnt - heavycnt - elem.second << ",";

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
						// if(real_cnt - heavycnt - elem.second == 0){
						// 	continue;
						// }
						outFile << elem.first << "," << heavycnt << ",," << elem.second << "," << real_cnt << "," << real_cnt - heavycnt - elem.second << ",";

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
							continue;
						}
						else{
							decode_fal ++;
						}
						outFile << elem.first << "," << heavycnt << "," << elem.second << "," << it->second << "," << real_cnt << "," << real_cnt - heavycnt - flcsketch->Eleresult[elem.first] << ",";

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
		if(_fermatcount) outfile << "yes" << ",";
		else outfile << "no" << ",";
		if(_addundecodec) outfile << "yes" << ",";
		else outfile << "no" << ",";
		
		if(_fing) outfile << "yes" << ',' << array_num << ',';
		else outfile << "no" << ',' << array_num << ',';
		// if(entry_num == (TOT_MEM_IN_BYTES - HEAVY_MEM)) outfile << "Same Entry Num" << ',';
		// else outfile << "Same Mem" << ',';
		outfile << entry_num << ",";
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



		// int decodedNum_sketch = flcsketch_sketch->decode();
		// int decodNum_count = flcsketch_count->decode();

		// double ARE = 0;
		
	    // // outfile << "Key,Real Value,Est Value,Heavy Part Query,Decoded Result,CountMin Query\n";
	
        // // double preARE = 0;
		// for(unordered_map<string, int>::iterator it = Real_Freq.begin(); it != Real_Freq.end(); ++it)
		// {
		// 	uint8_t key[4];
		// 	memcpy(key, (it->first).c_str(), 4);


		// 	int est_val = flcsketch->query(reinterpret_cast<const char*>(key), _addundecodec);

		// 	int dist = std::abs(it->second - est_val);
		// 	ARE += dist * 1.0 / (it->second);

		// }
	

		// delete flcsketch_count;
		// delete flcsketch_sketch;
		// // delete elastic;
		// Real_Freq.clear();
	}
	taskno ++;
}



// void run_er_count(int array_num = 3, int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM)/(3 * 8), bool _fing = 0, const std::string &str = NULL){
// // static constexpr int bucket_num = 100000;
// 	static constexpr int bucket_num = BUCKET_NUM;
// 	// ElasticSketch<BUCKET_NUM, TOT_MEM_IN_BYTES> *elastic = NULL;
// 	FLCSketch<bucket_num> *flcsketch = NULL;

// 	for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt)
// 	{
// 		unordered_map<string, int> Real_Freq;
// 		// elastic = new ElasticSketch<BUCKET_NUM, TOT_MEM_IN_BYTES>();
// 		flcsketch = new FLCSketch<bucket_num>(BUCKET_NUM, array_num, entry_num, _fing, INIT);

// 		int packet_cnt = (int)traces[datafileCnt - 1].size();
//         // packet_cnt = incluingpacketnum;
// 		auto start_insert = std::chrono::high_resolution_clock::now();
// 		for(int i = 0; i < packet_cnt; ++i)
// 		{
// 			// elastic->insert((uint8_t*)(traces[datafileCnt - 1][i].key));
// 			flcsketch->insert(reinterpret_cast<const char*>(traces[datafileCnt - 1][i].key));
// 			// elastic->quick_insert((uint8_t*)(traces[datafileCnt - 1][i].key));

// 			string str((const char*)(traces[datafileCnt - 1][i].key), 4);
// 			Real_Freq[str]++;
// 		}
// 		auto start_decode = std::chrono::high_resolution_clock::now();
// 		flcsketch->decode();
// 		auto start_query = std::chrono::high_resolution_clock::now();
// 		// printf("Decode over!\n");

// 		double ARE = 0;
//         // double preARE = 0;
// 		for(unordered_map<string, int>::iterator it = Real_Freq.begin(); it != Real_Freq.end(); ++it)
// 		{
// 			uint8_t key[4];
// 			memcpy(key, (it->first).c_str(), 4);

// 			int est_val = flcsketch->query(reinterpret_cast<const char*>(key));
// 			int dist = std::abs(it->second - est_val);
// 			ARE += dist * 1.0 / (it->second);

// 		}
// 		auto end_query = std::chrono::high_resolution_clock::now();
// 		// printf("Real_Freq.size(): %d\n",(int)Real_Freq.size());
// 		ARE /= (int)Real_Freq.size();

// // #define HEAVY_HITTER_THRESHOLD(total_packet) (total_packet * 1 / 1000)
// 		printf("%s\n", str.c_str());
// 		printf("FLCSketch %d.dat:\t ARE=%.3lf\t", datafileCnt - 1, ARE);
// 		printf("Insert time: %d\t Decode time: %d\t Query time %d\n", start_decode - start_insert, start_query - start_decode, end_query - start_query);
// 		printf("-------------------------------------------------------------------------------\n");


// 		delete flcsketch;
// 		// delete elastic;
// 		Real_Freq.clear();
// 	}
// }

int main()
{
	ReadInTraces("./datasample/");
	outfile << "UsingFermatCount,AddUndecoded,FingerPrint,ArrayNum,LightEntryNum,DecodedNum,DecodedRate,ARE,InsertTime,DecodeTime,QueryTime\n";
	// cout << 71%11 << " " << (-71)%11 << " " << 71%(-11) << " " << (-71)%(-11) << " " ;
	// run_flc(true, true, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(3 * 12), true, "With finger 3 arrays Same Mem"); //Same Mem
	// run_flc(true, true, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, true, "With finger 3 arrays Same total num of entry"); //Same total num of entry
	// run_flc(true, true, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(3 * 8), false, "No finger 3 arrays and Same Mem"); //Same Mem
	run_flc(true, true, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*1000, false, "No finger 3 arrays and Same single array's num of entry"); //Same single array's num of entry
	// run_flc(true, true, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(1 * 12), true, "With finger 1 array and Same Mem"); //Same Mem
	// run_flc(true, true, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, true, "With finger 1 array and Same total num of entry");//Same total num of entry
	// run_flc(true, true, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(1 * 8), false, "No finger 1 array and Same Mem"); //Same Mem
	// run_flc(true, true, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, false, "No finger 1 Same total num of entry"); //Same total num of entry

	// run_flc(true, false, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(3 * 12), true, "With finger 3 arrays Same Mem"); //Same Mem
	// run_flc(true, false, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, true, "With finger 3 arrays Same total num of entry"); //Same total num of entry
	// run_flc(true, false, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(3 * 8), false, "No finger 3 arrays and Same Mem"); //Same Mem
	// run_flc(true, false, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, false, "No finger 3 arrays and Same single array's num of entry"); //Same single array's num of entry
	// run_flc(true, false, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(1 * 12), true, "With finger 1 array and Same Mem"); //Same Mem
	// run_flc(true, false, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, true, "With finger 1 array and Same total num of entry");//Same total num of entry
	// run_flc(true, false, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(1 * 8), false, "No finger 1 array and Same Mem"); //Same Mem
	// run_flc(true, false, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, false, "No finger 1 Same total num of entry"); //Same total num of entry

	// run_flc(false, true, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(3 * 12), true, "With finger 3 arrays Same Mem"); //Same Mem
	// run_flc(false, true, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, true, "With finger 3 arrays Same total num of entry"); //Same total num of entry
	// run_flc(false, true, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(3 * 8), false, "No finger 3 arrays and Same Mem"); //Same Mem
	// run_flc(false, true, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, false, "No finger 3 arrays and Same single array's num of entry"); //Same single array's num of entry
	// run_flc(false, true, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(1 * 12), true, "With finger 1 array and Same Mem"); //Same Mem
	// run_flc(false, true, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, true, "With finger 1 array and Same total num of entry");//Same total num of entry
	// run_flc(false, true, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(1 * 8), false, "No finger 1 array and Same Mem"); //Same Mem
	// run_flc(false, true, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, false, "No finger 1 Same total num of entry"); //Same total num of entry

	// run_flc(false, false, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(3 * 12), true, "With finger 3 arrays Same Mem"); //Same Mem
	// run_flc(false, false, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, true, "With finger 3 arrays Same total num of entry"); //Same total num of entry
	// run_flc(false, false, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(3 * 8), false, "No finger 3 arrays and Same Mem"); //Same Mem
	// run_flc(false, false, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, false, "No finger 3 arrays and Same single array's num of entry"); //Same single array's num of entry
	// run_flc(false, false, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(1 * 12), true, "With finger 1 array and Same Mem"); //Same Mem
	// run_flc(false, false, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, true, "With finger 1 array and Same total num of entry");//Same total num of entry
	// run_flc(false, false, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10/(1 * 8), false, "No finger 1 array and Same Mem"); //Same Mem
	// run_flc(false, false, 1, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, false, "No finger 1 Same total num of entry"); //Same total num of entry
	// run_elastic("Original Elastic Sketch"); //Original Elastic Sketch

	// two_counters_diff(true, true, 3, (TOT_MEM_IN_BYTES - HEAVY_MEM)*10, false, "No finger 3 arrays and Same single array's num of entry");

	outfile.close();
	
}	