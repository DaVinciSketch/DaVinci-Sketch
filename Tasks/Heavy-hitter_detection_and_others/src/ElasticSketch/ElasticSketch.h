#ifndef _ELASTIC_SKETCH_H_
#define _ELASTIC_SKETCH_H_

#include "HeavyPart.h"
#include "LightPart.h"
#include <map>
#include<algorithm>

template<int bucket_num, int tot_memory_in_bytes>
class ElasticSketch
{
public:
    // static constexpr int heavy_mem = bucket_num * COUNTER_PER_BUCKET * 8;
    // static constexpr int light_mem = tot_memory_in_bytes - heavy_mem;
    int heavy_mem;
    int light_mem;


    HeavyPart<bucket_num> *heavy_part;
    LightPart<tot_memory_in_bytes - bucket_num * COUNTER_PER_BUCKET * 8> *light_part;

public:
    ElasticSketch(int num_bucket, int light_mem_byte){ 

        // cout << "Enter Init of ElasticSketch!" << endl;
        heavy_mem = num_bucket;
        // cout << "Enter Init of ElasticSketch!" << endl;
        light_mem = light_mem_byte;
        // cout << "Enter Init of ElasticSketch!" << endl;
        heavy_part = new HeavyPart<bucket_num>(num_bucket);
        // cout << "Gonna enter Init of LightPart! template varry = " << tot_memory_in_bytes - bucket_num * COUNTER_PER_BUCKET * 8  << "light_mem = " << light_mem << endl;
        light_part = new LightPart<tot_memory_in_bytes - bucket_num * COUNTER_PER_BUCKET * 8>(light_mem);
        // cout << "ElasticSketch Init over!" << endl;
    }
    ~ElasticSketch(){}
    void clear();

    void insert(uint8_t *key, int f = 1);
    void quick_insert(uint8_t *key, int f = 1);

    int query(uint8_t *key);
    int query_compressed_part(uint8_t *key, uint8_t *compress_part, int compress_counter_num);

    int get_compress_width(int ratio) { return light_part->get_compress_width(ratio);}
    void compress(int ratio, uint8_t *dst) {    light_part->compress(ratio, dst); }

    int get_bucket_num() { return heavy_part->get_bucket_num(); }
    double get_bandwidth(int compress_ratio) ;

    void get_heavy_hitters(int threshold, vector<pair<string, int>> & results);
    int get_cardinality();
    double get_entropy();
    void get_distribution(vector<double> &dist);

    void *operator new(size_t sz);
    void operator delete(void *p);
};

template<int bucket_num, int tot_memory_in_bytes>
void Union(ElasticSketch<bucket_num, tot_memory_in_bytes> &sketch1, ElasticSketch<bucket_num, tot_memory_in_bytes> &sketch2, ElasticSketch<bucket_num, tot_memory_in_bytes> &sketch3) 
{
    const int heavy_bucket_num = sketch1.get_bucket_num();
    const int array_num = bucket_num;
    const int entry_num = MAX_VALID_COUNTER;

    // get HeavyPart
    Bucket* sketch1_buckets = sketch1.heavy_part->buckets;
    Bucket* sketch2_buckets = sketch2.heavy_part->buckets;
    Bucket* sketch3_buckets = sketch3.heavy_part->buckets;

    std::vector<uint32_t> kickout_keys;
    std::vector<uint32_t> kickout_vals;

    // every bucket
    for(int i = 0; i < heavy_bucket_num; ++i) {
        std::map<uint32_t, uint32_t> merged_keys_vals; // 存储合并后的键值对
        bool full1 = true, full2 = true;

        // key & value
        for(int j = 0; j < entry_num; ++j) {
            uint32_t key1 = sketch1_buckets[i].key[j];
            uint32_t val1 = sketch1_buckets[i].val[j];
            if(key1 != 0) {
                merged_keys_vals[key1] += GetCounterVal(val1);
                if (full1 && j >= entry_num - 1) full1 = false; // sketch1 桶满的标志
            }

            uint32_t key2 = sketch2_buckets[i].key[j];
            uint32_t val2 = sketch2_buckets[i].val[j];
            if(key2 != 0) {
                merged_keys_vals[key2] += GetCounterVal(val2);
                if (full2 && j >= entry_num - 1) full2 = false; // sketch2 桶满的标志
            }
        }

        // save to sketch3
        std::vector<std::pair<uint32_t, uint32_t>> sorted_keys_vals(merged_keys_vals.begin(), merged_keys_vals.end());
        std::sort(sorted_keys_vals.begin(), sorted_keys_vals.end(), [](const std::pair<uint32_t, uint32_t> &a, const std::pair<uint32_t, uint32_t> &b) {
            return a.second < b.second;
        });

        int start_index = sorted_keys_vals.size() > 7 ? sorted_keys_vals.size() - 7 : 0;
        for(int k = start_index; k < sorted_keys_vals.size(); ++k) {
            sketch3_buckets[i].key[k - start_index] = sorted_keys_vals[k].first;
            sketch3_buckets[i].val[k - start_index] = sorted_keys_vals[k].second;
        }

        // deal with kickout_keys & vals
        for(int k = 0; k < start_index; ++k) {
            kickout_keys.push_back(sorted_keys_vals[k].first);
            kickout_vals.push_back(sorted_keys_vals[k].second);
        }
    }

    // insert kickout_keys & vals to sketch3.light_part
    for(int i = 0; i < kickout_keys.size(); ++i) {
        uint8_t key_bytes[sizeof(uint32_t)];
        memcpy(key_bytes, &kickout_keys[i], sizeof(uint32_t));
        sketch3.light_part->insert(key_bytes, kickout_vals[i]);
    }
}

#endif // _ELASTIC_SKETCH_H_