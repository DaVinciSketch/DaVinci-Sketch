#include "./src/DaVinci/DaVinci.h"
#include "./src/common_func.h"
#include <iostream>
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (200 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;

int main()
{
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB, FERMAT_BUCKET %d\n", TOT_MEM, ELE_BUCKET);
    uint32_t totnum_packet = myReadTraces();

    int array_num = 3;
    int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM);
    int _fermatcount = 2; //Use Count version with id+ and cnt +-
    bool _fing = false;

    DaVinci<bucket_num> *davinci = NULL;


    double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0;
    double ave_HH_are = 0.0;
    double ave_WMRD = 0.0, ave_entr_RE = 0.0;
    unordered_map<uint32_t, uint32_t> true_freqs[2];
    for (int times = 0; times < 1; times++)//TIMES; times++)
    {
        std::cout << "times: " << times << std::endl;
        davinci = new DaVinci<bucket_num>(500*1024,37800,3200,269400,3,false,813+times);
        true_freqs[0].clear();
        int allwindowscnt = 1;

        printf("[INFO] %3d-th trace starts to be processed..\n", times);
        unordered_map<uint32_t, uint32_t> &true_freq = true_freqs[0];
        vector<int> true_dist(1);
        int traceindex = 0;
        int num_pkt = (int)traces[traceindex].size();
        printf("num_pkt: %d\n", num_pkt);
        for (int i = 0; i < num_pkt; ++i)
        {
            ++true_freq[*((uint32_t *)(traces[0][i].key))];
            davinci->insert((const char *)(traces[0][i].key), 1);
        }
        cout << "insertion done" << endl;
        davinci->write2file("davinci.txt");
        davinci->tower->printCountersToCSV();

        // 假设array_num是3，entry_num是数组的长度
        int array_num = 3;
        int entry_num = davinci->fermatEle->get_entry_num(); // 你需要提供一个获取entry_num的方法


        davinci->decode(1);
        double temare = 0.0;
        // 创建一个输出文件流对象
        std::ofstream out("davinci_findwhtaresobig.csv");

        // 写入CSV文件的头部
        out << "Key,Estimated,Actual\n";
		for(auto it = true_freq.begin(); it != true_freq.end(); ++it)
        {
            int estimated = (int)davinci->query((const char *)&(it->first), 1);
            int actual = (int)it->second;
            double dist = abs((int)it->second - (int)davinci->query((const char *)&(it->first), 1));
			temare += dist * 1.0 / (it->second);
            if(dist > 100)
                out << it->first << "," << estimated << "," << actual << "\n";
		}

        out.close();

        std::ofstream outFile2("outputs/davinci_result_compare.csv");
        outFile2 << "flowid,real_result,flcsketch_result\n";
        for (const auto& elem : true_freq) {
            int cnt = davinci->query((char*)&(elem.first), true);
            
            if(davinci->decode_track[elem.first][2] != 0)
                outFile2 << uint32_t(elem.first) << "," << elem.second << "," 
                << cnt << "(" << davinci->decode_track[elem.first][0] << "-" << davinci->decode_track[elem.first][1] << "-" << davinci->decode_track[elem.first][2] << "-" << davinci->decode_track[elem.first][3] <<")" << "\n";
        }

        outFile2.close();
        davinci->write2file("sketch_result_afterdecoding.txt");

        temare /= true_freq.size();
        aveare += temare;

        std::cout << times << ": ARE: " << temare << std::endl;

        std::cout << "start to delete" << std::endl;

        std::cout << "deleted" << std::endl;
        davinci = NULL;
    }
    aveare /= TIMES;
    
    
    printf("average ARE : %.8lf\n", aveare);
    printf("\n\n\n");
}