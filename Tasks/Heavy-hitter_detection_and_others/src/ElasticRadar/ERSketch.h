#pragma once
// #include "tower.h"
#include <map>
#include "common/EMFSD.h"

#define TOT_MEMORY TOT_MEM * 1024 
#define ELE_BUCKET 2500
#define ELE_THRESHOLD 250
#define USE_FING 0
#define INIT ((uint32_t)random() % 800)
#define FERMAT_EM_ITER 15
// #include "./common_func.h"

#include "fermat.h"
#include "HeavyPart.h"
#include "../common_func.h"

#define HEAVY_MEM_ (150 * 1024)
#define BUCKET_NUM_ (HEAVY_MEM_ / 64)

using namespace std;
template<int _bucket_num>
class FLCSketch
{
    int light_array_num;
    int light_entry_num;
public:
    int tot_memory;
    int tot_packets;
    int fermatEleMem;
    int towerfilterMem;
    bool ifFermatCount;
    // TowerSketch *towerfilter;
    EMFSD *em_fsd_algos = NULL;

    // int bucket_num;
    int heavy_mem = _bucket_num * COUNTER_PER_BUCKET * 8; //this is fake, only to satisfy "template" but not really used
    int heavy_bucket_num; // This is real

    static constexpr int bucket_num = _bucket_num;
    HeavyPart<bucket_num> *heavy_part;
    Fermat *fermatEle;

    //for test track
    unordered_map<int32_t, int> Eleresult;
    unordered_map<int32_t, vector<pair<int, int>>> insert_tracking;

    unordered_map<int32_t, vector<int>> decode_track;

    
public:

    FLCSketch(int _heavypartBucketNum, int _fermatEleMem, int _fermatcount = 2, 
                 bool usefing = USE_FING, uint32_t _init = INIT) : fermatEleMem(_fermatEleMem)
    {
        printf("parameters: _heavypartBucketNum = %d, _fermatEleMem = %d, _fermatcount = %d, usefing = %d, _init = %d\n", _heavypartBucketNum, _fermatEleMem, _fermatcount, usefing, _init);
        heavy_bucket_num = _heavypartBucketNum;
        // light_array_num = array_num;
        // light_entry_num = entry_num;
        tot_packets = 0;
        // fermatEle = new Fermat(fermatEleMem, usefing, _init);
        if(_fermatcount == 1){
            fermatEle = new Fermat_Count(fermatEleMem, usefing, _init);
        }
        else if(_fermatcount == 2){
            cout << "Running Fermat_Count_IDP_CNTPM" << endl;
            fermatEle = new Fermat_Count_IDP_CNTPM(fermatEleMem, usefing, _init);
        }
        else{
            fermatEle = new Fermat_Sketch(fermatEleMem, usefing, _init);
        }
        heavy_part = new HeavyPart<bucket_num>(_heavypartBucketNum);
        

    }
    FLCSketch(int _heavypartBucketNum, int array_num, int entry_num, int _fermatcount = 2, 
                 bool usefing = USE_FING, uint32_t _init = INIT)
    {
        printf("parameters: _heavypartBucketNum = %d, array_num = %d, entry_num = %d, _fermatcount = %d, usefing = %d, _init = %d\n", _heavypartBucketNum, array_num, entry_num, _fermatcount, usefing, _init);
        heavy_bucket_num = _heavypartBucketNum;
        light_array_num = array_num;
        light_entry_num = entry_num;

        ifFermatCount =  _fermatcount;
        tot_packets = 0;
        // fermatEle = new Fermat(array_num, entry_num, usefing, _init);
        if(_fermatcount == 1){
            fermatEle = new Fermat_Count(array_num, entry_num, usefing, _init);
        }
        else if(_fermatcount == 2){
            cout << "Running Fermat_Count_IDP_CNTPM, fermatEleMem = " << fermatEleMem << endl;
            fermatEle = new Fermat_Count_IDP_CNTPM(array_num, entry_num, usefing, _init);
        }
        else{
            fermatEle = new Fermat_Sketch(array_num, entry_num, usefing, _init);
        }
        // fermatEle = new Fermat_Count_IDP_CNTPM(fermatEleMem, usefing, _init);
        // heavy_part = new HeavyPart<bucket_num>(bucket_num);
        heavy_part = new HeavyPart<bucket_num>(_heavypartBucketNum);
        
        em_fsd_algos = new EMFSD[light_array_num];

    }
    void insert(const char *key, int f = 1)
    {
        //heavy part
        uint8_t swap_key[KEY_LENGTH_4];
        uint32_t swap_val = 0;
        
        //tracking 
        int result = heavy_part->insert((uint8_t *)key, swap_key, swap_val, f);
        uint32_t keysfing = *(uint32_t *)key;
        if(result == 1) { // Swap out entry
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

        if(!ifFermatCount)
            switch(result)
            {
                case 0: break; // Inserted into the heavy part and nothing to do with the light part
                case 1: fermatEle->Insert(*(uint32_t*) swap_key, GetCounterVal(swap_val)); break;//
                case 2: fermatEle->Insert(*(uint32_t*) key, 1); break;
                default:
                    printf("error return value !\n");
                    exit(1);
            }
        else{
            switch(result)
            {
                case 0: break;
                case 1: {
                    if(*(int32_t*)swap_key == 3444890234){
                        cout << "Got ya bitch! swap_key = " << *(int32_t*)swap_key << ", swap_val = " << GetCounterVal(swap_val) << endl;
                    }
                    fermatEle->Insert(*(int32_t*) swap_key, GetCounterVal(swap_val)); 
                    break;
                    }
                case 2: fermatEle->Insert(*(int32_t*) key, 1); break;
                default:
                    printf("error return value !\n");
                    exit(1);
            }
        }

        // printf("After Insert in ERSketch switch\n");

        tot_packets++;
    }

    void get_distribution(vector<double> &dist, int index = 0) {
        int32_t **counters;
        fermatEle->cpy_counters_to_pos(&counters);
        
        // EMFSD **em_fsd_algo = NULL;
        
        // for(int i=0; i<light_array_num; i++) {
        em_fsd_algos[index].set_counters(light_entry_num, (uint32_t*)counters[index]);
        int times = 10;
        while(times--) {
            em_fsd_algos[index].next_epoch();
        }
        // }


        dist = em_fsd_algos[index].ns;
        
        for(int i = 0; i < heavy_bucket_num; ++i)
            for(int j = 0; j < MAX_VALID_COUNTER; ++j) {
                uint8_t key[KEY_LENGTH_4];
                *(uint32_t*)key = heavy_part->buckets[i].key[j];
                int val = heavy_part->buckets[i].val[j];

                int ex_val = fermatEle->query_array((char*)key, index); //TODO:
                // cout << "ex_val = " << ex_val << endl;

                if(HIGHEST_BIT_IS_1(val) && ex_val != 0) {
                    val += ex_val;
                    dist[ex_val]--;
                }
                val = GetCounterVal(val);
                if(val) {
                    if(val + 1 > dist.size())
                        dist.resize(val + 1);
                    dist[val]++;
                }
            }
    }
    // int get_cardinality()
    // {
    //     int used = 0, total = 0;
    //     return towerfilter->get_cardinality();
    // }

    int decode()
    {
        // printf("Decoding...... Eleresult.size() = %d\n", Eleresult.size());
        // 创建 DataVariant 类型的实例
        cout << "Size of insertedflow is " << fermatEle->insertedflows.size() << endl;
        DataVariant variantEleresult = Eleresult;

        printf("Gonna decode %p\n", fermatEle);
        // 将 variantEleresult 传递给 Decode 函数
        if (fermatEle->Decode(variantEleresult)) 
            printf("Decode Successfully!\n");
        else
            printf("Decode Fail!\n");
        Eleresult = std::get<std::unordered_map<int, int>>(variantEleresult);
        bool flag_some_is_zero = 0;
        for(auto i : Eleresult){
            if(i.second == 0){
                printf("Key: %u, Value: %d\n", i.first, i.second);
                flag_some_is_zero = 1;
            }
        }
        if(flag_some_is_zero){
            printf("Some keys are zero!\n");
        }
        else{
            printf("All keys are non-zero!\n");
        }
        printf("Eleresult: %lu\n", Eleresult.size());
        printf("Lightpart-inserted num: %lu\n", fermatEle->insertedflows.size());
        printf("Decoded rate: %f\n", (double)Eleresult.size() / fermatEle->insertedflows.size());
        
        printf("-----------------------------------------------------------------------------\n");

        //update decode_track with heavypart and lightpart

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
        // add_undecoded = 0;
        uint32_t hp_cnt = heavy_part->query((uint8_t *)key);
        // uint32_t heavy_result = heavy_part.query(key);
        uint32_t id = *(uint32_t*) key;
        uint32_t checked_id = 3439783959;

        if(hp_cnt == 0 || HIGHEST_BIT_IS_1(hp_cnt))
        // if(1)
        {
            if(id == checked_id){
                cout << "Enter the if statement" << endl;
            }
            if (Eleresult.count(*(uint32_t *)key))
            {
                // return hp_cnt + Eleresult[*(uint32_t *)key];
            // int light_result = light_part.query(key);
                // printf("Don't need CM! %d\n", (int)GetCounterVal(hp_cnt) + Eleresult[*(uint32_t *)key]);
                decode_track[*(uint32_t *)key] = vector<int>{(int)GetCounterVal(hp_cnt), Eleresult[*(uint32_t *)key], 0};
                return (int)GetCounterVal(hp_cnt) + Eleresult[*(uint32_t *)key];

            }
            else if(add_undecoded){
                int cm_query = fermatEle->undecoded_query(key);
                // printf("Count Min Result: %d\n", cm_query);
                // return (int)GetCounterVal(hp_cnt)
                if(id == checked_id){
                    if(fermatEle->insertedflows.find(id) != fermatEle->insertedflows.end()){
                        cout << checked_id << " exists in insertedflows!" << endl;
                    }
                    else{
                        cout << checked_id << "3510838475 does not exist in insertedflows!" << endl;
                    }
                }
                decode_track[*(uint32_t *)key] = vector<int>{(int)GetCounterVal(hp_cnt), 0, cm_query};
                return (int)GetCounterVal(hp_cnt) + cm_query;
                // printf("Add Count Min Result: %d\n", cm_query);
                // fermatEle->counter[][];
            }
        }
        if(id == checked_id){
            cout << checked_id << " is not in Eleresult!" << endl;
        }
        // else
        // {
            // printf("Don't need CM! %d\n", (int)GetCounterVal(hp_cnt));
            decode_track[*(uint32_t *)key] = vector<int>{(int)GetCounterVal(hp_cnt), 0, 0};
            return (int)GetCounterVal(hp_cnt);
        // }

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

    uint32_t query_only_light_part(const char *key, bool add_undecoded = 1)
    {
        // cout << "Querying only light part!key = " << *(uint32_t *)key << endl;
        uint32_t hp_cnt = heavy_part->query((uint8_t *)key);
        uint32_t id = *(uint32_t*) key;
        uint32_t checked_id = 3439783959;

        if(hp_cnt == 0 || HIGHEST_BIT_IS_1(hp_cnt)){
            if(id == checked_id){
                cout << "Enter the if statement" << endl;
            }
            if (Eleresult.count(*(uint32_t *)key))
            {
                decode_track[*(uint32_t *)key] = vector<int>{(int)GetCounterVal(hp_cnt), Eleresult[*(uint32_t *)key], 0};
                return Eleresult[*(uint32_t *)key];
            }
            else if(add_undecoded){
                int cm_query = fermatEle->undecoded_query(key);
                if(id == checked_id){
                    if(fermatEle->insertedflows.find(id) != fermatEle->insertedflows.end()){
                        cout << checked_id << " exists in insertedflows!" << endl;
                    }
                    else{
                        cout << checked_id << "3510838475 does not exist in insertedflows!" << endl;
                    }
                }
                decode_track[*(uint32_t *)key] = vector<int>{(int)GetCounterVal(hp_cnt), 0, cm_query};
                return cm_query;
            }
        }
        if(id == checked_id){
            cout << checked_id << " is not in Eleresult!" << endl;
        }
        decode_track[*(uint32_t *)key] = vector<int>{(int)GetCounterVal(hp_cnt), 0, 0};
        return 0;
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
    void get_heavy_hitters(set<uint32_t> &hh){
        for (auto i : Eleresult)
        {
            if (query((const char *)&i.first) >= HH_THRESHOLD)
                hh.insert(i.first);
        }
    }

    int get_heavy_bucket_num(){
        return heavy_bucket_num;
    }
    int get_light_array_num(){
        return light_array_num;
    }
    int get_light_entry_num(){
        return light_entry_num;
    }

    bool print_sketch(){
        printf("Heavy Part:\n");
        for(int i = 0; i < heavy_bucket_num; ++i){
            printf("Bucket %d: ", i);
            for(int j = 0; j < MAX_VALID_COUNTER; ++j){
                printf("%d ", GetCounterVal(heavy_part->buckets[i].val[j]));
            }
            printf("\n");
        }
        printf("Light Part:\n");
        for(int i = 0; i < light_array_num; ++i){
            printf("Array %d: ", i);
            for(int j = 0; j < light_entry_num; ++j){
                printf("(%d, %d)", fermatEle->get_id(i,j), fermatEle->get_counter(i,j));
            }
            printf("\n");
        }
        return true;
    
    }

    bool write2file(char* filename){
        //rename to filename + "heavy"
        // FILE *fp = fopen(filename, "w");
        string heavyFilename = "./outputs/heavy_" + string(filename);
        string lightFilename = "./outputs/light_" + string(filename);
        FILE *fp = fopen(heavyFilename.c_str(), "w");
        // std::ofstream outFile2(heavyFilename.c_str());
        if(fp == NULL){
            printf("Open file failed!\n");
            return false;
        }
        //Heavy part
        for(int i = 0; i < heavy_bucket_num; ++i){
            for(int j = 0; j < MAX_VALID_COUNTER; ++j){
                fprintf(fp, "(%u, %d)", heavy_part->buckets[i].key[j], GetCounterVal(heavy_part->buckets[i].val[j]));
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
        //Light part
        FILE *fp2 = fopen(lightFilename.c_str(), "w");
        for(int j = 0; j < light_entry_num; ++j){
            bool all_zero_flag = 1;
            for(int i = 0; i < light_array_num; ++i){
                if(fermatEle->get_id(i,j) != 0){
                    all_zero_flag = 0;
                    break;
                }
            }
            if(!all_zero_flag){
                fprintf(fp2, "%d: ", j);
                for(int i = 0; i < light_array_num; ++i){
                    fprintf(fp2, "(%u, %d) ", fermatEle->get_id(i,j), fermatEle->get_counter(i,j));
                }
                fprintf(fp2, "\n");
            }
        }
        fclose(fp2);
        return true;
    }
};

template<int bucket_num>
bool compareLightpart(FLCSketch<bucket_num> &sketch1, FLCSketch<bucket_num> &sketch2){
    int array_num_1 = sketch1.get_light_array_num();
    int array_num_2 = sketch2.get_light_array_num();
    int entry_num_1 = sketch1.get_light_entry_num();
    int entry_num_2 = sketch2.get_light_entry_num();
    bool flag = 1;
    if(array_num_1 != array_num_2){
        printf("Fermat array num is different!\n");
        return false;
    }
    if(entry_num_1 != entry_num_2){
        printf("Fermat entry num is different!\n");
        return false;
    }
    int array_num = array_num_1;
    int entry_num = entry_num_1;
    for(int i = 0; i < array_num; ++i){
        for(int j = 0; j < entry_num; ++j){
            if(sketch1.fermatEle->get_id(i,j) != sketch2.fermatEle->get_id(i,j)){
                printf("ID at (%d, %d) is different, %d, %d\n", i, j, sketch1.fermatEle->get_id(i,j), sketch2.fermatEle->get_id(i,j));
                // return false;
                flag = 0;
            }
            if(sketch1.fermatEle->get_counter(i,j) != sketch2.fermatEle->get_counter(i,j)){
                printf("Counter at (%d, %d) is different, %d, %d\n", i, j, sketch1.fermatEle->get_counter(i,j), sketch2.fermatEle->get_counter(i,j));
                // return false;
                flag = 0;
            }
        }
    }
    printf("Light parts are the same!\n");
    return true;
}


//Check whether the two sketches are the same in Heavy_part size, light part array num and entry num
template<int bucket_num>
bool check_sketches_same_size(FLCSketch<bucket_num> &sketch1, FLCSketch<bucket_num> &sketch2)
{
    if(sketch1.get_heavy_bucket_num() != sketch2.get_heavy_bucket_num()){
        printf("Heavy part size is different!\n");
        return false;
    }
    if(sketch1.get_light_array_num() != sketch2.get_light_array_num()){
        printf("Fermat array num is different!\n");
        return false;
    }
    if(sketch1.get_light_entry_num() != sketch2.get_light_entry_num()){
        printf("Fermat entry num is different!\n");
        return false;
    }
    return true;
}

/*
struct Bucket
{
	uint32_t key[COUNTER_PER_BUCKET];
	uint32_t val[COUNTER_PER_BUCKET];
};
*/
template<int bucket_num>
FLCSketch<bucket_num> Union(FLCSketch<bucket_num> &sketch1, FLCSketch<bucket_num> &sketch2, uint32_t init_seed = 37)
{
    //Check whether the two sketches are the same in size
    if(!check_sketches_same_size(sketch1, sketch2)){
        printf("Sketches are not the same size!\n");
        exit(1);
    }
    int heavy_bucket_num = sketch1.get_heavy_bucket_num();
    int array_num = sketch1.get_light_array_num();
    int entry_num = sketch1.get_light_entry_num();
    // FLCSketch<bucket_num> sketch3(heavy_bucket_num, array_num, entry_num, sketch1.ifFermatCount);
    FLCSketch<bucket_num> sketch3(heavy_bucket_num, array_num, entry_num, 2, 0, init_seed);
    //Heavy part
    alignas(64) Bucket* sketch1_buckets = sketch1.heavy_part->buckets;
    alignas(64) Bucket* sketch2_buckets = sketch2.heavy_part->buckets;
    alignas(64) Bucket* sketch3_buckets = sketch3.heavy_part->buckets;

    bool full1=1, full2=1;
    map<uint32_t, bool> key_sign_map;
    map<uint32_t, bool> key_sign_map_1;
    map<uint32_t, bool> key_sign_map_2;

    vector<uint32_t> kickout_keys;
    vector<uint32_t> kickout_vals;

    int print_key = 4063245528;
    for(int i = 0; i < heavy_bucket_num; ++i){
        int total_keys_num = 0;
        map<uint32_t, uint32_t> merged_keys_vals;
        
        // Merge keys and values from both buckets
        for(int j = 0; j < MAX_VALID_COUNTER; ++j){
            uint32_t key = sketch1_buckets[i].key[j];
            uint32_t originalVal = sketch1_buckets[i].val[j];
            uint32_t val = GetCounterVal(originalVal);
            if(key != 0){
                if(HIGHEST_BIT_IS_1(originalVal)){
                    key_sign_map[key] = 1;
                    key_sign_map_1[key] = 1;
                }
                else{
                    key_sign_map[key] = -1;
                    key_sign_map_1[key] = -1;
                }
                merged_keys_vals[key] += val;
            }else{
                full1 = 0;
            }

            uint32_t key2 = sketch2_buckets[i].key[j];
            uint32_t originalVal2 = sketch2_buckets[i].val[j];
            uint32_t val2 = GetCounterVal(originalVal2);
            if(key2 != 0){
                if(HIGHEST_BIT_IS_1(originalVal2)){
                    key_sign_map[key2] = 1;
                    key_sign_map_2[key2] = 1;
                }
                else{
                    key_sign_map[key2] = -1;
                    key_sign_map_2[key2] = -1;
                }
                merged_keys_vals[key2] += val2;
            }else{
                full2 = 0;
            }
        }

        total_keys_num = merged_keys_vals.size();

        vector<pair<uint32_t, uint32_t>> sorted_merged_keys_vals_vec(merged_keys_vals.begin(), merged_keys_vals.end());
        if(total_keys_num > 7){
            //sort the merged keys and values based on value
            sort(sorted_merged_keys_vals_vec.begin(), sorted_merged_keys_vals_vec.end(), [](const pair<uint32_t, uint32_t> &a, const pair<uint32_t, uint32_t> &b){
                return a.second < b.second;
            });
        
            // Remove the smallest values if there are more than 7 different keys
            for(int k = 0; k <  sorted_merged_keys_vals_vec.size() - 7; ++k){
                kickout_keys.push_back(sorted_merged_keys_vals_vec[k].first);
                kickout_vals.push_back(sorted_merged_keys_vals_vec[k].second);
            }
        }
        
        // Store the remaining keys and values in the new sketch
        int start_index = max(static_cast<int>(sorted_merged_keys_vals_vec.size()) - 7, 0);
        for(int k = start_index; k < sorted_merged_keys_vals_vec.size(); ++k){
            uint32_t key = sorted_merged_keys_vals_vec[k].first;
            uint32_t val = sorted_merged_keys_vals_vec[k].second;
            sketch3_buckets[i].key[k - start_index] = key;
            bool sure_1_not_set = (key_sign_map_1.count(key) == 0 && (!full1)) || (key_sign_map_1.count(key) > 0 && key_sign_map_1[key] == -1);
            bool sure_2_not_set = (key_sign_map_2.count(key) == 0 && (!full2)) || (key_sign_map_2.count(key) > 0 && key_sign_map_2[key] == -1);
            if((sure_1_not_set)&&(sure_2_not_set)){
                sketch3_buckets[i].val[k - start_index] = val;
            }
            else{
                sketch3_buckets[i].val[k - start_index] = val | 0x80000000;
            }
            // sketch3_buckets[i].val[k - start_index] = val;
        }
            
    }
    
    //light part
    // uint32_t **light_id1 = sketch1.fermatEle->id;
    // uint32_t **light_id2 = sketch2.fermatEle->id;
    // uint32_t **light_id3 = sketch3.fermatEle->id;
    // uint32_t **light_cnt1 = sketch1.fermatEle->counter;
    // uint32_t **light_cnt2 = sketch2.fermatEle->counter;
    // uint32_t **light_cnt3 = sketch3.fermatEle->counter;

    for(int i = 0; i < array_num; ++i){
        for(int j = 0; j < entry_num; ++j){
            // light_id3[i][j] = (PRIME_ID_IDP_CNTPM + (uint64_t)light_id1[i][j] + (uint64_t)light_id2[i][j]) % PRIME_ID_IDP_CNTPM;
            // light_cnt3[i][j] = light_cnt1[i][j]+ light_cnt2[i][j];
            uint32_t sketch3_id = ((uint64_t)(uint32_t)(sketch1.fermatEle->get_id(i,j)) + (uint64_t)(uint32_t)(sketch2.fermatEle->get_id(i,j))) % (uint64_t)PRIME_ID_IDP_CNTPM;
            int32_t sketch3_counter = sketch1.fermatEle->get_counter(i,j) + sketch2.fermatEle->get_counter(i,j);
            sketch3.fermatEle->set_id(i, j, sketch3_id);
            sketch3.fermatEle->set_counter(i, j, sketch3_counter);
            uint32_t checking = 3458834590;
            if(sketch1.fermatEle->get_id(i,j) == checking){
                cout << "Checking situation of " << checking << ":" << endl;
                cout << "sketch1: id = " << (uint32_t)(sketch1.fermatEle->get_id(i,j)) << ", counter = " << sketch1.fermatEle->get_counter(i,j) << endl;
                cout << "calculated id = " << sketch3_id << ", counter = " << sketch3_counter << endl;
                cout << "sketch2: id = " << (uint32_t)(sketch2.fermatEle->get_id(i,j)) << ", counter = " << sketch2.fermatEle->get_counter(i,j) << endl;
                cout << "sketch3: id = " << (uint32_t)sketch3_id << ", counter = " << sketch3_counter << endl;
            }

        }
    }
    //kick out
    for(int i = 0; i < kickout_keys.size(); ++i){
        uint32_t key = kickout_keys[i];
        uint32_t val = kickout_vals[i];
        sketch3.fermatEle->Insert(key, val);
    }
    return sketch3;

    //get total num of keys in different buckets
}

template<int bucket_num>
FLCSketch<bucket_num> Difference(FLCSketch<bucket_num> &sketch1, FLCSketch<bucket_num> &sketch2, uint32_t init_seed = 37)
{
    // Check whether the two sketches are the same in size
    if (!check_sketches_same_size(sketch1, sketch2))
    {
        printf("Sketches are not the same size!\n");
        exit(1);
    }

    int heavy_bucket_num = sketch1.get_heavy_bucket_num();
    int array_num = sketch1.get_light_array_num();
    int entry_num = sketch1.get_light_entry_num();

    FLCSketch<bucket_num> sketch3(heavy_bucket_num, array_num, entry_num, 2, 0, init_seed);
    printf("info about sketch3: heavy_bucket_num = %d, array_num = %d, entry_num = %d, fermatkind = %d\n", sketch3.get_heavy_bucket_num(), sketch3.get_light_array_num(), sketch3.get_light_entry_num(), sketch3.ifFermatCount);

    // Heavy part
    alignas(64) Bucket* sketch1_buckets = sketch1.heavy_part->buckets;
    alignas(64) Bucket* sketch2_buckets = sketch2.heavy_part->buckets;
    alignas(64) Bucket* sketch3_buckets = sketch3.heavy_part->buckets;

    vector<uint32_t> kickout_keys;
    vector<int32_t> kickout_vals;

    int print_key = 4063245528;
    cout << "Operating heavy part\n";
    for (int i = 0; i < heavy_bucket_num; i++)
    {
        bool full1=1, full2=1;
        bool have_signed_val_1 = 0, have_signed_val_2 = 0;
        bool kicked_flag_2 = 0;
        map<uint32_t, uint32_t> merged_keys_vals;
        map<uint32_t, bool> key_sign_map;
        map<uint32_t, bool> key_sign_map_1;
        map<uint32_t, bool> key_sign_map_2;
        for(int j = 0; j < MAX_VALID_COUNTER; ++j){
            uint32_t key = sketch1_buckets[i].key[j];
            uint32_t originalVal = sketch1_buckets[i].val[j];
            uint32_t val = GetCounterVal(originalVal);
            if(key != 0){
                if(HIGHEST_BIT_IS_1(originalVal)){
                    key_sign_map[key] = 1;
                    key_sign_map_1[key] = 1;
                    have_signed_val_1 = 1;
                }
                else{
                    key_sign_map[key] = -1;
                    key_sign_map_1[key] = -1;
                }
                merged_keys_vals[key] += val;
            }else{
                full1 = 0;
            }
        }
        for(int j = 0; j < MAX_VALID_COUNTER; ++j){
            uint32_t key = sketch2_buckets[i].key[j];
            uint32_t originalVal = sketch2_buckets[i].val[j];
            uint32_t val = GetCounterVal(originalVal);
            if(HIGHEST_BIT_IS_1(originalVal)){
                kicked_flag_2 = 1;              
            }
            if(key != 0){
                if(HIGHEST_BIT_IS_1(originalVal)){
                    kicked_flag_2 = 1;
                    key_sign_map[key] = 1;
                    key_sign_map_2[key] = 1;
                    have_signed_val_2 = 1;
                }
                else if(key_sign_map.count(key) == 0){
                    key_sign_map[key] = -1;
                    key_sign_map_2[key] = -1;
                }
                else{
                    key_sign_map_2[key] = -1;
                }
            }else{
                full2 = 0;
            }

            if(key != 0 && merged_keys_vals.count(key) > 0){
                if(merged_keys_vals[key] > val){
                    merged_keys_vals[key] -= val;
                }
                // else if(key_sign_map.count(key)>0 && key_sign_map[key]==0){
                else if(key_sign_map[key] == -1){
                    merged_keys_vals.erase(key);
                }
                else{
                    //kick out
                    kickout_keys.push_back(key);
                    kickout_vals.push_back(merged_keys_vals[key] - val);
                    merged_keys_vals.erase(key);
                }
            }
            else if(merged_keys_vals.count(key) == 0){
                kickout_keys.push_back(key);
                kickout_vals.push_back(0 - val);
            }
        }

        // Store the remaining keys and values in the new sketch
        int start_index = 0;
        for(auto it = merged_keys_vals.begin(); it != merged_keys_vals.end(); ++it){
            uint32_t key = it->first;
            uint32_t val = it->second;
            sketch3_buckets[i].key[start_index] = key;
            bool sure_1_not_set = (key_sign_map_1.count(key) == 0 && (!full1)) || (key_sign_map_1.count(key) > 0 && key_sign_map_1[key] == -1);
            bool sure_2_not_set = (key_sign_map_2.count(key) == 0 && (!full2)) || (key_sign_map_2.count(key) > 0 && key_sign_map_2[key] == -1);
            if((sure_1_not_set)&&(sure_2_not_set)){
                sketch3_buckets[i].val[start_index] = val;
            }
            else{
                sketch3_buckets[i].val[start_index] = val | 0x80000000;
            }
            start_index++;
        }
    }
    // Light part
    for (int i = 0; i < array_num; i++)
    {
        for (int j = 0; j < entry_num; j++)
        {
            uint32_t sketch1_id = sketch1.fermatEle->get_id(i, j);
            uint32_t sketch2_id = sketch2.fermatEle->get_id(i, j);
            int32_t sketch1_counter = sketch1.fermatEle->get_counter(i, j);
            int32_t sketch2_counter = sketch2.fermatEle->get_counter(i, j);
            uint32_t diff_id = ((uint64_t)PRIME_ID_IDP_CNTPM + (uint64_t)sketch1_id - (uint64_t)sketch2_id) % (uint64_t)PRIME_ID_IDP_CNTPM;
            sketch3.fermatEle->set_id(i, j, diff_id);
            sketch3.fermatEle->set_counter(i, j, sketch1_counter - sketch2_counter);
            if(sketch1_id == 3398410894){
                cout << "i: " << i << ", j: " << j << endl;
                cout << "sketch1_id: " << sketch1_id << ", sketch2_id: " << sketch2_id << ", sketch1_counter: " << sketch1_counter << ", sketch2_counter: " << sketch2_counter << endl;
                cout << "sketch1_id - sketch2_id: " << sketch1_id - sketch2_id << ", sketch1_counter - sketch2_counter: " << sketch1_counter - sketch2_counter << endl;
                cout << "diff_id: " << diff_id << " PRIME: " << PRIME_ID_IDP_CNTPM << endl;
                cout << "sketch3_id: " << (uint32_t)(sketch3.fermatEle->get_id(i, j)) << ", sketch3_counter: " << sketch3.fermatEle->get_counter(i, j) << endl;
            }
        }
    }
    // Kick out
    cout << "Operating kick out, kickout_keys.size() = " << kickout_keys.size() << endl;
    for (int i = 0; i < kickout_keys.size(); i++)
    {
        uint32_t key = kickout_keys[i];
        int32_t val = kickout_vals[i];
        if(key == 1699205447){
            cout << "Inserting into lightpart " << key << ", val = " << val << endl;
            cout << "id[0][4134176] = " << (uint32_t)(sketch3.fermatEle->get_id(0, 4134176)) << ", counter[0][4134176] = " << sketch3.fermatEle->get_counter(0, 4134176) << endl;
        }
        sketch3.fermatEle->Insert(key, val);
        if(key == 1699205447){
            cout << "After inserting..." << endl;
            cout << "id[0][4134176] = " << (uint32_t)(sketch3.fermatEle->get_id(0, 4134176)) << ", counter[0][4134176] = " << sketch3.fermatEle->get_counter(0, 4134176) << endl;
        }
    }
    cout << "Size of insertedflows in sketch1 is " << sketch1.fermatEle->insertedflows.size() << endl;
    cout << "Size of insertedflows in sketch2 is " << sketch2.fermatEle->insertedflows.size() << endl;
    cout << "Size of insertedflows in sketch3 is " << sketch3.fermatEle->insertedflows.size() << endl;
    return sketch3;
}

template<int bucket_num>
long double InnerProduct(FLCSketch<bucket_num>& sketch1, FLCSketch<bucket_num>& sketch2)
{
    cout << "Enter InnerProduct" << endl;
    std::ofstream outFile("./outputs/innerP_result_compare.csv");
    outFile << "key, type1, type2, est_val1, est_val2, real_val1, real_val2, est_innerP, real_innerP" << endl;
    long double innerProduct_light = 0;
    long double innerProduct_light_heavy = 0;
    long double innerProduct_heavy_light = 0;
    long double innerProduct_heavy = 0;
    long double innerProduct = 0;
    int array_num = sketch1.get_light_array_num();
    int entry_num = sketch1.get_light_entry_num();
    long double res[array_num];
    for (int i = 0; i < array_num; i++)
    {
        long double k = 0;
        for (int j = 0; j < entry_num; j++)
            k += 1ll * sketch1.fermatEle->get_counter(i, j) * sketch2.fermatEle->get_counter(i, j);
        res[i] = 1.0 * k / entry_num; //TODO: check if this is correct
    }
    long double re = 0;
    for (int i = 0; i < array_num; i++)
        re += res[i];
    innerProduct_light = 1.0 * re / array_num;
    // innerProduct += innerProduct_light;

    // cout << "Total inner product is " << innerProduct << endl;
    sketch1.decode();
    sketch2.decode();

    // 1. Calculate inner product of heavy part
    for (int i = 0; i < bucket_num; i++)
    {
        map<uint32_t, uint32_t> merged_keys_vals_1;
        map<uint32_t, uint32_t> merged_keys_vals_2;
        for(int j = 0; j < MAX_VALID_COUNTER; ++j){
            // cout << "Operating bucket " << i << ", counter " << j << endl;
            uint32_t key = sketch1.heavy_part->buckets[i].key[j];
            // cout << "Key is " << key << endl;
            uint32_t originalVal = sketch1.heavy_part->buckets[i].val[j];
            // cout << "Original value is " << originalVal << endl;
            uint32_t val = GetCounterVal(originalVal);
            // cout << "Value is " << val << endl;
            if(key != 0){
                // cout << "Key " << key << endl;
                if(merged_keys_vals_1.count(key) == 0){
                    // cout << "Key " << key << " does not exist in merged_keys_vals_1" << endl;
                    merged_keys_vals_1[key] = val;
                }
                else{
                    cout << "Key " << key << " exists in merged_keys_vals_1" << endl;
                    merged_keys_vals_1[key] += val;
                }
                // cout << "Merged_keys_vals_1[" << key << "] = " << merged_keys_vals_1[key] << endl;
                // 2. Calculate inner product of heavy part and light part
                uint32_t lightValEst = sketch2.fermatEle->undecoded_query((char*)&key);
                if((int)lightValEst < 0)
                    lightValEst = 0;
                uint32_t lightValWithDecoding = sketch2.query_only_light_part((char*)&key);
                innerProduct_heavy_light += val * lightValEst;
                // if(val * lightValEst > 1000)
                //     cout << "heavy_light_ValEst is " << val * lightValEst << endl;
                if(lightValEst != lightValWithDecoding)
                    outFile << key << ", heavy, light, " << val << ", " << lightValEst << ", " << val << ", " << lightValWithDecoding << ", " << val * lightValEst << ", " << val * lightValWithDecoding << endl;
                // cout << "InnerProduct is " << innerProduct << endl;
            }
        }
        // for(int j = 0; j < MAX_VALID_COUNTER; ++j){
        //     uint32_t key = sketch2.heavy_part->buckets[i].key[j];
        //     uint32_t val = sketch2.heavy_part->buckets[i].val[j];
        //     if(key != 0){
        //         merged_keys_vals_2[key] += val;
        //     }
        // }
        for(int j = 0; j < MAX_VALID_COUNTER; ++j){
            uint32_t key = sketch2.heavy_part->buckets[i].key[j];
            uint32_t originalVal = sketch2.heavy_part->buckets[i].val[j];
            uint32_t val = GetCounterVal(originalVal);
            //key!=0 and key exists in sketch1
            if(key != 0){
                if(merged_keys_vals_1.count(key) > 0){
                    innerProduct_heavy += merged_keys_vals_1[key] * val;
                    // 2. Calculate inner product of heavy part and light part
                }
                // cout << "Key " << key << endl;
                uint32_t lightValEst = sketch1.fermatEle->undecoded_query((char*)&key);
                if((int)lightValEst < 0)
                    lightValEst = 0;
                uint32_t lightValWithDecoding = sketch1.query_only_light_part((char*)&key);
                innerProduct_light_heavy += val * lightValEst;
                if(lightValEst != lightValWithDecoding)
                    outFile << key << ", light, heavy, " << lightValEst << ", " << val << ", " << lightValWithDecoding << "," << val << ", " << val * lightValEst << ", " << val * lightValWithDecoding << endl;
            }
        }

        // 2. Calculate inner product of heavy part and light part
        // for(int j = 0; j < MAX_VALID_COUNTER; ++j){
        //     uint32_t key = sketch1.heavy_part->buckets[i].key[j];
        //     uint32_t val = sketch1.heavy_part->buckets[i].val[j];
        //     if(key != 0){
        //         int32_t lightValEst = sketch2.fermatEle->undecoded_query((char*)&key);
        //         innerProduct += val * lightValEst;
        //     }
        // }
        // for(int j = 0; j < MAX_VALID_COUNTER; ++j){
        //     uint32_t key = sketch2.heavy_part->buckets[i].key[j];
        //     uint32_t val = sketch2.heavy_part->buckets[i].val[j];
        //     if(key != 0){
        //         int32_t lightValEst = sketch1.fermatEle->undecoded_query((char*)&key);
        //         innerProduct += val * lightValEst;
        //     }
        // }

    }

    innerProduct = innerProduct_light + innerProduct_heavy + innerProduct_light_heavy + innerProduct_heavy_light;

    cout << "Inner product with only light part involved is " << innerProduct_light << endl;
    cout << "Inner product with 1 heavy part and 2 light part involved is " << innerProduct_heavy_light << endl;
    cout << "Inner product with 1 light part and 2 heavy part involved is " << innerProduct_light_heavy << endl;
    cout << "Inner product with only heavy part involved is " << innerProduct_heavy << endl;
    cout << "Total inner product is " << innerProduct << endl;


    // 3. Calculate inner product of light part
    // long double Join(Sketch*_other){
	// 	const AGMS* other=(AGMS*)_other;
	// 	long double res[MAX_HASH_NUM];
	// 	for (int i = 0; i < d; i++){
	// 		long double k=0;
	// 		for (int j = 0; j < w; j++)
	// 			k+=1ll*counter[i][j]*other->counter[i][j];
	// 		res[i]=1.0*k/w;
	// 	}
	// 	long double re=0;
	// 	for(int i=0;i<d;i++)re+=res[i];
	// 	return 1.0*re/d;
	// }
    
    outFile.close();
    return innerProduct;
}
