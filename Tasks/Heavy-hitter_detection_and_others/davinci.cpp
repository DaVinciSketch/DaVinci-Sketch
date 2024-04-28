#include "./src/DaVinci/DaVinci.h"
#include "./src/common_func.h"
#include <iostream>
#define HEAVY_MEM (150 * 1024)
#define TOT_MEM_IN_BYTES (200 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
static constexpr int bucket_num = BUCKET_NUM;
// template class HeavyPart<2400>;
// struct CompareByAbs{
//     bool operator()(const int &a, const int &b) const{
//         return a < b;
//     }
// };

int main()
{
    printf("Start accuracy measurement of tower_fermat: TOTAL_MEMORY %dKB, FERMAT_BUCKET %d\n", TOT_MEM, ELE_BUCKET);
    // uint32_t totnum_packet = ReadTraces();
    uint32_t totnum_packet = myReadTraces();

    int array_num = 3;
    int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM);
    int _fermatcount = 2; //Use Count version with id+ and cnt +-
    bool _fing = false;

    DaVinci<bucket_num> *davinci = NULL;

    // Fermat_tower *sketches[2];
    // Fermat_tower *fermat_tower = NULL;
    double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0;
    double ave_HH_are = 0.0;
    double ave_WMRD = 0.0, ave_entr_RE = 0.0;
    unordered_map<uint32_t, uint32_t> true_freqs[2];
    double alphas[] = {0.2, 0.4, 0.6, 0.8, 1.0, 1.2};
    vector<double> hhresult(6, 0);
    vector<double> Cardresult(6, 0);
    vector<double> flowsizeresult(6, 0);
    for(auto alpha:alphas){
        double hh_pre = 0;
        int totaltime = 1;
        ave_card_RE = 0;
        for (int times = 0; times < totaltime; times++)//TIMES; times++)
        {
            std::cout << "times: " << times << std::endl;
            // davinci = new DaVinci<bucket_num>(alpha*500*1024,alpha*37800,alpha*3200,alpha*269400,3,false,813+times);
            int singletest_fermat_mem = alpha*24000;//160000;
            int singletest_heavy_bucket_num = alpha*3100;
            int singletest_tower_mem = alpha*512000 - singletest_fermat_mem - singletest_heavy_bucket_num*64;
            cout << "singletest_fermat_mem: " << singletest_fermat_mem << " singletest_heavy_bucket_num: " << singletest_heavy_bucket_num << " singletest_tower_mem: " << singletest_tower_mem << endl;
            davinci = new DaVinci<bucket_num>(alpha*500*1024, singletest_fermat_mem, singletest_heavy_bucket_num, singletest_tower_mem, 3, false,813+times);
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
            davinci->write2file("outputs/davinci.txt");
            davinci->tower->printCountersToCSV();
            davinci->tower->printHashCrushAnal2File();


            // std::map<int, int, CompareByAbs> counter_mapping;

            // 假设array_num是3，entry_num是数组的长度
            int array_num = 3;
            int entry_num = davinci->fermatEle->get_entry_num(); // 你需要提供一个获取entry_num的方法
            set<uint32_t> HH_true;
            // 遍历三个数组

            
            // for (int n_array = 0; n_array < array_num; ++n_array) {
            //     for (int n = 0; n < entry_num; ++n) {
            //         // 获取counter的值
            //         int counter_value = davinci->fermatEle->get_counter(n_array, n);
            //         // 更新字典
            //         // if(counter_value == 1 || counter_value == -1) {
            //         //     cout << "Counter: " << counter_value << endl;
            //         // }
            //         counter_mapping[counter_value]++;
            //     }
            // }

            // 打印字典
            // for (const auto& pair : counter_mapping) {
            //     std::cout << "Counter: " << pair.first << ", Count: " << pair.second << std::endl;
            // }

            // davinci->decode(true);
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
                if(actual > HH_THRESHOLD){
                    HH_true.insert(it->first);
                }
            }
            flowsizeresult.push_back(temare/true_freq.size());

            out.close();

            std::ofstream outFile2("outputs/davinci_result_compare.csv");
            outFile2 << "flowid,real_result,flcsketch_result\n";
            for (const auto& elem : true_freq) {
                int cnt = davinci->query((char*)&(elem.first), true);
                // if(elem.second != cnt)// && elem.second > 0)
                // if(uint32_t(elem.first) == 3948909411 || elem.second != cnt)// && elem.second > 0)
                if(davinci->decode_track[elem.first][2] != 0)// && elem.second > 0)
                    outFile2 << uint32_t(elem.first) << "," << elem.second << "," 
                    << cnt << "(" << davinci->decode_track[elem.first][0] << "-" << davinci->decode_track[elem.first][1] << "-" << davinci->decode_track[elem.first][2] << "-" << davinci->decode_track[elem.first][3] <<")" << "\n";
            }

            outFile2.close();
            davinci->write2file("sketch_result_afterdecoding.txt");

            temare /= true_freq.size();
            aveare += temare;

            std::cout << times << ": ARE: " << temare << std::endl;

            // set<uint32_t>::iterator itr;
            // set<uint32_t> davinci_hh;
            // davinci->get_heavy_hitters(davinci_hh);
            
            // double HH_precision = 0;
            // double HH_are = 0;
            // int HH_PR = 0;
            // int HH_PR_denom = 0;
            // int HH_RR = 0;
            // int HH_RR_denom = 0;
            // for (itr = HH_true.begin(); itr != HH_true.end(); ++itr)
            // {
            //     HH_PR_denom += 1;
            //     HH_PR += (davinci_hh.find(*itr) != davinci_hh.end());
            // }
            // for (itr = davinci_hh.begin(); itr != davinci_hh.end(); ++itr)
            // {
            //     HH_RR_denom += 1;
            //     HH_RR += (HH_true.find(*itr) != HH_true.end());
            // }
            // for (itr = HH_true.begin(); itr != HH_true.end(); ++itr)
            // {
            //     uint32_t key = *itr;
            //     HH_are += std::abs((double)((int)true_freq[key] - (int)davinci->query((const char *)&key))) / (double)true_freq[key];
            // }
            // HH_are /= HH_true.size();
            // HH_precision = (2 * (double(HH_PR) / double(HH_PR_denom)) * (double(HH_RR) / double(HH_RR_denom))) / ((double(HH_PR) / double(HH_PR_denom)) + (double(HH_RR) / double(HH_RR_denom)));

            // printf("HH_precision : %3.5f\n", HH_precision);
            // printf("HH_PR : %d, HH_PR_denom : %d, HH_RR : %d, HH_RR_denom : %d\n", HH_PR, HH_PR_denom, HH_RR, HH_RR_denom);
            // printf("HH_are : %3.5f\n", HH_are);
            // ave_HH += HH_precision;
            // ave_HH_are += HH_are;
            // hh_pre += HH_precision;


            // int card = davinci->get_cardinality();
            // double card_err = abs(card - int(true_freq.size())) / double(true_freq.size());
            // printf("RE of cardinality : %2.6f (est=%8.0d, true=%8.0d)\n", card_err, card, (int)true_freq.size());
            // ave_card_RE += card_err;
        }
        hhresult.push_back(hh_pre/totaltime);
        Cardresult.push_back(ave_card_RE/totaltime);
    }
    std::cout << "HH Precision: " << std::endl;
    for(auto i:hhresult){
        std::cout << i << " ";
    }
    std::cout << std::endl;
    std::cout << "Cardinality RE: " << std::endl;
    for(auto i:Cardresult){
        std::cout << i << " ";
    }
    std::cout << std::endl;
    std::cout << "Flow size ARE: " << std::endl;
    for(auto i:flowsizeresult){
        std::cout << i << " ";
    }
    aveare /= TIMES;
    // aveaae /= TIMES;
    // ave_HH /= TIMES;
    // ave_HC /= TIMES;
    // ave_card_RE /= TIMES;
    // ave_WMRD /= TIMES;
    // ave_entr_RE /= TIMES;
    // ave_HH_are /= TIMES;
    // printf("average ARE : %.8lf, average AAE : %.8lf\n", aveare, aveaae);
    printf("average ARE : %.8lf\n", aveare);
    // printf("average HH F1 score : %.8lf\n", ave_HH);
    // printf("average HH ARE : %.8lf\n", ave_HH_are);
    // printf("average HC F1 score : %.8lf\n", ave_HC);
    // printf("average cardinality RE : %.8lf\n", ave_card_RE);
    // printf("average distribution WMRD : %.8lf\n", ave_WMRD);
    // printf("average entropy RE : %.8lf\n", ave_entr_RE);
    // printf("END\n");

    //HH detection
    

}