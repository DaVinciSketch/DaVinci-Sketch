#include "../src/Lossradar/lossradar.h"
// /home/lzx/ERSketch/Tasks/Heavy-hitter_detection_and_others/src/Flowradar/flowradar.h
#include "../src/common_func.h"
#include <iostream>
#include <chrono>
//std::ofstream outfile1("query_diff.csv");
int main()
{
    //printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB,\n", TOT_MEM);
    // uint32_t totnum_packet = ReadTraces();
    //uint32_t totnum_packet = myReadTraces();
    //uint32_t totnum_packet = ReadNTraces(10);
    double t = 0.0;
    uint32_t totnum_packet = ReadTwoWindows();

    LossRadar *sketches[2];
	unordered_map<uint32_t, uint32_t> true_freqs[2];
    sketches[0] = new LossRadar(3, TOT_MEM * 1024, INIT);
    sketches[1] = new LossRadar(3, TOT_MEM * 1024, INIT);
    true_freqs[0].clear();
    true_freqs[1].clear();

    int traceindex = 0;
    int num_pkt1 = (int)traces[traceindex].size();
    int num_pkt2 = (int)traces[1].size();
    //int bound = num_pkt / 2;
    printf("num_pkt1: %d\n", num_pkt1);
    printf("num_pkt2: %d\n", num_pkt2);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_pkt1; ++i)
    {
        ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
       sketches[0]->Insert_range_data(*((uint32_t *)(traces[traceindex][i].key)), 1);
    }
    for (int i = 0; i < num_pkt2; ++i)
    {
        ++true_freqs[1][*((uint32_t *)(traces[1][i].key))];
        sketches[1]->Insert_range_data(*((uint32_t *)(traces[1][i].key)), 1);
    }

	//printf("Insertion finished\n");
    printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());

    unordered_map<uint32_t, int> result;
    // sketches[1]->Decode(result);
    // printf("pure_cnt: %d\n", sketches[1]->pure_cnt);
    // return 0;
    sketches[0]->diff(*sketches[1]);
    sketches[1]->Decode(result);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    t = diff.count();

    printf("pure_cnt: %d\n", sketches[1]->pure_cnt);
	unordered_map<uint32_t, int32_t> real_result;
    for (const auto &elem : true_freqs[0])
    {
        if (true_freqs[1].find(elem.first) == true_freqs[1].end())
        {
            real_result[elem.first] = elem.second;
        }
        else
        {
            real_result[elem.first] = elem.second - true_freqs[1][elem.first];
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
    for (auto& elem : real_result) {
        int trueFreq = elem.second;
        if(trueFreq == 0) continue;

        int estimatedFreq = result[elem.first];
        //outfile1 << (elem.first) << "," << estimatedFreq << "," << elem.second << endl;
        double are = fabs((double)(trueFreq - estimatedFreq) / trueFreq);
        // cout << are << endl;
        totalARE += are;
        count++;
    }
    double averageARE = totalARE / count;

	//cout << "averageARE: " << averageARE  << endl;
    //cout << "lossradar," << averageARE << "," << TOT_MEM << endl;
	delete sketches[0];
    delete sketches[1];
    cout << TOT_MEM << "," << averageARE << "," << t << endl;
    //cout << "lossradar," << averageARE << "," << TOT_MEM << endl;
    //return averageARE;
    return 0;
}