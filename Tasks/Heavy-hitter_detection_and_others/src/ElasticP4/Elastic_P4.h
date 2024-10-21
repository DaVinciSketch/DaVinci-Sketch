#ifndef _ELASTIC_P4_H_
#define _ELASTIC_P4_H_

#include "./HeavyPart.h"
#include "./LightPart.h"

class ElasticSketch
{
    public:
    HeavyPart * heavy_part[ELASTIC_HEAVY_STAGE] = {NULL};
    LightPart * light_part = NULL;
    BOBHash32* hash_heavy[ELASTIC_HEAVY_STAGE] = {NULL};
    int memory_access_counter;

public:
    ElasticSketch(){
        int init = INIT;
        for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
            heavy_part[i] = new HeavyPart();
            hash_heavy[i] = new BOBHash32(init+i);
        }
        light_part = new LightPart();
        clear();
        // printf("[ElasticP4_Simulator] Memory Usage : %2.2f MB\n", (ELASTIC_WL + ELASTIC_HEAVY_STAGE*ELASTIC_BUCKET * 12.0) / 1024 / 1024);
        // if (ELASTIC_TOFINO)
        //     printf("[ElasticP4_Tofino] Memory Usage : %2.2f MB\n", (ELASTIC_WL + ELASTIC_HEAVY_STAGE*ELASTIC_BUCKET * 28.0) / 1024 / 1024);
        // else
        //     printf("[ElasticP4_Simulator] Memory Usage : %2.2f MB\n", (ELASTIC_WL + ELASTIC_HEAVY_STAGE*ELASTIC_BUCKET * 12.0) / 1024 / 1024);
    }
    ElasticSketch(int init){
        for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
            heavy_part[i] = new HeavyPart();
            hash_heavy[i] = new BOBHash32(init+i);
        }
        light_part = new LightPart();
        clear();
        // printf("[ElasticP4_Simulator] Memory Usage : %2.2f MB\n", (ELASTIC_WL + ELASTIC_HEAVY_STAGE*ELASTIC_BUCKET * 12.0) / 1024 / 1024);
        // if (ELASTIC_TOFINO)
        //     printf("[ElasticP4_Tofino] Memory Usage : %2.2f MB\n", (ELASTIC_WL + ELASTIC_HEAVY_STAGE*ELASTIC_BUCKET * 28.0) / 1024 / 1024);
        // else
        //     printf("[ElasticP4_Simulator] Memory Usage : %2.2f MB\n", (ELASTIC_WL + ELASTIC_HEAVY_STAGE*ELASTIC_BUCKET * 12.0) / 1024 / 1024);
    }
    ~ElasticSketch(){
        delete light_part;
        for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
            delete heavy_part[i];
            delete hash_heavy[i];
        }
    }
    void clear()
    {
        for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i)
            heavy_part[i]->clear();
        light_part->clear();
    }

    void insert(uint8_t *key, int f=1)
    {
        uint8_t swap_key[4]; // metafield for key
        uint32_t swap_val = 0; // metafield for value
        int pos;
        int result;
        int val = f;
        memory_access_counter = 0;
        for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
            pos = hash_heavy[i]->run((const char *)key, 4) % ELASTIC_BUCKET;
            result = heavy_part[i]->insert(key, swap_key, swap_val, pos, val);
            memory_access_counter += heavy_part[i]->hp_memory_access_counter;

            switch(result) {
                case 0: return;
                case 1:{ // key, f
                    swap_val = 1;
                    continue;
                }
                case 2:{ // swap_key, swap_val
                    std::copy(swap_key, swap_key + 4, key);
                    continue;
                }
            }
        }
        // if (swap_val)
        //     cout << "[Elastic] swap_val = " << swap_val << endl; 
        light_part->insert(key, swap_val);
        memory_access_counter += light_part->lp_memory_access_counter;
        return;
    }

    uint32_t query(uint8_t *key) // 4 bytes prefix
    {
        uint32_t ret_val = 0;
        int pos;
        for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
            pos = hash_heavy[i]->run((const char*)key, 4) % ELASTIC_BUCKET;
            ret_val += heavy_part[i]->query(key, pos);
        }
        ret_val += light_part->query(key);
        return ret_val;
    }

    void get_heavy_hitters(set<uint32_t> & results)
    {
        for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
            for (int j = 0; j < ELASTIC_BUCKET; ++j) {
                uint32_t key = heavy_part[i]->buckets[j].key;
                uint32_t val = this->query((uint8_t *)&key);
                if (val > HH_THRESHOLD)
                    results.insert(key);
            }
        }
    }
    void get_hc_candidates(unordered_map<uint32_t, int>& hc_candidates)
    {
        for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
            for (int j = 0; j < ELASTIC_BUCKET; ++j) {
                uint32_t key = heavy_part[i]->buckets[j].key;
                uint32_t val = this->query((uint8_t *)&key);
                if (val > HC_THRESHOLD)
                    hc_candidates[key] = val;
            }
        }
    }

/* interface */
    int get_cardinality()
    {

        int num_light_nonzero = 0;

        int card = light_part->get_cardinality();
        // step 1. collecting all items in Heavy Part
        set<uint32_t> checked_items;
        for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
            for (int j = 0; j < ELASTIC_BUCKET; ++j){
                uint32_t key = heavy_part[i]->buckets[j].key;
                if (key != 0)
                    checked_items.insert(key);
            }
        }

        // step 2. Merge heavy & light cardinality
        for (auto it = checked_items.begin(); it != checked_items.end(); ++it){
            uint32_t sum_val = 0;
            // heavy part query
            uint32_t heavy_val = 0;
            uint8_t key[4]; // temp key
            for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
                *(uint32_t*)key = *it;
                int pos = hash_heavy[i]->run((const char*)key, 4) % ELASTIC_BUCKET;
                heavy_val += heavy_part[i]->query(key, pos);
            }
            sum_val += heavy_val;

            // light part query
            uint32_t light_val = light_part->query(key);
            if (light_val != 0){
                sum_val += light_val;
                card--;
                num_light_nonzero++; 
            }
            if (sum_val){
                card++;
            }
        }
        return card;
    }

    double get_entropy()
    {
        int tot = 0;
        double entr = 0;

        light_part->get_entropy(tot, entr);

        // step 1. collecting all items in Heavy Part
        set<uint32_t> checked_items;
        for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
            for (int j = 0; j < ELASTIC_BUCKET; ++j){
                uint32_t key = heavy_part[i]->buckets[j].key;
                if (key != 0)
                    checked_items.insert(key);
            }
        }

        // step 2. Merge heavy & light entropy
        for (auto it = checked_items.begin(); it != checked_items.end(); ++it){
            uint32_t sum_val = 0;
            // heavy part query
            uint32_t heavy_val = 0;
            uint8_t key[4]; // temp key
            for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
                *(uint32_t*)key = *it;
                int pos = hash_heavy[i]->run((const char*)key, 4) % ELASTIC_BUCKET;
                heavy_val += heavy_part[i]->query(key, pos);
            }
            sum_val += heavy_val;

            // light part query
            uint32_t light_val = light_part->query(key);

            if (light_val != 0){
                sum_val += light_val;
                tot -= light_val;
                entr -= light_val * log2(light_val);
            }
            if (sum_val){
                tot += sum_val;
                entr += sum_val * log2(sum_val);
            }
        }
        return -entr / tot + log2(tot);
    }

    void get_distribution(vector<double> &dist)
    {
        // printf("*** Start getting distribution in light part\n");
        light_part->get_distribution(dist);
        
        // printf("*** Finish getting distribution in light part\n");

        // step 1. collecting all items in Heavy Part
        set<uint32_t> checked_items;
        for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
            for (int j = 0; j < ELASTIC_BUCKET; ++j){
                uint32_t key = heavy_part[i]->buckets[j].key;
                if (key != 0)
                    checked_items.insert(key);
            }
        }
 
        // step 2. Merge heavy & light entropy
        for (auto it = checked_items.begin(); it != checked_items.end(); ++it){
            uint32_t sum_val = 0;
            // heavy part query
            uint32_t heavy_val = 0;
            uint8_t key[4]; // temp key
            for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i){
                *(uint32_t*)key = *it;
                int pos = hash_heavy[i]->run((const char*)key, 4) % ELASTIC_BUCKET;
                heavy_val += heavy_part[i]->query(key, pos);
            }
            sum_val += heavy_val;

            // light part query
            uint32_t light_val = light_part->query(key);

            if (light_val != 0){
                sum_val += light_val;
                dist[light_val]--;
            }
            if (sum_val){
                if (sum_val + 1 > dist.size())
                    dist.resize(sum_val + 1);
                dist[sum_val]++;
            }
        }
    }

    void Union(const ElasticSketch &sketch1, const ElasticSketch &sketch2, ElasticSketch &sketch3) 
    {
        sketch3.clear();
        // �ϲ� HeavyPart
        for (int i = 0; i < ELASTIC_HEAVY_STAGE; ++i) {
            for (int j = 0; j < ELASTIC_BUCKET; ++j) {
                uint32_t key1 = sketch1.heavy_part[i]->buckets[j].key;
                uint32_t val1 = sketch1.heavy_part[i]->buckets[j].val;
                uint32_t key2 = sketch2.heavy_part[i]->buckets[j].key;
                uint32_t val2 = sketch2.heavy_part[i]->buckets[j].val;
                // ���������������ͬλ�ö��м�����������ǵ�ֵ
                if (key1 != 0 && key2 != 0 && key1 == key2) {
                    uint8_t key_bytes[4];
                    std::copy(reinterpret_cast<uint8_t*>(&key1), reinterpret_cast<uint8_t*>(&key1) + 4, key_bytes);
                    uint32_t sum_val = val1 + val2;
                    sketch3.insert(key_bytes, sum_val);
                } else if (key1 != 0) {
                    uint8_t key_bytes[4];
                    std::copy(reinterpret_cast<uint8_t*>(&key1), reinterpret_cast<uint8_t*>(&key1) + 4, key_bytes);
                    sketch3.insert(key_bytes, val1);
                } else if (key2 != 0) {
                    uint8_t key_bytes[4];
                    std::copy(reinterpret_cast<uint8_t*>(&key2), reinterpret_cast<uint8_t*>(&key2) + 4, key_bytes);
                    sketch3.insert(key_bytes, val2);
                }
            }
        }
        // �ϲ� LightPart
        for (int i = 0; i < ELASTIC_WL; ++i) {
            uint8_t count1 = sketch1.light_part->counters[i];
            uint8_t count2 = sketch2.light_part->counters[i];
            uint8_t sum_count = count1 + count2;
            if (sum_count < 255) {
                sketch3.light_part->counters[i] = sum_count;
            } else {
                sketch3.light_part->counters[i] = 255;
            }
            sketch3.light_part->mice_dist[count1]--;
            sketch3.light_part->mice_dist[count2]--;
            sketch3.light_part->mice_dist[sketch3.light_part->counters[i]]++;
        }
    }
};



#endif
