#include "./src/DaVinci/DaVinci.h"
#include "./src/common_func.h"
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;
// template class HeavyPart<2400>;
int main()
{
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB, FERMAT_BUCKET %d\n", TOT_MEM, ELE_BUCKET);
    // uint32_t totnum_packet = ReadTraces();
    // uint32_t totnum_packet = myReadTraces();
    uint32_t totnum_packet = ReadNTraces(10);

    int array_num = 3;
    int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM)*10;
    int _fermatcount = 2; //Use Count version with id+ and cnt +-
    bool _fing = false;

    DaVinci<bucket_num> *sketches[2];
    DaVinci<bucket_num> *flcsketch = NULL;

    // Fermat_tower *sketches[2];
    // Fermat_tower *fermat_tower = NULL;
    double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0;
    double ave_HH_are = 0.0;
    double ave_WMRD = 0.0, ave_entr_RE = 0.0;
    unordered_map<uint32_t, uint32_t> true_freqs[2];

    uint32_t init_seed = INIT;
    sketches[0] = new DaVinci<bucket_num>();
    sketches[1] = new DaVinci<bucket_num>();
    flcsketch = new DaVinci<bucket_num>();

    true_freqs[0].clear();
    true_freqs[1].clear();

    // printf("[INFO] %3d-th trace starts to be processed..\n", times);
    // unordered_map<uint32_t, int32_t> &true_freq = true_freqs[iter_data];
    vector<int> true_dist(1);
    int traceindex = 0;
    // flcsketch= sketches[iter_data];
    int num_pkt = (int)traces[traceindex].size();
    printf("num_pkt: %d\n", num_pkt);
    int num1, num2 = 0;
    for (int i = 0; i < num_pkt; ++i)
    {
        if(i%2==0){
            ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
            num1++;
            sketches[0]->insert((const char *)(traces[traceindex][i].key), 1);
        }
        else{
            ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
            num2++;
            sketches[1]->insert((const char *)(traces[traceindex][i].key), 1);
        }
    }
    printf("Insertion finished\n");
    printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());

    Union<bucket_num>(*sketches[0], *sketches[1], *flcsketch, init_seed);
    // return 0;
    sketches[0]->write2file("union_davinci1.txt");
    sketches[1]->write2file("union_davinci2.txt");
    flcsketch->write2file("union_davinci_result.txt");
    flcsketch->decode();

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

    sketches[0]->decode();
    sketches[1]->decode();

    //ARE calculation
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
    
    std::ofstream outFile2("./outputs/union_result_compare.csv");
    outFile2 << "flowid,sketch0real,sketch1real,sketch0decode,sketch1decode,real_result,flcsketch_result\n";
    for (const auto& elem : real_result) {
        int cnt0 = sketches[0]->query((char*)&(elem.first), true);
        int cnt1 = sketches[1]->query((char*)&(elem.first), true);
        int cnt = flcsketch->query((char*)&(elem.first), true);
        if(elem.second != cnt)// && elem.second > 0)
        // if(uint32_t(elem.first) == 3948909411 || elem.second != cnt)// && elem.second > 0)
        // if(flcsketch->decode_track[elem.first][2] != 0)// && elem.second > 0)
            outFile2 << uint32_t(elem.first) << "," << true_freqs[0][elem.first] << "," 
            << true_freqs[1][elem.first] << "," 
            << cnt0 << "(" << sketches[0]->decode_track[elem.first][0] << "-" << sketches[0]->decode_track[elem.first][1] << "-" << sketches[0]->decode_track[elem.first][2] << ")" << ","
            << cnt1 << "(" << sketches[1]->decode_track[elem.first][0] << "-" << sketches[1]->decode_track[elem.first][1] << "-" << sketches[1]->decode_track[elem.first][2] << ")" << ","
                << elem.second << "," 
                << cnt << "(" << flcsketch->decode_track[elem.first][0] << "-" << flcsketch->decode_track[elem.first][1] << "-" << flcsketch->decode_track[elem.first][2] <<")" << "\n";
    }

    outFile2.close();
    flcsketch->write2file("davinci_result_afterdecoding.txt");
    
    printf("totalARE: %f, count: %d\n", totalARE, count);
    printf("Average ARE: %f\n", averageARE);
    /*-*-*-* End of packet insertion *-*-*-*/


    delete sketches[0];
    delete sketches[1];

}