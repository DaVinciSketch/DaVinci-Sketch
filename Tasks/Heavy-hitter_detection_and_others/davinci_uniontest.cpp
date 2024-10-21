#include "./src/DaVinci/DaVinci.h"
#include "./src/common_func.h"
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;

int main()
{
    int totalMem[] = {100*1024, 200*1024, 300*1024, 400*1024, 500*1024, 600*1024};
    int fermatMemList[] = {1800, 3600, 5400, 7200, 9000, 10800};
    int heavyBucketNumList[] = {400, 800, 1200, 1600, 2000, 2400};
    int towerMemList[] = {75000, 150000, 225000, 300000, 375000, 450000};
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
    int test_num = 6;
    std::ofstream outFile3("./outputs/davinci_union_final_result.csv");
    outFile3 << "totalMem,fermatMem,heavyBucketNum,towerMem,averageARE\n";
    for(int i=0; i < test_num; i++){
        uint32_t init_seed = INIT;
        unique_ptr<DaVinci<bucket_num>> davinci0 = make_unique<DaVinci<bucket_num>>(totalMem[i], fermatMemList[i], heavyBucketNumList[i], towerMemList[i], 3, false, 813, 1);
        unique_ptr<DaVinci<bucket_num>> davinci1 = make_unique<DaVinci<bucket_num>>(totalMem[i], fermatMemList[i], heavyBucketNumList[i], towerMemList[i], 3, false, 813, 1);
        unique_ptr<DaVinci<bucket_num>> flcsketch = make_unique<DaVinci<bucket_num>>(totalMem[i], fermatMemList[i], heavyBucketNumList[i], towerMemList[i], 3, false, 813);
        
        
        true_freqs[0].clear();
        true_freqs[1].clear();


        vector<int> true_dist(1);
        int traceindex = 0;

        int num_pkt = (int)traces[traceindex].size();
        printf("num_pkt: %d\n", num_pkt);
        int num1, num2 = 0;
        for (int i = 0; i < num_pkt; ++i)
        {
            if(i%2==0){
                ++true_freqs[0][*((uint32_t *)(traces[traceindex][i].key))];
                num1++;
                davinci0->insert((const char *)(traces[traceindex][i].key), 1);

            }
            else{
                ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
                num2++;
                davinci1->insert((const char *)(traces[traceindex][i].key), 1);

            }
        }
        printf("Insertion finished\n");
        printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());


        Union<bucket_num>(*davinci0, *davinci1, *flcsketch, 813);
       
       
        davinci0->write2file("union_davinci1.txt");
        davinci1->write2file("union_davinci2.txt");
        flcsketch->write2file("union_davinci_result.txt");
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
        
        
        std::ofstream outFile2("./outputs/davinci_union_result_compare.csv");
        outFile2 << "flowid,sketch0real,sketch1real,sketch0decode,sketch1decode,real_result,flcsketch_result\n";
        for (const auto& elem : real_result) {

            int cnt0 = davinci0->query((char*)&(elem.first), true);

            int cnt1 = davinci1->query((char*)&(elem.first), true);
            int cnt = flcsketch->query((char*)&(elem.first), true);
            if(elem.second != cnt)// && elem.second > 0)
            
            
                outFile2 << uint32_t(elem.first) << "," << true_freqs[0][elem.first] << "," 
                << true_freqs[1][elem.first] << "," 
                << cnt0 << "(" << davinci0->decode_track[elem.first][0] << "-" << davinci0->decode_track[elem.first][1] << "-" << davinci0->decode_track[elem.first][2] << ")" << ","
                << cnt1 << "(" << davinci1->decode_track[elem.first][0] << "-" << davinci1->decode_track[elem.first][1] << "-" << davinci1->decode_track[elem.first][2] << ")" << ","
                    << elem.second << "," 
                    << cnt << "(" << flcsketch->decode_track[elem.first][0] << "-" << flcsketch->decode_track[elem.first][1] << "-" << flcsketch->decode_track[elem.first][2] <<")" << "\n";
        }

        outFile2.close();
        flcsketch->write2file("davinci_result_afterdecoding.txt");
        outFile3 << totalMem[i] << "," << fermatMemList[i] << "," << heavyBucketNumList[i] << "," << towerMemList[i] << "," << averageARE << "\n";
        printf("totalARE: %f, count: %d\n", totalARE, count);
        printf("Average ARE: %f\n", averageARE);
        /*-*-*-* End of packet insertion *-*-*-*/

    }
    outFile3.close();


}