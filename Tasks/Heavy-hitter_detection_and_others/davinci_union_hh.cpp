#include "./src/DaVinci/DaVinci.h"
#include "./src/common_func.h"
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (600 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;
// template class HeavyPart<2400>;
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
    unordered_map<uint32_t, uint32_t> true_freqs[3];
    int test_num = 6;
    std::ofstream outFile3("./outputs/davinci_union_hh_result.csv");
    outFile3 << "totalMem,fermatMem,heavyBucketNum,towerMem,HHF1,UnionHHF1,HHAre,UnionHHAre,HHAae,UnionHHAae,MissRate,UnionMissRate,TopK\n";
    for(int i=0; i < test_num; i++){
        uint32_t init_seed = INIT;
        unique_ptr<DaVinci<bucket_num>> davinci0 = make_unique<DaVinci<bucket_num>>(totalMem[i], fermatMemList[i], heavyBucketNumList[i], towerMemList[i], 3, false, 813, 1);
        unique_ptr<DaVinci<bucket_num>> davinci1 = make_unique<DaVinci<bucket_num>>(totalMem[i], fermatMemList[i], heavyBucketNumList[i], towerMemList[i], 3, false, 813, 1);
        unique_ptr<DaVinci<bucket_num>> davinci01 = make_unique<DaVinci<bucket_num>>(totalMem[i], fermatMemList[i], heavyBucketNumList[i], towerMemList[i], 3, false, 813);
        unique_ptr<DaVinci<bucket_num>> flcsketch = make_unique<DaVinci<bucket_num>>(totalMem[i], fermatMemList[i], heavyBucketNumList[i], towerMemList[i], 3, false, 813);


        true_freqs[0].clear();
        true_freqs[1].clear();
        true_freqs[2].clear();

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
            davinci01->insert((const char *)(traces[traceindex][i].key), 1);
            ++true_freqs[2][*((uint32_t *)(traces[traceindex][i].key))];
        }
        printf("Insertion finished\n");
        printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());

        Union<bucket_num>(*davinci0, *davinci1, *flcsketch, 813);
        
        flcsketch->decode(1);
        davinci0->decode(1);
        davinci1->decode(1);
        davinci01->decode(1);

        set<uint32_t> union_hh;
        set<uint32_t> davinci_hh; 

        uint32_t cnt_num = davinci01->heavy_part->get_bucket_num() * MAX_VALID_COUNTER;
        davinci01->get_heavy_hitters(davinci_hh, cnt_num);
        flcsketch->get_heavy_hitters(union_hh, cnt_num);

        set<uint32_t> HH_true;
        vector<pair<uint32_t, uint32_t>> freq_vec;
        for(auto it = true_freqs[2].begin(); it != true_freqs[2].end(); ++it)
        {
            freq_vec.push_back(*it);
        }
        sort(freq_vec.begin(), freq_vec.end(), [&](auto const& p, auto const& q){
            if (p.second != q.second)
                return p.second > q.second;
            else
                return p.first < q.first;
        });
        for (int i = 0; i < cnt_num; ++i) {
            HH_true.insert(freq_vec[i].first);
        }
        set<uint32_t>::iterator itr;
        double HH_precision = 0;
        double HH_are = 0;
        double HH_aae = 0;
        int HH_PR = 0;
        int HH_PR_denom = 0;
        int HH_RR = 0;
        int HH_RR_denom = 0;
        for (itr = HH_true.begin(); itr != HH_true.end(); ++itr)
        {
            HH_PR_denom += 1;
            HH_PR += (davinci_hh.find(*itr) != davinci_hh.end());
        }
        for (itr = davinci_hh.begin(); itr != davinci_hh.end(); ++itr)
        {
            HH_RR_denom += 1;
            HH_RR += (HH_true.find(*itr) != HH_true.end());
        }
        for (itr = HH_true.begin(); itr != HH_true.end(); ++itr)
        {
            uint32_t key = *itr;
            HH_are += std::abs((double)((int)true_freqs[2][key] - (int)davinci01->query((const char *)&key))) / (double)true_freqs[2][key];
            HH_aae += std::abs((double)((int)true_freqs[2][key] - (int)davinci01->query((const char *)&key)));
        }
        HH_are /= HH_true.size();
        HH_aae /= HH_true.size();
        HH_precision = (2 * (double(HH_PR) / double(HH_PR_denom)) * (double(HH_RR) / double(HH_RR_denom))) / ((double(HH_PR) / double(HH_PR_denom)) + (double(HH_RR) / double(HH_RR_denom)));

        double union_HH_precision = 0;
        double union_HH_are = 0;
        double union_HH_aae = 0;
        int union_HH_PR = 0;
        int union_HH_PR_denom = 0;
        int union_HH_RR = 0;
        int union_HH_RR_denom = 0;
        for (itr = HH_true.begin(); itr != HH_true.end(); ++itr)
        {
            union_HH_PR_denom += 1;
            union_HH_PR += (union_hh.find(*itr) != union_hh.end());
        }
        for (itr = union_hh.begin(); itr != union_hh.end(); ++itr)
        {
            union_HH_RR_denom += 1;
            union_HH_RR += (HH_true.find(*itr) != HH_true.end());
        }
        for (itr = HH_true.begin(); itr != HH_true.end(); ++itr)
        {
            uint32_t key = *itr;
            union_HH_are += std::abs((double)((int)true_freqs[2][key] - (int)flcsketch->query((const char *)&key))) / (double)true_freqs[2][key];
            union_HH_aae += std::abs((double)((int)true_freqs[2][key] - (int)flcsketch->query((const char *)&key)));
        }
        union_HH_are /= HH_true.size();
        union_HH_aae /= HH_true.size();
        union_HH_precision = (2 * (double(union_HH_PR) / double(union_HH_PR_denom)) * (double(union_HH_RR) / double(union_HH_RR_denom))) / ((double(union_HH_PR) / double(union_HH_PR_denom)) + (double(union_HH_RR) / double(union_HH_RR_denom)));

        set<int32_t> hp1, hp2;
        davinci01->get_all_heavy_parts(hp1);
        flcsketch->get_all_heavy_parts(hp2);

        uint32_t missflow = 0;
        for (auto const& key : hp1) {
            if (hp2.find(key) == hp2.end()) missflow++;
        }
        double rate = (double) missflow / (double) hp1.size();

        double missRate = 1.0 - (double)HH_PR / (double) HH_PR_denom;
        double unionMissRate = 1.0 - (double) union_HH_PR / (double) union_HH_PR_denom;
        outFile3 << totalMem[i] << "," << fermatMemList[i] << "," << heavyBucketNumList[i] << "," << towerMemList[i] << "," << HH_precision << "," << union_HH_precision << "," << HH_are << "," << union_HH_are << "," << HH_aae << "," << union_HH_aae << "," << missRate << "," << unionMissRate << "," << cnt_num << "\n";
    }
    outFile3.close();
}