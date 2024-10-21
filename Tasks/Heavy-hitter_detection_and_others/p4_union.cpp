#include "./src/ElasticP4/Elastic_P4.h"
#include "./src/common_func.h"
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
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;

int main()
{
    printf("TOTAL_MEMORY %dKB\n", TOT_MEM);
    uint32_t totnum_packet = myReadTraces();
    int _num_heavy_bucket = BUCKET_NUM;
    int _light_mem_byte = (TOT_MEM * 1024) - BUCKET_NUM*8*8;


    ElasticSketch *sketches[2];
    ElasticSketch *elastic = NULL;
	unordered_map<uint32_t, uint32_t> true_freqs[2];
    sketches[0] = new ElasticSketch();
    sketches[1] = new ElasticSketch();
    elastic = new ElasticSketch();
    true_freqs[0].clear();
    true_freqs[1].clear();

    int traceindex = 0;
    int num_pkt = (int)traces[traceindex].size();
    printf("num_pkt: %d\n", num_pkt);
    for (int i = 0; i < num_pkt; ++i)
    {
        if(i%2==0){
            ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[0]->insert((uint8_t*)(traces[traceindex][i].key), 1);
        }
        else{
            ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[1]->insert((uint8_t*)(traces[traceindex][i].key), 1);
        }
    }
	printf("Insertion finished\n");
    printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());
	elastic->Union(*sketches[0], *sketches[1], *elastic);
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
    double totalAAE = 0.0;
    int count = 0;
    for (auto& elem : real_result) {
        int trueFreq = elem.second;
        if(trueFreq == 0) continue;

        int estimatedFreq = elastic->query((uint8_t*)&(elem.first));
        double are = fabs((double)(trueFreq - estimatedFreq) / trueFreq);
        double aae = fabs((double)(trueFreq - estimatedFreq));
        // cout << are << endl;
        totalARE += are;
        totalAAE += aae;
        count++;
    }
    double averageARE = totalARE / count;
    double averageAAE = totalAAE / count;

	cout << "elastic," << averageARE << "," << averageAAE << "," << TOT_MEM << endl;
	delete sketches[0];
    delete sketches[1];
	delete elastic;
}