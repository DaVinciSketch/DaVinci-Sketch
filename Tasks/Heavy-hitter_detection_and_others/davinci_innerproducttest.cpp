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
    uint32_t totnum_packet = myReadTraces();

    int array_num = 3;
    int entry_num = (TOT_MEM_IN_BYTES - HEAVY_MEM)*10;
    int _fermatcount = 2; //Use Count version with id+ and cnt +-
    bool _fing = false;
    int testindex = 0;
    long double minare = 1000000;
    int minindex = -1;

    bool singletest = 0;
    int singletest_fermat_mem = 24000;//160000;
    int singletest_heavy_bucket_num = 3100;
    int singletest_tower_mem = 512000 - singletest_fermat_mem - singletest_heavy_bucket_num*64;


    // Fermat_tower *sketches[2];
    // Fermat_tower *fermat_tower = NULL;
    double aveare = 0.0, aveaae = 0.0, ave_HH = 0.0, ave_HC = 0.0, ave_card_RE = 0.0;
    double ave_HH_are = 0.0;
    double ave_WMRD = 0.0, ave_entr_RE = 0.0;
    unordered_map<uint32_t, uint32_t> true_freqs[2];
    // int totalMem[] = {25*1024, 50*1024, 75*1024, 100*1024, 125*1024, 200*1024, 300*1024, 400*1024, 500*1024, 600*1024};
    int totalMem[] = {200*1024, 300*1024, 400*1024, 500*1024, 600*1024};
    std::ofstream outFile("./outputs/finalresults/davinci_innerp_final_result.csv", std::ios::app);
    outFile << "index,totalMem,fermatMem,heavyBucketNum,towerMem,averageARE\n";

    uint32_t init_seed = 37;
    for(int fermat_mem_base = 10000; fermat_mem_base <= 500*1024; fermat_mem_base+=5000){
        for(int heavy_bucket_num_base = 1000; heavy_bucket_num_base*64 <= 500*1024 - fermat_mem_base; heavy_bucket_num_base += 100){
            for(int index = 0; index < 5; index++){
                int cur_total_mem = totalMem[index];    

                double cur_alpha = (double)cur_total_mem / (500*1024);
                // int cur_fermat_mem = 9000 * cur_alpha;
                // int cur_heavy_bucket_num = 2000 * cur_alpha;
                // int cur_tower_mem = 375000 * cur_alpha;
                // int cur_fermat_mem = 90000 * cur_alpha;
                // int cur_heavy_bucket_num = 1920 * cur_alpha;
                // int cur_tower_mem = 299120 * cur_alpha;
                int cur_fermat_mem = fermat_mem_base * cur_alpha;
                int cur_heavy_bucket_num = heavy_bucket_num_base * cur_alpha;
                int cur_tower_mem = cur_total_mem - cur_fermat_mem - cur_heavy_bucket_num*64;
                if(singletest){
                    cur_fermat_mem = singletest_fermat_mem;
                    cur_heavy_bucket_num = singletest_heavy_bucket_num;
                    cur_tower_mem = singletest_tower_mem;
                }
                if(cur_tower_mem < 1){
                    continue;
                }
                cout << "cur_total_mem: " << cur_total_mem << ", cur_fermat_mem: " << cur_fermat_mem << ", cur_heavy_bucket_num: " << cur_heavy_bucket_num << ", cur_tower_mem: " << cur_tower_mem << endl;
                unique_ptr<DaVinci<bucket_num>> davinci0 = make_unique<DaVinci<bucket_num>>(cur_total_mem, cur_fermat_mem, cur_heavy_bucket_num, cur_tower_mem, 3, false, 813);
                unique_ptr<DaVinci<bucket_num>> davinci1 = make_unique<DaVinci<bucket_num>>(cur_total_mem, cur_fermat_mem, cur_heavy_bucket_num, cur_tower_mem, 3, false, 813);
                
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
                        davinci0->insert((const char *)(traces[traceindex][i].key), 1);
                    }
                    else{
                        ++true_freqs[1][*((uint32_t *)(traces[traceindex][i].key))];
                        num2++;
                        davinci1->insert((const char *)(traces[traceindex][i].key), 1);
                    }
                }
                // for (int i = 0; i < bucket_num; i++)
                // {
                //     for(int j = 0; j < MAX_VALID_COUNTER; ++j){
                //         if(davinci0->heavy_part->buckets[i].key[j] == 50331651)
                //         cout << davinci0->heavy_part->buckets[i].key[j] << ": " << davinci0->heavy_part->buckets[i].val[j] << endl;
                //     }
                // }
                //check 50331651
                if(true_freqs[0].count(50331651)==0){
                    printf("50331651 not found in 0\n");
                }
                else{
                    printf("50331651 found in 0\n");
                }
                if(true_freqs[1].count(50331651)==0){
                    printf("50331651 not found in 1\n");
                }
                else{
                    printf("50331651 found in 1\n");
                }
                printf("Insertion finished\n");
                printf("Sizes: %d, %d\n", true_freqs[0].size(), true_freqs[1].size());

                // FLCSketch<bucket_num> sketch_result = Union<bucket_num>(*sketches[0], *sketches[1], init_seed);
                long double calculated_innerproduct = InnerProduct<bucket_num>(*davinci0, *davinci1, true);

                //Real inner product
                long double real_innerproduct = 0;
                for (auto it = true_freqs[0].begin(); it != true_freqs[0].end(); ++it)
                {
                    real_innerproduct += (long double)it->second * (long double)true_freqs[1][it->first];
                }
                double minare_tem = (real_innerproduct - calculated_innerproduct) / real_innerproduct;
                if(minare_tem < minare){
                    minare = minare_tem;
                    minindex = testindex;
                }
                outFile << (testindex++) << "," << cur_total_mem << "," << cur_fermat_mem << "," << cur_heavy_bucket_num << "," << cur_tower_mem << "," << (real_innerproduct - calculated_innerproduct) / real_innerproduct << std::endl;
                printf("Real inner product: %Lf\n", real_innerproduct);
                printf("Calculated inner product: %Lf\n", calculated_innerproduct);
                printf("Error rate: %Lf\n", (real_innerproduct - calculated_innerproduct) / real_innerproduct);

                if(singletest){
                    return 0;
                }
            }
        }


    }

    // sketches[0]->write2file("innerp_sketch1.txt");
    // sketches[1]->write2file("innerp_sketch2.txt");
    
    /*-*-*-* End of packet insertion *-*-*-*/

}