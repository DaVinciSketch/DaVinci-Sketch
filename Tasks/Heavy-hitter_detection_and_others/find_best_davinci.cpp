#include "./src/DaVinci/DaVinci.h"
#include "./src/common_func.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <thread>
#include <memory>
#include <cmath>
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


void run_experiment(int TOTAL_MEMORY, const std::string& output_filename) {
    std::ofstream out(output_filename, std::ios::app);
    out << "index,fermatEleMem,heavypartBucketNum,towerMem,ARE\n";

    unordered_map<uint32_t, uint32_t> true_freqs[2];
    int index_expr = 0;
    int total_exprs = (TOTAL_MEMORY / 1800) * (TOTAL_MEMORY / 6400);
    std::cout << "Total exprs: " << total_exprs << std::endl;
    map<int, double> results;
    int best_index = 0;
    double alpha = (double)TOTAL_MEMORY/(500*1024);
    int fermatGap = 9000 * alpha;
    int heavyGap = 200 * alpha;

    for (int fermatEleMem = fermatGap; fermatEleMem <= fermatGap; fermatEleMem += fermatGap) {
        for (int heavypartBucketNum = heavyGap; heavypartBucketNum * 64 <= TOTAL_MEMORY - fermatEleMem; heavypartBucketNum += heavyGap) {
            int towerMem = TOTAL_MEMORY - fermatEleMem - heavypartBucketNum * 64;
            if(towerMem < 1){
                continue;
            }
            std::cout << "Currenctly running parameters: " << fermatEleMem << "/" << TOTAL_MEMORY << ", " << heavypartBucketNum << "/" << TOTAL_MEMORY - fermatEleMem <<", " << towerMem  << ", alpha = " << alpha << std::endl;
            // return;
            double totalARE = 0;
            for (int t = 0; t < TIMES; t++) {
                uint32_t init_seed = 813 + t;  // 为每次实验更改种子
                std::unique_ptr<DaVinci<bucket_num>> davinci = std::make_unique<DaVinci<bucket_num>>(TOTAL_MEMORY, fermatEleMem, heavypartBucketNum, towerMem, 3, false, init_seed);
                true_freqs[0].clear();
                unordered_map<uint32_t, uint32_t>& true_freq = true_freqs[0];
                int num_pkt = (int)traces[0].size();
                for (int i = 0; i < num_pkt; ++i) {
                    ++true_freq[*((uint32_t*)(traces[0][i].key))];
                    davinci->insert((const char*)(traces[0][i].key), 1);
                }

                davinci->decode();
                double temare = 0.0;
                for(auto it = true_freq.begin(); it != true_freq.end(); ++it) {
                    int estimated = davinci->query((const char *)&(it->first), 1);
                    int actual = it->second;
                    double dist = std::abs(actual - estimated);
                    temare += dist / actual;
                }
                temare /= true_freq.size();
                totalARE += temare;
                std::cout << "ARE: " << temare << std::endl;
            }
            double averageARE = totalARE / TIMES;
            out << index_expr << "," << fermatEleMem << "," << heavypartBucketNum << "," << towerMem << "," << averageARE << "\n";
            results[index_expr] = averageARE;
            if (averageARE < results[best_index]) {
                best_index = index_expr;
            }
            index_expr++;
        }
    }

    out << "Best index: " << best_index << "\n";
    out << "Best ARE: " << results[best_index] << "\n";
    out.close();
}


int main() {
    uint32_t totnum_packet = myReadTraces();
    std::vector<int> total_memories = {550 * 1024};//,100 * 1024, 150 * 1024, 200 * 1024, 250 * 1024, 300 * 1024, 350 * 1024, 400 * 1024, 450 * 1024, 500 * 1024}; // You can add more memory configurations here.
    std::vector<std::thread> threads;

    for (int mem : total_memories) {
        std::string filename = "outputs/find_best_results/find_best_results_" + std::to_string(mem) + ".csv";
        threads.push_back(std::thread(run_experiment, mem, filename));
    }

    for (auto& th : threads) {
        th.join();
    }

    return 0;
}


// int main() {
//     uint32_t totnum_packet = myReadTraces();
//     const int TOTAL_MEMORY = 500*1024;  // 总内存，单位为某种你选择的量
//     // const int TIMES = 10;  // 每个配置重复的次数

//     std::ofstream out("outputs/find_best_results.csv", std::ios::app);
//     out << "index,fermatEleMem,heavypartBucketNum,towerMem,ARE\n";
//     unordered_map<uint32_t, uint32_t> true_freqs[2];
//     int index_expr = 0;
//     int total_exprs = (TOTAL_MEMORY/1800) * ((TOTAL_MEMORY)/6400);
//     std::cout << "Total exprs: " << total_exprs << std::endl;
//     map<int, double> results;
//     int best_index = 0;

//     for (int fermatEleMem = 163800; fermatEleMem <= TOTAL_MEMORY; fermatEleMem += 9000) {
//         for (int heavypartBucketNum = 400; heavypartBucketNum * 64 <= TOTAL_MEMORY - fermatEleMem; heavypartBucketNum += 400) {
//             // for (int towerMem = 0; towerMem <= TOTAL_MEMORY - fermatEleMem - heavypartBucketNum * 64; towerMem += 100) {
                
//                 int towerMem = TOTAL_MEMORY - fermatEleMem - heavypartBucketNum * 64;
//                 if(towerMem < 1){
//                     continue;
//                 }
//                 double totalARE = 0;
//                 for (int t = 0; t < TIMES; t++) {
//                     uint32_t init_seed = 813 + t;  // 为每次实验更改种子
//                     // DaVinci<bucket_num> *davinci = new DaVinci<bucket_num>(TOTAL_MEMORY, fermatEleMem, heavypartBucketNum, towerMem, 3, false, init_seed);
//                     std::unique_ptr<DaVinci<bucket_num>> davinci = std::make_unique<DaVinci<bucket_num>>(TOTAL_MEMORY, fermatEleMem, heavypartBucketNum, towerMem, 3, false, init_seed);
//                     true_freqs[0].clear();
//                     unordered_map<uint32_t, uint32_t> &true_freq = true_freqs[0];
//                     int num_pkt = (int)traces[0].size();
//                     for (int i = 0; i < num_pkt; ++i)
//                     {
//                         ++true_freq[*((uint32_t *)(traces[0][i].key))];
//                         davinci->insert((const char *)(traces[0][i].key), 1);
//                     }

//                     davinci->decode();
//                     double temare = 0.0;
//                     for(auto it = true_freq.begin(); it != true_freq.end(); ++it) {
//                         int estimated = davinci->query((const char *)&(it->first), 1);
//                         int actual = it->second;
//                         double dist = abs(actual - estimated);
//                         temare += dist * 1.0 / actual;
//                     }
//                     temare /= true_freq.size();
//                     totalARE += temare;
//                     cout << "ARE: " << temare << endl;
//                     // delete davinci;
//                 }
//                 double averageARE = totalARE / TIMES;
//                 out << index_expr << "," << fermatEleMem << "," << heavypartBucketNum << "," << towerMem << "," << averageARE << "\n";
//                 results[index_expr] = averageARE;
//                 if (averageARE < results[best_index]) {
//                     best_index = index_expr;
//                 }
//                 index_expr++;

//             // }
//         }
//     }

//     //find best are index
//     out << "Best index: " << best_index << "\n";
//     out << "Best ARE" << results[best_index] << "\n";


//     out.close();
//     return 0;
// }