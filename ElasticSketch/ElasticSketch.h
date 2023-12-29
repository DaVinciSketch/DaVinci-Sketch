#ifndef _ELASTIC_SKETCH_H_
#define _ELASTIC_SKETCH_H_

#include "HeavyPart.h"
#include "LightPart.h"


template<int bucket_num, int tot_memory_in_bytes>
class ElasticSketch
{
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



#endif // _ELASTIC_SKETCH_H_
