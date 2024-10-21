#include "./src/DaVinci/DaVinci.h"
#include "./src/common_func.h"
#include <iostream>
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (200 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;


int totalMem[] = {200*1024, 300*1024, 400*1024, 500*1024, 600*1024};

int main()
{
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB, FERMAT_BUCKET %d\n", TOT_MEM, ELE_BUCKET);
    
    uint32_t totnum_packet = ReadTwoWindows();

    int array_num = 3;
    int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM);
    int _fermatcount = 2; //Use Count version with id+ and cnt +-
    bool _fing = false;

    double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0;
    double ave_HH_are = 0.0;
    double ave_WMRD = 0.0, ave_entr_RE = 0.0;
    unordered_map<uint32_t, uint32_t> true_freqs[2];
    bool singletest = 0;
    int singletest_fermat_mem = 24000;//160000;
    int singletest_heavy_bucket_num = 3100;
    int singletest_tower_mem = 512000 - singletest_fermat_mem - singletest_heavy_bucket_num*64;
    int index = 0;
    int maxpre = 0;
    int maxindex = -1;
    
    std::ofstream outFile("./outputs/finalresults/davinci_hc_final_result.csv", std::ios::app);
    outFile << "index, totalMem, fermatMem, heavyBucketNum, towerMem, WMRD\n";
    for (int fermat_mem_base = 10000; fermat_mem_base <= 500 * 1024; fermat_mem_base += 10000)
    {
        for (int heavy_bucket_num_base = 1000; heavy_bucket_num_base * 64 <= 500 * 1024 - fermat_mem_base; heavy_bucket_num_base += 100)
        {
            int sumpre = 0;
            for (int index_ = 0; index_ < 5; index_++)
            {
                int cur_total_mem = totalMem[index_]; 
                double cur_alpha = (double)cur_total_mem / (500*1024);
                int cur_fermat_mem = fermat_mem_base * cur_alpha;
                int cur_heavy_bucket_num = heavy_bucket_num_base * cur_alpha;
                int cur_tower_mem = cur_total_mem - cur_fermat_mem - cur_heavy_bucket_num*64;
                if(singletest){
                    cur_total_mem = 204800;
                    cur_fermat_mem = 8000;
                    cur_heavy_bucket_num = 2040;
                    cur_tower_mem = 66240;
                }
                
                unique_ptr<DaVinci<bucket_num>> davinci0 = make_unique<DaVinci<bucket_num>>(cur_total_mem, cur_fermat_mem, cur_heavy_bucket_num, cur_tower_mem, 3, false, 37);
                unique_ptr<DaVinci<bucket_num>> davinci1 = make_unique<DaVinci<bucket_num>>(cur_total_mem, cur_fermat_mem, cur_heavy_bucket_num, cur_tower_mem, 3, false, 37);
                true_freqs[0].clear();
                true_freqs[1].clear();
                
                unordered_map<uint32_t, uint32_t> &true_freq0 = true_freqs[0];
                unordered_map<uint32_t, uint32_t> &true_freq1 = true_freqs[1];

                int windowno = 0;
                vector<int> true_dist(1);
                
                
                for (int i = 0; i < traces[windowno].size(); ++i)
                {
                    ++true_freq0[*((uint32_t *)(traces[windowno][i].key))];
                    davinci0->insert((const char *)(traces[windowno][i].key), 1);
                }
                for (int i = 0; i < traces[windowno + 1].size(); ++i)
                {
                    ++true_freq1[*((uint32_t *)(traces[windowno + 1][i].key))];
                    davinci1->insert((const char *)(traces[windowno + 1][i].key), 1);
                }

                // 假设array_num是3，entry_num是数组的长度
                int array_num = 3;
                int entry_num = davinci0->fermatEle->get_entry_num(); // 你需要提供一个获取entry_num的方法
                set<uint32_t> HH_true;
                // 遍历三个数组

                // davinci->decode(true);
                davinci0->decode(1);
                davinci1->decode(1);
                davinci0->get_all_results();
                davinci1->get_all_results();
                double temare = 0.0;
                // 创建一个输出文件流对象
                set<uint32_t> fermattower_hc;
                set<uint32_t> est_hc;
                set<uint32_t> real_hc;
                double HC_precision = 0;
                int HC_PR = 0;
                int HC_PR_denom = 0;
                int HC_RR = 0;
                int HC_RR_denom = 0;
                set<uint32_t>::iterator itr;
                cout << "davinci0->allResult.size() = " << davinci0->allResult.size() << endl;
                cout << "davinci1->allResult.size() = " << davinci1->allResult.size() << endl;
                cout << "true_freqs[0].size() = " << true_freqs[0].size() << endl;
                cout << "true_freqs[1].size() = " << true_freqs[1].size() << endl;
                cout << "true_freq0.size() = " << true_freq0.size() << endl;
                cout << "true_freq1.size() = " << true_freq1.size() << endl;
                for (auto f : davinci1->allResult)
                {
                    if ((int)davinci1->query((const char *)&f.first) -
                            (int)davinci0->query((const char *)&f.first) >
                        HC_THRESHOLD)
                        est_hc.insert(f.first);
                }
                for (auto f : davinci0->allResult)
                {
                    if ((int)davinci0->query((const char *)&f.first) -
                            (int)davinci1->query((const char *)&f.first) >
                        HC_THRESHOLD)
                        est_hc.insert(f.first);
                }
                for (auto f : true_freqs[1])
                {
                    if (!true_freqs[0].count(f.first))
                    {
                        if (f.second > HC_THRESHOLD)
                            real_hc.insert(f.first);
                    }
                    else if (int(f.second - true_freqs[0][f.first]) > HC_THRESHOLD)
                        real_hc.insert(f.first);
                }
                for (auto f : true_freqs[0])
                {
                    if (!true_freqs[1].count(f.first))
                    {
                        if (f.second > HC_THRESHOLD)
                            real_hc.insert(f.first);
                    }
                    else if (int(f.second - true_freqs[1][f.first]) > HC_THRESHOLD)
                        real_hc.insert(f.first);
                }
                for (itr = real_hc.begin(); itr != real_hc.end(); ++itr)
                {
                    HC_PR_denom += 1;
                    HC_PR += (est_hc.find(*itr) != est_hc.end());
                }
                for (itr = est_hc.begin(); itr != est_hc.end(); ++itr)
                {
                    HC_RR_denom += 1;
                    HC_RR += (real_hc.find(*itr) != real_hc.end());
                }
                HC_precision = (2 * (double(HC_PR) / double(HC_PR_denom)) * (double(HC_RR) / double(HC_RR_denom))) / ((double(HC_PR) / double(HC_PR_denom)) + (double(HC_RR) / double(HC_RR_denom)));
                printf("HC_precision : %3.5f\n", HC_precision);
                printf("HC_PR : %d, HC_PR_denom : %d, HC_RR : %d, HC_RR_denom : %d\n", HC_PR, HC_PR_denom, HC_RR, HC_RR_denom);
                sumpre += HC_precision;
                outFile << (index++) << ", " << cur_total_mem << ", " << cur_fermat_mem << ", " << cur_heavy_bucket_num << ", " << cur_tower_mem << ", " << HC_precision << "\n";
            }
            if(sumpre > maxpre){
                maxpre = sumpre;
                maxindex = index;
            }
        }
    }
    outFile << "maxindex, " << maxindex << "maxtotalpre, " << maxpre << "\n";

}