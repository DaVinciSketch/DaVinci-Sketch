#include "./src/DaVinci/DaVinci.h"
#include "./src/common_func.h"
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;

int main()
{
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB, FERMAT_BUCKET %d\n", TOT_MEM, ELE_BUCKET);
    uint32_t totnum_packet = myReadTraces();

    int array_num = 3;
    int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM)*10;
    int _fermatcount = 2; //Use Count version with id+ and cnt +-
    bool _fing = false;
    
    double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0;
    double ave_HH_are = 0.0;
    double ave_WMRD = 0.0, ave_entr_RE = 0.0;
    unordered_map<uint32_t, uint32_t> true_freqs[2];

    uint32_t init_seed = INIT;
    
    unique_ptr<DaVinci<bucket_num>> davinci0 = make_unique<DaVinci<bucket_num>>(500*1024,37800,3200,269400,3,false,813);
    unique_ptr<DaVinci<bucket_num>> davinci1 = make_unique<DaVinci<bucket_num>>(500*1024,37800,3200,269400,3,false,813);
    
    unique_ptr<DaVinci<bucket_num>> flcsketch = make_unique<DaVinci<bucket_num>>(500*1024,37800,3200,269400,3,false,813);
    true_freqs[0].clear();
    true_freqs[1].clear();

    vector<int> true_dist(1);
    int traceindex = 0;
    
    int num_pkt = (int)traces[traceindex].size();
    printf("num_pkt: %d\n", num_pkt);
    int num1, num2 = 0;
    for (int i = 0; i < num_pkt; ++i)
    {
        
        ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
        num1++;
        davinci0->insert((const char *)(traces[traceindex][i].key), 1);
        
        if(i%2==0)
        {
            ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
            num2++;
            davinci1->insert((const char *)(traces[traceindex][i].key), 1);
        }
    }
    printf("Insertion finished\n");
    printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());

    Difference<bucket_num>(*davinci0, *davinci1, *flcsketch, init_seed);

    davinci0->write2file("sketch1.txt");
    davinci1->write2file("sketch2.txt");
    flcsketch->write2file("sketch_result.txt");
    
    flcsketch->decode(1);

    //Real result of difference
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
            real_result[elem.first] = -elem.second;
        }
    }

    //output 2
    
    std::ofstream outFile("output.csv");

    // 首先处理只存在于一个字典中的键
    for (const auto& elem : flcsketch->Eleresult) {
        if (flcsketch->fermatEle->insertedflows.find(elem.first) == flcsketch->fermatEle->insertedflows.end()) {
            // 如果键仅存在于 Eleresult 中
            outFile << elem.first << "," << elem.second << ",\n";
        }
    }

    for (const auto& elem : flcsketch->fermatEle->insertedflows) {
        if (flcsketch->Eleresult.find(elem.first) == flcsketch->Eleresult.end()) {
            // 如果键仅存在于 EleFermatInserted 中
            outFile << elem.first << ",," << elem.second << "\n";
        }
    }

    // 然后处理两个字典中都有的键
    for (const auto& elem : flcsketch->Eleresult) {
        auto it = flcsketch->fermatEle->insertedflows.find(elem.first);
        if (it != flcsketch->fermatEle->insertedflows.end()) {
            // 如果键同时存在于两个字典中
            outFile << elem.first << "," << elem.second << "," << it->second << "\n";
        }
    }
    outFile.close();
    
    davinci0->decode(1);
    davinci1->decode(1);

    //ARE calculation
    double totalARE = 0.0;
    int count = 0;
    for (const auto& elem : real_result) {
        int trueFreq = elem.second;
        if(trueFreq == 0) continue;
        int estimatedFreq = flcsketch->query((char*)&(elem.first), true);
        double are = fabs((double)(trueFreq - estimatedFreq) / trueFreq);
        totalARE += are;
        count++;
    }
    double averageARE = totalARE / count;
    printf("totalARE: %f, count: %d\n", totalARE, count);
    printf("Average ARE: %f\n", averageARE);
    
    std::ofstream outFile2("result_compare.csv");
    outFile2 << "flowid,sketch0real,sketch1real,sketch0decode,sketch1decode,real_result,flcsketch_result\n";
    for (const auto& elem : real_result) {
        int cnt0 = davinci0->query((char*)&(elem.first), true);
        int cnt1 = davinci1->query((char*)&(elem.first), true);
        int cnt = flcsketch->query((char*)&(elem.first), true);
        
        if(flcsketch->decode_track[elem.first][2] != 0)
            outFile2 << uint32_t(elem.first) << "," << true_freqs[0][elem.first] << "," 
            << true_freqs[1][elem.first] << "," 
            << cnt0 << "(" << davinci0->decode_track[elem.first][0] << "-" << davinci0->decode_track[elem.first][1] << "-" << davinci0->decode_track[elem.first][2] << ")" << ","
            << cnt1 << "(" << davinci1->decode_track[elem.first][0] << "-" << davinci1->decode_track[elem.first][1] << "-" << davinci1->decode_track[elem.first][2] << ")" << ","
                << elem.second << "," 
                << cnt << "(" << flcsketch->decode_track[elem.first][0] << "-" << flcsketch->decode_track[elem.first][1] << "-" << flcsketch->decode_track[elem.first][2] <<")" << "\n";
    }

    outFile2.close();
    flcsketch->write2file("sketch_result_afterdecoding.txt");
    
    /*-*-*-* End of packet insertion *-*-*-*/
}