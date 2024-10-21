#include "./src/Flowradar/flowradar.h"
// /home/lzx/ERSketch/Tasks/Heavy-hitter_detection_and_others/src/Flowradar/flowradar.h
#include "./src/common_func.h"
#include <iostream>
// std::ofstream outfile1("query_diff1.csv");
// std::ofstream outfile2("query_diff2.csv");
int main()
{
    //printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB,\n", TOT_MEM);
    // uint32_t totnum_packet = ReadTraces();
    uint32_t totnum_packet = myReadTraces();
    //uint32_t totnum_packet = ReadNTraces(10);


    FlowRadar *sketches[2];
	unordered_map<uint32_t, uint32_t> true_freqs[2];
    // FlowRadar(int _len, int c, int bit_num, int f, int _init)
    sketches[0] = new FlowRadar(TOT_MEM*1024*8, 3, 42);//(TOT_MEM * 1024 * 0.9 / 12, 3, TOT_MEM * 1024, 10, INIT);
    sketches[1] = new FlowRadar(TOT_MEM*1024*8, 3, 42);
    true_freqs[0].clear();
    true_freqs[1].clear();

    int traceindex = 0;
    int num_pkt = (int)traces[traceindex].size();
    printf("num_pkt: %d\n", num_pkt);
    for (int i = 0; i < num_pkt; ++i)
    {
        //++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
        //sketches[0]->Insert(*((uint32_t *)(traces[traceindex][i].key)));
        //if(i%2 == 0){
        //    ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
        //    sketches[1]->Insert(*((uint32_t *)(traces[traceindex][i].key)));
        //}
        
        if (i % 3 == 0) {
            ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[0]->Insert(*((uint32_t *)(traces[traceindex][i].key)));
        }
        if (i % 3 == 1) {
            ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[0]->Insert(*((uint32_t *)(traces[traceindex][i].key)));

           ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[1]->Insert(*((uint32_t *)(traces[traceindex][i].key)));
        }
        if (i % 3 == 2) { 
            ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
            sketches[1]->Insert(*((uint32_t *)(traces[traceindex][i].key)));
        }
    }
	//printf("Insertion finished\n");
    printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());

    unordered_map<uint32_t, int> result;
	sketches[0]->diff(*sketches[1]);//, result);
    bool flag = sketches[1]->SingleDecode(result);
    // printf("pure_cnt: %d\n", sketches[1]->pure_cnt);
    // cout << flag << endl;
	// flcsketch = &sketch_result;
	//flcsketch->decode();
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
  double totalAAE = 0.0;
    int count = 0;
    for (auto& elem : real_result) {
        int trueFreq = elem.second;
        if(trueFreq == 0) continue;

        int estimatedFreq = result[elem.first];
        //outfile1 << (elem.first) << "," << estimatedFreq << "," << elem.second << endl;
        double are = fabs((double)(trueFreq - estimatedFreq) / trueFreq);
        double aae = fabs((double)(trueFreq - estimatedFreq));
        totalARE += are;
        totalAAE += aae;
        count++;
    }
    double averageARE = totalARE / count;
    double averageAAE = totalAAE / count;

	//cout << "averageARE: " << averageARE  << endl;
    cout << "flowradar," << averageARE << "," << averageAAE << "," << TOT_MEM << endl;
	  delete sketches[0];
    delete sketches[1];
    //return averageARE;
    return 0;
}

//int main()
//{
//    double total = 0;
//    int times = 1;
//    // for (int i = 0; i < times; i++)
//    // {
//        total = run_flow_diff();
//    // }
//    //cout << "averageARE: " << total / times << endl;
//    cout << "flowradar," << total / times << "," << TOT_MEM << endl;
//    return 0;
//}