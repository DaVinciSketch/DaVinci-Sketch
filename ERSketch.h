#pragma once
// #include "tower.h"
#include <map>
#include "./common/EMFSD1.h"

#define TOT_MEMORY TOT_MEM * 1024 
#define ELE_BUCKET 2500
#define ELE_THRESHOLD 250
#define USE_FING 0
#define INIT ((uint32_t)random() % 800)
#define FERMAT_EM_ITER 15
// #include "./common_func.h"

#include "Fermat/fermat.h"
#include "ElasticSketch/HeavyPart.h"

#define HEAVY_MEM_ (150 * 1024)
#define BUCKET_NUM_ (HEAVY_MEM_ / 64)

using namespace std;
template<int _bucket_num>
class FLCSketch
{
public:
    int tot_memory;
    int tot_packets;
    int fermatEleMem;
    int towerfilterMem;
    bool ifFermatCount;
    // TowerSketch *towerfilter;
    unordered_map<uint32_t, int> Eleresult;
    EMFSD1 *em = NULL;

    // int bucket_num;
    int heavy_mem = _bucket_num * COUNTER_PER_BUCKET * 8;

    static constexpr int bucket_num = _bucket_num;
    HeavyPart<bucket_num> *heavy_part;
    Fermat *fermatEle;

    //for test track
    unordered_map<uint32_t, vector<pair<int, int>>> insert_tracking;

public:

    FLCSketch(int _heavepartBucketNum, int _fermatEleMem, bool _fermatcount = 1, 
                 bool usefing = USE_FING, uint32_t _init = INIT) : fermatEleMem(_fermatEleMem), ifFermatCount(_fermatcount)
    {
        tot_packets = 0;
        // fermatEle = new Fermat(fermatEleMem, usefing, _init);
        if(_fermatcount){
            fermatEle = new Fermat_Count(fermatEleMem, usefing, _init);
        }
        else{
            fermatEle = new Fermat_Sketch(fermatEleMem, usefing, _init);
        }
        heavy_part = new HeavyPart<bucket_num>();
        

    }
    FLCSketch(int _heavepartBucketNum, int array_num, int entry_num, bool _fermatcount = 1, 
                 bool usefing = USE_FING, uint32_t _init = INIT)
    {
        tot_packets = 0;
        // fermatEle = new Fermat(array_num, entry_num, usefing, _init);
        if(_fermatcount){
            fermatEle = new Fermat_Count(array_num, entry_num, usefing, _init);
        }
        else{
            fermatEle = new Fermat_Sketch(array_num, entry_num, usefing, _init);
        }
        heavy_part = new HeavyPart<bucket_num>();

    }
    void insert(const char *key, int f = 1)
    {
        //heavy part
        uint8_t swap_key[KEY_LENGTH_4];
        uint32_t swap_val = 0;
        
        //tracking 
        int result = heavy_part->insert((uint8_t *)key, swap_key, swap_val, f);
        uint32_t keysfing = *(uint32_t *)key;
        if(result == 1) {
            keysfing = *(uint32_t *)swap_key;
        }
        else if(result == 2){
            swap_val = 1;
        }

        int sign = 1;
        // if(ifFermatCount){
        //     sign = fermatEle->get_sign(key, 2);
        // }
        pair<int, int> valuePair = std::make_pair(result, GetCounterVal(swap_val));
        insert_tracking[keysfing].push_back(valuePair);
        if(keysfing == 24690453){
            cout << "#################" << keysfing << " " << result << " " <<GetCounterVal(swap_val) << endl;
        }

        switch(result)
        {
            case 0: break;
            case 1: fermatEle->Insert(*(uint32_t*) swap_key, GetCounterVal(swap_val)); break;
            case 2: fermatEle->Insert(*(uint32_t*) key, 1); break;
            default:
                printf("error return value !\n");
                exit(1);
        }

        // printf("After Insert in ERSketch switch\n");

        tot_packets++;
    }
    // void get_distribution(vector<double> &ed)
    // {
    //     em = new EMFSD1;
    //     uint32_t *countercpy;
    //     countercpy = new uint32_t[towerfilter->line[1].width];
    //     for (int i = 0; i < this->towerfilter->line[1].width; i++)
    //     {
    //         countercpy[i] = towerfilter->line[1].index(i);
    //         //cout << countercpy[i]<<endl;
    //     }
    //     em->set_counters(this->towerfilter->line[1].width, countercpy, 65535);
    //     for (int i = 0; i < FERMAT_EM_ITER; i++)
    //     {
    //         printf("%d_epoch\n", i);
    //         em->next_epoch();
    //     }
        
        
    //     // printf("%ld\n", em->ns.size());
    //     // fflush(stdout);
    //     ed.resize(em->ns.size());
    //     for (int i = 1; i < em->ns.size(); i++)
    //     {
    //         ed[i] = em->ns[i];
    //     }
    //     ed.resize(2000000);
    //     for(auto i : Eleresult){
    //         if(i.second + towerfilter->threshold > 65535){
    //             ed[i.second+towerfilter->threshold]++;
    //             if(ed[65535])
    //                 ed[65535]-=1;
    //         }
    //     }
    //     int sum = 0;
    //     for (int i = 1; i < 2000000; i++)
    //     {
    //         sum += i * ed[i];
    //     }
    // }
    // int get_cardinality()
    // {
    //     int used = 0, total = 0;
    //     return towerfilter->get_cardinality();
    // }

    int decode()
    {
        // printf("Decoding......\n");
        if (fermatEle->Decode(Eleresult))
            printf("Decode Successfully!\n");
        else
            printf("Decode Fail!\n");
        printf("Eleresult: %lu\n", Eleresult.size());
        return Eleresult.size();
            // 遍历并打印 Eleresult 中的每个元素
        // printf("Begin printing...\n");
        // for (const auto &item : Eleresult)
        // {
        //     // 将 key 转换为 uint32_t 并打印
        //     uint32_t key = item.first;
        //     printf("Key: %.8x, Value: %d\n", key, item.second);
        // }
    }
    uint32_t query(const char *key, bool add_undecoded = 1, bool ifprint = 0)
    {
        uint32_t hp_cnt = heavy_part->query((uint8_t *)key);
        // uint32_t heavy_result = heavy_part.query(key);
        if(hp_cnt == 0 || HIGHEST_BIT_IS_1(hp_cnt))
        {
            if (Eleresult.count(*(uint32_t *)key))
            {
                // return hp_cnt + Eleresult[*(uint32_t *)key];
            // int light_result = light_part.query(key);
                // printf("Don't need CM! %d\n", (int)GetCounterVal(hp_cnt) + Eleresult[*(uint32_t *)key]);
                return (int)GetCounterVal(hp_cnt) + Eleresult[*(uint32_t *)key];

            }
            else if(add_undecoded){
                int cm_query = fermatEle->undecoded_query(key);
                // printf("Count Min Result: %d\n", cm_query);
                // return (int)GetCounterVal(hp_cnt)
                return (int)GetCounterVal(hp_cnt) + cm_query;
                // printf("Add Count Min Result: %d\n", cm_query);
                // fermatEle->counter[][];
            }
        }
        return (int)GetCounterVal(hp_cnt);

        // if(hp_cnt < 0){
        //     printf("Negative in hp_cnt!\n");
        //     printf("Key: ");
        //     for (int i = 0; i < 4; ++i) {
        //         printf("%02x ", (unsigned char)key[i]);
        //     }            
        //     printf("\n");
        // }
        
        // return towerfilter->query(key);
        // return hp_cnt;
    }
    // double get_entropy(vector<double> &distribution)
    // {
    //     double entropy = 0.0;
    //     double tot = 0.0;
    //     double entr = 0.0;
    //     for (int i = 1; i < distribution.size(); i++)
    //     {
    //         if(distribution[i] < 1.0)
    //             continue;
    //         tot += i * (int)distribution[i];
    //         entr += i * distribution[i] * log2(i);
    //     }
    //     //tot = tot_packets;
    //     entropy = -entr / tot + log2(tot);
    //     return entropy;
    // }

};