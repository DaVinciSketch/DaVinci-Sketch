#ifndef _FERMAT_H_
#define _FERMAT_H_

#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <queue>
#include <cstring>
// #include "util/BOBHash32.h"
#include "common/BOBHash32.h"
#include "util/mod.h"
#include "util/prime.h"
#include "util/murmur3.h"
#include <vector>
#include <algorithm>
#include <variant>
#include <cassert>
using namespace std;

#define HASH_TO_SIGN(hash_value) (((hash_value) & 1) ? -1 : 1)
#define DEBUG_F 0

// fingprint no used

// use a 16-bit prime, so 2 * a mod PRIME will not overflow
static const uint32_t PRIME_ID = MAXPRIME[32]; //PRIME for Fermat_sketch
static const uint32_t PRIME_ID_IDP_CNTPM = MAXPRIME[32]; //PRIME for Fermat_sketch
static const uint32_t PRIME_ID_COUNT = MAXPRIME[31]; //PRIME for Fermat_count
static const uint32_t PRIME_FING = MAXPRIME[32];

inline uint64_t checkTable(uint64_t pos)
{
    // cout << "Using in checkTable PRIME_ID: " << PRIME_ID << endl;
    return powMod32(pos, PRIME_ID - 2, PRIME_ID);
}

inline uint64_t checkTable_count(uint64_t pos, uint32_t prime = PRIME_ID_IDP_CNTPM)
{
    // cout << "Using in checkTable_count PRIME_ID_COUNT: " << PRIME_ID_COUNT << endl;
    return powMod32(pos, prime - 2, prime);
}


using DataVariant = std::variant<std::unordered_map<int, int>, std::unordered_map<unsigned int, int>>;

class Fermat
{
    // bool use_fing;


public:

    int pure_cnt;
    unordered_map<int32_t, int> insertedflows;

    

    virtual void clear_look_up_table() = 0;
    virtual void create_array() = 0;
    virtual void clear_array() = 0;

    virtual void Insert(uint32_t flow_id, uint32_t cnt) = 0;
    // virtual void Insert() = 0;
    virtual void Insert_one(uint32_t flow_id) = 0;

    virtual void Delete_in_one_bucket(int row, int col, int pure_row, int pure_col, int sign = 1) = 0;

    // virtual bool verify(int row, int col, uint32_t &flow_id, uint32_t &fing) = 0;
    virtual int verify(int row, int col, uint32_t &flow_id, uint32_t &fing) = 0;

    virtual void display() = 0;
    virtual int query(const char *key) = 0;
    virtual int undecoded_query(const char *key) = 0;
    // virtual bool Decode(unordered_map<uint32_t, int> &result);
    // virtual bool Decode(unordered_map<int32_t, int> &result);
    virtual bool Decode(DataVariant& data) = 0;
    virtual int get_id(int n_array, int n) = 0;
    virtual int get_counter(int n_array, int n) = 0;

    virtual int set_id(int n_array, int n, int value) = 0;
    virtual int set_counter(int n_array, int n, int value) = 0;

    virtual int get_array_num() = 0;
    virtual int get_entry_num() = 0;

    virtual int query_from_cpy(const char *key){
        cout << "query_from_cpy() is not implemented in root class!" << endl;
        return -1;
    }
    virtual int query_after_decoding(const char *key){
        cout << "query_after_decoding() is not implemented in root class!" << endl;
        return -1;
    }
    virtual int undecoded_query_before_decoding(const char *key){
        cout << "undecoded_query_before_decoding() is not implemented in root class!" << endl;
        return -1;
    }

    bool cpy_counters(){ 
        cout << "cpy_counters() is not implemented!" << endl;
        return false;
    }

    virtual bool cpy_counters_to_pos(int32_t ***countercpy){
        cout << "cpy_counters_to() is not implemented!" << endl;
        return false;
    }
    
    virtual int query_array(const char *key, int array_index){
        cout << "query_array() is not implemented in root class!" << endl;
        return -1;
    }

    virtual ~Fermat() {};
};

class Fermat_Sketch : public Fermat
{
    int array_num;
    int entry_num;
    int decodeflag = 0;
    // hash
    BOBHash32 *hash;
    BOBHash32 *hash_fp;

    uint32_t *table;

    bool use_fing;
    // arrays
    // int array_num;
    // int entry_num;
    uint32_t **id;
    uint32_t **counter;
    uint32_t **fingerprint;
    uint32_t **idcpy, **fingcpy, **countercpy;
    // int decodeflag = 0;
    // // hash
    // BOBHash32 *hash;
    // BOBHash32 *hash_fp;

    // uint32_t *table;

    // bool use_fing;

public:


    void clear_look_up_table() override
    {
        delete[] table;
    }

    void create_array() override
    {
        pure_cnt = 0;
        // id
        id = new uint32_t *[array_num];
        for (int i = 0; i < array_num; ++i)
        {
            id[i] = new uint32_t[entry_num];
            memset(id[i], 0, entry_num * sizeof(uint32_t));
        }
        // fingerprint
        if (use_fing)
        {
            fingerprint = new uint32_t *[array_num];
            for (int i = 0; i < array_num; ++i)
            {
                fingerprint[i] = new uint32_t[entry_num];
                memset(fingerprint[i], 0, entry_num * sizeof(uint32_t));
            }
        }

        // counter
        counter = new uint32_t *[array_num];
        for (int i = 0; i < array_num; ++i)
        {
            counter[i] = new uint32_t[entry_num];
            memset(counter[i], 0, entry_num * sizeof(uint32_t));
        }
    }

    void clear_array() override
    {
        for (int i = 0; i < array_num; ++i)
            delete[] id[i];
        delete[] id;

        if (fingerprint)
        {
            for (int i = 0; i < array_num; ++i)
                delete[] fingerprint[i];
            delete[] fingerprint;
        }

        for (int i = 0; i < array_num; ++i)
            delete[] counter[i];
        delete[] counter;
    }

    Fermat_Sketch(int _a, int _e, bool _fing, uint32_t _init) : array_num(_a), entry_num(_e), use_fing(_fing), fingerprint(nullptr), hash_fp(nullptr)
    {
        cout << "You are running Fermat Sketch version. Prime for ID: " << PRIME_ID <<endl;
        create_array();
        // hash
        if (use_fing)
            hash_fp = new BOBHash32(_init);
        hash = new BOBHash32[array_num];
        for (int i = 0; i < array_num; ++i)
        {
            hash[i].initialize(_init + (10 * i) + 1);
        }
    }

    Fermat_Sketch(int _memory, bool _fing, uint32_t _init) : use_fing(_fing), fingerprint(nullptr), hash_fp(nullptr)
    {
        printf("You are running Fermat Sketch version.\n");
        array_num = 3;
        if (use_fing)
            entry_num = _memory / (array_num * 12);
        else
            entry_num = _memory / (array_num * 8);

        // cout << "construct fermat with " << entry_num << " entry" << endl;
        create_array();

        // hash
        if (use_fing)
            hash_fp = new BOBHash32(_init);
        hash = new BOBHash32[array_num];
        for (int i = 0; i < array_num; ++i)
            hash[i].initialize(_init + i + 1);
    }

    ~Fermat_Sketch() override
    {
        clear_array();
        clear_look_up_table();
        if (hash_fp)
            delete hash_fp;
        delete[] hash;
    }

    void Insert(uint32_t flow_id, uint32_t cnt) override
    {
        insertedflows[flow_id]+=cnt;
        if (use_fing)
        {
            uint32_t fing = hash_fp->run((char *)&flow_id, sizeof(uint32_t));
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                id[i][pos] = (mulMod32(flow_id, cnt, PRIME_ID) + (uint64_t)id[i][pos]) % PRIME_ID;
                fingerprint[i][pos] = ((uint64_t)fingerprint[i][pos] + mulMod32(fing, cnt, PRIME_FING)) % PRIME_FING;
                counter[i][pos] += cnt;
            }
        }
        else
        {
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                id[i][pos] = (mulMod32(flow_id, cnt, PRIME_ID) + (uint64_t)id[i][pos]) % PRIME_ID;
                counter[i][pos] += cnt;
            }
        }
    }

    void Insert_one(uint32_t flow_id) override
    {
        // flow_id should < PRIME_ID
        if (use_fing)
        {
            uint32_t fing = hash_fp->run((char *)&flow_id, sizeof(uint32_t)) % PRIME_FING;
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                id[i][pos] = ((uint64_t)id[i][pos] + (uint64_t)(flow_id % PRIME_ID)) % PRIME_ID;
                fingerprint[i][pos] = ((uint64_t)fingerprint[i][pos] + (uint64_t)fing) % PRIME_FING;
                counter[i][pos]++;
            }
        }
        else
        {
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                id[i][pos] = ((uint64_t)id[i][pos] + (uint64_t)(flow_id % PRIME_ID)) % PRIME_ID;
                counter[i][pos]++;
            }
        }
    }

    void Delete_in_one_bucket(int row, int col, int pure_row, int pure_col, int sign = 1) override
    {
        // delete (flow_id, fing, cnt) in bucket (row, col)
        id[row][col] = ((uint64_t)PRIME_ID + (uint64_t)id[row][col] - (uint64_t)id[pure_row][pure_col]) % PRIME_ID;
        if (use_fing)
            fingerprint[row][col] = ((uint64_t)PRIME_FING + (uint64_t)fingerprint[row][col] - (uint64_t)fingerprint[pure_row][pure_col]) % PRIME_FING;
        counter[row][col] -= counter[pure_row][pure_col];
        
    }

    // bool verify(int row, int col, uint32_t &flow_id, uint32_t &fing) override
    int verify(int row, int col, uint32_t &flow_id, uint32_t &fing) override
    {
        // cout << "I'm in verify of fermat_sketch!\n";
#if DEBUG_F
        ++pure_cnt;
#endif
        if (counter[row][col] & 0x80000000)
        {
            // cout << "I'm in verify(sketch) and counter[row][col]'s highest pos is 1!" << endl;
            uint64_t temp = checkTable(~counter[row][col] + 1);
            flow_id = mulMod32(PRIME_ID - id[row][col], temp, PRIME_ID);
        }
        else
        {
            // cout << "I'm in verify(sketch) and counter[row][col]'s highest pos is 0!" << endl;
            uint64_t temp = checkTable(counter[row][col]);
            flow_id = mulMod32(id[row][col], temp, PRIME_ID);
        }
        // flow_id = (id[row][col] * table[counter[row][col] % PRIME_ID]) % PRIME_ID;
        if (use_fing)
        {
            fing = powMod32(counter[row][col], PRIME_FING - 2, PRIME_FING);
            fing = mulMod32(fingerprint[row][col], fing, PRIME_FING);
        }
        if (!(hash[row].run((char *)&flow_id, sizeof(int32_t)) % entry_num == col))
            return false;
        if (use_fing && !(hash_fp->run((char *)&flow_id, sizeof(uint32_t)) % PRIME_FING == fing))
            return false;
        return true;
    }

    void display() override
    {
        cout << " --- display --- " << endl;
        for (int i = 0; i < array_num; ++i)
        {
            for (int j = 0; j < entry_num; ++j)
            {
                if (counter[i][j])
                {
                    cout << i << "," << j << ":" << counter[i][j] << endl;
                }
            }
        }
    }
    int query(const char *key) override
    {
        uint32_t flow_id = *(uint32_t *)key;
        uint32_t ret = 1 << 30;
        if (decodeflag)
        {
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                ret = min(counter[i][pos], ret);
            }
        }
        return (int)ret;
    }
    int undecoded_query(const char *key) override
    {
        // cout << "Counting Min!\n";
        uint32_t flow_id = *(uint32_t *)key;
        uint32_t ret = 1 << 30;
        
        for (int i = 0; i < array_num; ++i)
        {
            uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
            ret = min(counter[i][pos], ret);
        }
        
        return (int)ret;
    }
    int undecoded_query_before_decoding(const char *key) override {
        cout << "Undecoded query of fermat_sketch is not implemented!" << endl;
        return -1;
    }
    // bool Decode(unordered_map<uint32_t, int> &result) override
    bool Decode(DataVariant& data) override
    {
        auto* mapPtr = std::get_if<std::unordered_map<unsigned int, int>>(&data);
        if (!mapPtr) {
            return false;  // 如果类型不匹配，则直接返回 false
        }
        
        auto& result = *mapPtr;
        
        // for (int i = 0; i < array_num; i++){
        //     for (int j = 0; j < entry_num; j++){
        //         cout << counter[i][j] << " ";
        //     }
        //     cout << endl <<endl;
        // }
        idcpy = new uint32_t *[array_num];
        for (int i = 0; i < array_num; i++)
        {
            idcpy[i] = new uint32_t[entry_num];
            for (int j = 0; j < entry_num; j++)
                idcpy[i][j] = id[i][j];
        }
        if (use_fing)
        {
            fingcpy = new uint32_t *[array_num];
            for (int i = 0; i < array_num; i++)
            {
                fingcpy[i] = new uint32_t[entry_num];
                for (int j = 0; j < entry_num; j++)
                    fingcpy[i][j] = fingerprint[i][j];
            }
        }
        countercpy = new uint32_t *[array_num];
        for (int i = 0; i < array_num; i++)
        {
            countercpy[i] = new uint32_t[entry_num];
            for (int j = 0; j < entry_num; j++)
                countercpy[i][j] = counter[i][j];
        }
        decodeflag = 1;
        queue<int> *candidate = new queue<int>[array_num];
        uint32_t flow_id = 0;
        uint32_t fing = 0;

        // first round
        for (int i = 0; i < array_num; ++i)
            for (int j = 0; j < entry_num; ++j)
            {
                if (counter[i][j] == 0)
                {
                    continue;
                }
                else if (verify(i, j, flow_id, fing))
                {
                    // find pure bucket
                    if (result.count(flow_id) != 0)
                    {
                        result[flow_id] += counter[i][j];
                    }
                    else
                    {
                        result[flow_id] = counter[i][j];
                    }
                    // delete flow from other rows
                    for (int t = 0; t < array_num; ++t)
                    {
                        if (t == i)
                            continue;
                        uint32_t pos = hash[t].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                        Delete_in_one_bucket(t, pos, i, j);
                        candidate[t].push(pos);
                    }
                    Delete_in_one_bucket(i, j, i, j);
                }
            }

        bool pause;
        int acc = 0;
        while (true)
        {
            acc++;
            pause = true;
            for (int i = 0; i < array_num; ++i)
            {
                if (!candidate[i].empty())
                    pause = false;
                while (!candidate[i].empty())
                {
                    int check = candidate[i].front();
                    candidate[i].pop();
                    // cout << i << " " << check << endl;
                    if (counter[i][check] == 0)
                    {
                        continue;
                    }
                    else if (verify(i, check, flow_id, fing))
                    {
                        // find pure bucket
                        if (result.count(flow_id) != 0)
                        {
                            result[flow_id] += counter[i][check];
                        }
                        else
                        {
                            result[flow_id] = counter[i][check];
                        }
                        // delete flow from other rows
                        for (int t = 0; t < array_num; ++t)
                        {
                            if (t == i)
                                continue;
                            uint32_t pos = hash[t].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                            Delete_in_one_bucket(t, pos, i, check);
                            candidate[t].push(pos);
                        }
                        Delete_in_one_bucket(i, check, i, check);
                    }
                }
            }
            if (pause){
                printf("Break because pause!\n");
                break;

            }
            if (acc > 10000)
                printf("Break because acc is too big!\n");
                break;
        }
        printf("Get out of while in decode in fermat.h\n");

        delete[] candidate;
        bool flag = true;
        for (int i = 0; i < array_num; ++i)
            for (int j = 0; j < entry_num; ++j)
                if (counter[i][j] != 0)
                {
                    // cout << "undecoded  i " << i << " j " << j << endl;
                    // cout << counter[i][j] << endl;
                    flag = false;
                }
        for (auto p : result)
        {
            if (p.second == 0)
            {
                result.erase(p.first);
            }
        }
        return flag;
    }
    int get_id(int n_array, int n){
        if(n_array >=0 && n_array <= array_num && n >= 0 && n <= entry_num){
            return id[n_array][n];
        }
        else{
            cout << "get_id() out of range!" << endl;
            assert(0);
        }
    }
    int get_counter(int n_array, int n){
        if(n_array >=0 && n_array <= array_num && n >= 0 && n <= entry_num){
            return counter[n_array][n];
        }
        else{
            cout << "get_counter() out of range!" << endl;
            assert(0);
        }
    }
    int set_id(int n_array, int n, int value){
        if(n_array >=0 && n_array <= array_num && n >= 0 && n <= entry_num){
            id[n_array][n] = value;
            return 0;
        }
        else{
            cout << "set_id() out of range! " << n_array << "/" << array_num << " " << n << "/" << entry_num << endl;
            assert(0);
        }
    }
    int set_counter(int n_array, int n, int value){
        if(n_array >=0 && n_array <= array_num && n >= 0 && n <= entry_num){
            counter[n_array][n] = value;
            return 0;
        }
        else{
            cout << "set_counter() out of range!" << endl;
            assert(0);
        }
    }
    int get_array_num() override{
        return array_num;
    }
    int get_entry_num() override{
        return entry_num;
    }
};

class Fermat_Count : public Fermat
{
    int array_num;
    int entry_num;
    int decodeflag = 0;
    // hash
    BOBHash32 *hash;
    BOBHash32 *hash_fp;

    uint32_t *table;

    bool use_fing;
    // arrays
    // int array_num;
    // int entry_num;
    int32_t **id;
    int32_t **fingerprint;
    int32_t **counter;
    int32_t **idcpy, **countercpy;
    int32_t **fingcpy;
    // int decodeflag = 0;
    // // hash
    // BOBHash32 *hash;
    // BOBHash32 *hash_fp;
    BOBHash32 *hash_sign;

    // uint32_t *table;

    // bool use_fing;

public:
    // int pure_cnt;
    // unordered_map<uint32_t, int> insertedflows;

    void clear_look_up_table() override
    {
        delete[] table;
    }

    void create_array() override
    {
        pure_cnt = 0;
        // id
        id = new int32_t *[array_num];
        for (int i = 0; i < array_num; ++i)
        {
            id[i] = new int32_t[entry_num];
            memset(id[i], 0, entry_num * sizeof(int32_t));
        }
        // fingerprint
        if (use_fing)
        {
            fingerprint = new int32_t *[array_num];
            for (int i = 0; i < array_num; ++i)
            {
                fingerprint[i] = new int32_t[entry_num];
                memset(fingerprint[i], 0, entry_num * sizeof(int32_t));
            }
        }

        // counter
        counter = new int32_t *[array_num];
        for (int i = 0; i < array_num; ++i)
        {
            counter[i] = new int32_t[entry_num];
            memset(counter[i], 0, entry_num * sizeof(int32_t));
        }
    }

    void clear_array() override
    {
        for (int i = 0; i < array_num; ++i)
            delete[] id[i];
        delete[] id;

        if (fingerprint)
        {
            for (int i = 0; i < array_num; ++i)
                delete[] fingerprint[i];
            delete[] fingerprint;
        }

        for (int i = 0; i < array_num; ++i)
            delete[] counter[i];
        delete[] counter;
    }

    Fermat_Count(int _a, int _e, bool _fing, uint32_t _init) : array_num(_a), entry_num(_e), use_fing(_fing), fingerprint(nullptr), hash_fp(nullptr)
    {
        printf("You are running Fermat Count version. Prime for ID: %d\n", PRIME_ID_COUNT);
        create_array();
        // hash
        if (use_fing)
            hash_fp = new BOBHash32(_init);
        hash = new BOBHash32[array_num];
        hash_sign = new BOBHash32[array_num];
        
        for (int i = 0; i < array_num; ++i)
        {
            hash[i].initialize(_init + (10 * i) + 1);
            hash_sign[i].initialize(_init + (17 * i) + 1);
        }
    }

    Fermat_Count(int _memory, bool _fing, uint32_t _init) : use_fing(_fing), fingerprint(nullptr), hash_fp(nullptr)
    {
        printf("You are running Fermat Count version. _memory = %d\n", _memory);
        array_num = 3;
        if (use_fing)
            entry_num = _memory / (array_num * 12);
        else
            entry_num = _memory / (array_num * 8);

        // cout << "construct fermat with " << entry_num << " entry" << endl;
        create_array();

        // hash
        if (use_fing)
            hash_fp = new BOBHash32(_init);
        hash = new BOBHash32[array_num];
        hash_sign = new BOBHash32[array_num];
        for (int i = 0; i < array_num; ++i){

            hash[i].initialize(_init + i + 1);
            hash_sign[i].initialize(_init + (17 * i) + 1);
        }
    }

    ~Fermat_Count() override
    {
        clear_array();
        clear_look_up_table();
        if (hash_fp)
            delete hash_fp;
        delete[] hash;
    }

    int get_sign(int32_t flow_id, int i){
        uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
        uint32_t kk = hash_sign[i].run((char *)&flow_id, sizeof(uint32_t));
        int sign = HASH_TO_SIGN(kk);
        return sign;
    }
    // void Insert(uint32_t flow_id, uint32_t cnt) override {
    //     cout << "You have entered the wrong Insert()!" << endl;
    // }

    void Insert(uint32_t flow_id, uint32_t cnt) override
    {
        // printf("You are running Fermat Count version.\n");        
        int flow_id_ = (int)flow_id;
        insertedflows[flow_id_]+=cnt;
        if (use_fing)
        {
            uint32_t fing = hash_fp->run((char *)&flow_id, sizeof(uint32_t));
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id_, sizeof(uint32_t)) % entry_num;
                uint32_t kk = hash_sign[i].run((char *)&flow_id_, sizeof(uint32_t));
                int sign = HASH_TO_SIGN(kk);
                if(sign > 0){
                    id[i][pos] = ((int64_t)id[i][pos] + (int64_t)mulMod32_Cnt(flow_id_, cnt, PRIME_ID)) % PRIME_ID;
                    fingerprint[i][pos] = ((int64_t)fingerprint[i][pos] + mulMod32_Cnt(fing, cnt, PRIME_FING)) % PRIME_FING;
                    counter[i][pos] += cnt;
                }
                else{
                    id[i][pos] = ((int64_t)id[i][pos] - (int64_t)mulMod32_Cnt(flow_id_, cnt, PRIME_ID)) % PRIME_ID;
                    fingerprint[i][pos] = ((int64_t)fingerprint[i][pos] - mulMod32_Cnt(fing, cnt, PRIME_FING)) % PRIME_FING;
                    counter[i][pos] -= cnt;
                }
                // id[i][pos] = (mulMod32(flow_id, cnt, PRIME_ID) + (uint64_t)id[i][pos]) % PRIME_ID;
                // fingerprint[i][pos] = ((uint64_t)fingerprint[i][pos] + mulMod32(fing, cnt, PRIME_FING)) % PRIME_FING;
                // counter[i][pos] += cnt;
            }
        }
        else
        {
            for (int i = 0; i < array_num; ++i)
            {
                // printf("I'm in %d\n", __LINE__);
                uint32_t pos = hash[i].run((char *)&flow_id_, sizeof(uint32_t)) % entry_num;
                // printf("I'm in %d\n", __LINE__);
                uint32_t kk = hash_sign[i].run((char *)&flow_id_, sizeof(uint32_t));
                // printf("kk = %d\n", kk);
                // printf("I'm in %d\n", __LINE__);
                int sign = HASH_TO_SIGN(kk);
                // printf("Before judging in %d\n", __LINE__);
                // cout << "Inserted flow_id: " << flow_id << " ";
                if(sign > 0){
                    id[i][pos] = ((int64_t)id[i][pos] + (int64_t)mulMod32_Cnt(flow_id_, cnt, PRIME_ID_COUNT)) % (int64_t)PRIME_ID_COUNT;
                    counter[i][pos] += cnt;
                }
                else{
                    // cout << "(int64_t)id[" << i << "][" << pos << "]=" << (int64_t)id[i][pos] << " ";
                    // cout << "mulMod32(...)=" << mulMod32(flow_id, cnt, PRIME_ID_COUNT) << " ";
                    // cout << "diff=" << ((int64_t)id[i][pos] - (int64_t)mulMod32(flow_id, cnt, PRIME_ID_COUNT)) << " ";
                    id[i][pos] = ((int64_t)id[i][pos] - (int64_t)mulMod32_Cnt(flow_id_, cnt, PRIME_ID_COUNT)) % (int64_t)PRIME_ID_COUNT;
                    // cout << "(After module)id[" << i << "][" << pos << "]=" << (int64_t)id[i][pos] << " ";
                    counter[i][pos] -= cnt;
                }
                // cout << "id[" << i << "][" << pos << "]=" << id[i][pos] << " ";
                // cout << "counter[" << i << "][" << pos << "]=" << counter[i][pos] << endl;
                // printf("After judging in %d\n", __LINE__);
                // printf("Current i = %d, array_num = %d\n", i, array_num);
            }
        }
        // printf("Return from Insert in fermat_count\n");
    }

    void Insert_one(uint32_t flow_id) override
    {
        // flow_id should < PRIME_ID
        if (use_fing)
        {
            uint32_t fing = hash_fp->run((char *)&flow_id, sizeof(uint32_t)) % PRIME_FING;
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                id[i][pos] = ((uint64_t)id[i][pos] + (uint64_t)(flow_id % PRIME_ID)) % PRIME_ID;
                fingerprint[i][pos] = ((int64_t)fingerprint[i][pos] + (uint64_t)fing) % PRIME_FING;
                counter[i][pos]++;
            }
        }
        else
        {
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                id[i][pos] = ((uint64_t)id[i][pos] + (uint64_t)(flow_id % PRIME_ID)) % PRIME_ID;
                counter[i][pos]++;
            }
        }
    }

    void Delete_in_one_bucket(int row, int col, int pure_row, int pure_col, int sign = 1) override
    {
        // cout<<"Using Delete_in_one_bucket\n";
        // delete (flow_id, fing, cnt) in bucket (row, col)
        // uint32_t kk = hash_sign[pure_row].run((char *)&flow_id, sizeof(uint32_t));
        // printf("kk = %d\n", kk);
        // printf("I'm in %d\n", __LINE__);
        // int sign = HASH_TO_SIGN(kk);
        // cout << "sign: " << sign <<endl;
        // sign = -1;
        // return;
        // cout << "id[" << row << "][" << col << "]=" << (int64_t)id[row][col] << " (pure)id[" << pure_row << "][" << pure_col << "]=" << (int64_t)id[pure_row][pure_col] << " ";
        id[row][col] = ((int64_t)PRIME_ID_COUNT + (int64_t)id[row][col] - (int64_t)id[pure_row][pure_col]) % PRIME_ID_COUNT;
        // cout << "ID After minus: " << id[row][col] << endl;
        if (use_fing)
            fingerprint[row][col] = ((int64_t)PRIME_FING + (int64_t)fingerprint[row][col] - (int64_t)fingerprint[pure_row][pure_col]) % PRIME_FING;
        // cout << "counter[" << row << "][" << col << "]=" << counter[row][col] << " (pure)counter[" << pure_row << "][" << pure_col << "]=" << counter[pure_row][pure_col] << " ";
        counter[row][col] -= counter[pure_row][pure_col];
        // cout << "CNT After minus: " << counter[row][col] << endl;
    }

    // bool verify(int row, int col, uint32_t &flow_id, uint32_t &fing) override {
    //     cout << "You are in the wrong verify()!" << endl;
    // }
    int verify(int row, int col, uint32_t &flow_id, uint32_t &fing)
    // bool verify(int row, int col, uint32_t &flow_id, uint32_t &fing)
    {
        // cout << "####################I'm in verify of fermat_count!\n";
#if DEBUG_F
        ++pure_cnt;
#endif
        int32_t cnt_value = counter[row][col];
        int32_t id_value = id[row][col];
        // if (cnt_value & 0x80000000)
        // {
        //     cout << "I'm in verify(count) and cnt_value is negative!" << endl;
        //     uint64_t temp = checkTable_count((~cnt_value + 1));
        //     flow_id = mulMod32(PRIME_ID_COUNT - id_value, temp, PRIME_ID_COUNT);
        // }
        // else
        {
            // cout << "I'm in verify(count) and cnt_value is positive!" << endl;
            // cout << "\tid: " << id_value << "\tcnt: " << cnt_value << " ";
            uint64_t temp = checkTable_count(abs(cnt_value));
            // cout << "\tcnt^P: " << temp << " ";
            flow_id = mulMod32_Cnt(id_value, temp, PRIME_ID_COUNT);
            // cout << "\tflow_id*: " << flow_id;
        }
        // flow_id = (id[row][col] * table[counter[row][col] % PRIME_ID]) % PRIME_ID;
        if (use_fing)
        {
            fing = powMod32(cnt_value, PRIME_FING - 2, PRIME_FING);
            fing = mulMod32(fingerprint[row][col], fing, PRIME_FING);
        }
        int mapto = hash[row].run((char *)&flow_id, sizeof(int32_t)) % entry_num;
        // cout << "\tmapto: " << mapto << "\trealcol: " << col << endl;
        if (!(mapto == col))
            return false;
        if (use_fing && !(hash_fp->run((char *)&flow_id, sizeof(int32_t)) % PRIME_FING == fing))
            return false;
        return true;
    }

    void display() override
    {
        cout << " --- display --- " << endl;
        for (int i = 0; i < array_num; ++i)
        {
            for (int j = 0; j < entry_num; ++j)
            {
                if (counter[i][j])
                {
                    cout << i << "," << j << ":" << counter[i][j] << endl;
                }
            }
        }
    }
    int query(const char *key) override
    {
        uint32_t flow_id = *(uint32_t *)key;
        uint32_t ret = 1 << 30;
        if (decodeflag)
        {
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                if(ret > counter[i][pos]) return counter[i][pos];
                else return ret;
                // ret = min(counter[i][pos], ret);
            }
        }
        return (int)ret;
    }
    // int CountMin_query(const char *key)
    // {
    //     uint32_t flow_id = *(uint32_t *)key;
    //     uint32_t ret = 1 << 30;
    //     for (int i = 0; i < array_num; ++i)
    //     {
    //         uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
    //         ret = min((uint32_t)abs(counter[i][pos]), ret);
    //     }
    //     return (int)ret;
    // }

    int undecoded_query(const char *key) override
    {
        // printf("Calculating Medium!\n");
        uint32_t flow_id = *(uint32_t *)key;
        std::vector<int32_t> values;
        std::vector<int32_t> values_from_changed_counters;

        for (int i = 0; i < array_num; ++i)
        {
            uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
            
            uint32_t kk = hash_sign[i].run((char *)&flow_id, sizeof(uint32_t));
            int sign = HASH_TO_SIGN(kk);
            values.push_back(countercpy[i][pos]*sign);
            // values_from_changed_counters.push_back(counter[i][pos]*sign);
            cout << countercpy[i][pos]*sign << " ";
        }
        // cout<<endl;

        cout << "The size of the values in fermat_count: " << values.size() << endl;

        // 找到中位数
        size_t median_index = values.size() / 2;
        std::nth_element(values.begin(), values.begin() + median_index, values.end());
        // std::nth_element(values_from_changed_counters.begin(), values_from_changed_counters.begin() + median_index, values_from_changed_counters.end());
        int32_t median = values[median_index];
        // int32_t median_from = values_from_changed_counters[median_index];

        // if(median != median_from)
        //     cout << "(" << median << ", " << median_from << ") ";

        // 如果数组大小为偶数，则还需要找到下一个元素，取平均值
        if (values.size() % 2 == 0) {
            int32_t next_median = *std::max_element(values.begin(), values.begin() + median_index);
            median = (median + next_median) / 2;
        }

        return (int)median;
    }
    
    // bool Decode(unordered_map<uint32_t, int> &result) override {
    //     cout << "You are in the wrong Decode()!" << endl;
    // }
    // bool Decode(unordered_map<int32_t, int> &result)
    bool Decode(DataVariant& data) override
    {
        auto* mapPtr = std::get_if<std::unordered_map<int32_t, int>>(&data);
        if (!mapPtr) {
            return false;  // 如果类型不匹配，则直接返回 false
        }
        
        auto& result = *mapPtr;

        cout << "The size of the converted Variant map in fermat_count: " << result.size() << endl;
        // for (int i = 0; i < array_num; i++){
        //     for (int j = 0; j < entry_num; j++){
        //         cout << counter[i][j] << " ";
        //     }
        //     cout << endl <<endl;
        // }
        idcpy = new int32_t *[array_num];
        for (int i = 0; i < array_num; i++)
        {
            idcpy[i] = new int32_t[entry_num];
            for (int j = 0; j < entry_num; j++)
                idcpy[i][j] = id[i][j];
        }
        if (use_fing)
        {
            fingcpy = new int32_t *[array_num];
            for (int i = 0; i < array_num; i++)
            {
                fingcpy[i] = new int32_t[entry_num];
                for (int j = 0; j < entry_num; j++)
                    fingcpy[i][j] = fingerprint[i][j];
            }
        }
        countercpy = new int32_t *[array_num];
        for (int i = 0; i < array_num; i++)
        {
            countercpy[i] = new int32_t[entry_num];
            for (int j = 0; j < entry_num; j++)
                countercpy[i][j] = counter[i][j];
        }
        decodeflag = 1;
        queue<int> *candidate = new queue<int>[array_num];
        int32_t flow_id = 0;
        int32_t fing = 0;
        // cout << endl << endl << endl << endl << endl << endl << endl << endl << endl;
        // first round
        for (int i = 0; i < array_num; ++i){
            uint32_t kk = hash_sign[i].run((char *)&flow_id, sizeof(uint32_t));
            int sign = HASH_TO_SIGN(kk);
            for (int j = 0; j < entry_num; ++j)
            {
                uint32_t temp_flow_id = 0;
                uint32_t temp_fin = 0;
                if (counter[i][j] == 0)
                {
                    continue;
                }
                // else if((counter[i][j] <= 0 && id[i][j] >= 0) || (counter[i][j] >= 0 && id[i][j] <= 0)){
                //     cout << "counter[i][j] is " << counter[i][j] << " id[i][j] is " << id[i][j] << endl;
                //     continue;
                // }
                else if (verify(i, j, temp_flow_id, temp_fin))
                {
                    // find pure bucket
                    flow_id = (int32_t)temp_flow_id;
                    fing = (int32_t)temp_fin;
                    if (result.count(flow_id) != 0)
                    {
                        result[flow_id] += abs(counter[i][j]);
                        //if(counter[i][j] != abs(counter[i][j])) cout<<counter[i][j]<<" "<<abs(counter[i][j])<<" ";
                    }
                    else
                    {
                        result[flow_id] = abs(counter[i][j]);
                    }
                    // delete flow from other rows
                    
                    for (int t = 0; t < array_num; ++t)
                    {
                        if (t == i)
                            continue;
                        uint32_t pos = hash[t].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                        
                        Delete_in_one_bucket(t, pos, i, j, sign);
                        candidate[t].push(pos);
                    }
                    Delete_in_one_bucket(i, j, i, j, sign);
                }
            }
        }
        cout << endl << endl << endl << endl << endl << endl << endl << endl << endl;

        bool pause;
        int acc = 0;
        while (true)
        {
            acc++;
            pause = true;
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t kk = hash_sign[i].run((char *)&flow_id, sizeof(uint32_t));
                int sign = HASH_TO_SIGN(kk);
                if (!candidate[i].empty())
                    pause = false;
                while (!candidate[i].empty())
                {
                    int check = candidate[i].front();
                    candidate[i].pop();
                    uint32_t temp_flow_id = 0;
                    uint32_t temp_fin = 0;
                    // cout << i << " " << check << endl;
                    if (counter[i][check] == 0)
                    {
                        continue;
                    }
                    else if (verify(i, check, temp_flow_id, temp_fin))
                    {
                        // find pure bucket
                        flow_id = (int32_t)temp_flow_id;
                        fing = (int32_t)temp_fin;
                        if (result.count(flow_id) != 0)
                        {
                            result[flow_id] += abs(counter[i][check]);
                            // cout<<"I'm such a dick!\n";
                        }
                        else
                        {
                            result[flow_id] = abs(counter[i][check]);
                            // if(counter[i][check] != abs(counter[i][check])) cout<<counter[i][check]<<" "<<abs(counter[i][check])<<" ";
                            // cout<<"I'm such an asshole!\n";
                        }
                        // delete flow from other rows
                        for (int t = 0; t < array_num; ++t)
                        {
                            if (t == i)
                                continue;
                            uint32_t pos = hash[t].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                            Delete_in_one_bucket(t, pos, i, check, sign);
                            candidate[t].push(pos);
                        }
                        Delete_in_one_bucket(i, check, i, check, sign);
                    }
                }
            }
            if (pause){
                printf("Break because pauce! acc = %d.\n", acc);
                break;
            }
            if (acc > 100000){
                printf("Break because acc is too big!\n");
                break;
            }
        }

        delete[] candidate;
        bool flag = true;
        for (int i = 0; i < array_num; ++i)
            for (int j = 0; j < entry_num; ++j)
                if (counter[i][j] != 0)
                {
                    // cout << "undecoded  i " << i << " j " << j << endl;
                    // cout << counter[i][j] << endl;
                    flag = false;
                }
        for (auto p : result)
        {
            if (p.second == 0)
            {
                result.erase(p.first);
            }
        }
        return flag;
    }
    int get_id(int n_array, int n){
        if(n_array >=0 && n_array <= array_num && n >= 0 && n <= entry_num){
            return id[n_array][n];
        }
        else{
            cout << "get_id() out of range!" << endl;
            assert(0);
        }
    }
    int get_counter(int n_array, int n){
        if(n_array >=0 && n_array <= array_num && n >= 0 && n <= entry_num){
            return counter[n_array][n];
        }
        else{
            cout << "get_counter() out of range!" << endl;
            assert(0);
        }
    }

    int set_id(int n_array, int n, int value){
        if(n_array >=0 && n_array <= array_num && n >= 0 && n <= entry_num){
            id[n_array][n] = value;
            return 1;
        }
        else{
            cout << "set_id() out of range!" << endl;
            assert(0);
            return 0;
        }
    }
    int set_counter(int n_array, int n, int value){
        if(n_array >=0 && n_array <= array_num && n >= 0 && n <= entry_num){
            counter[n_array][n] = value;
            return 1;
        }
        else{
            cout << "set_counter() out of range!" << endl;
            assert(0);
            return 0;
        }
    }
    int get_array_num() override{
        return array_num;
    }
    int get_entry_num() override{
        return entry_num;
    }
};

//ID uses + only and cnt uses + and -.
class Fermat_Count_IDP_CNTPM : public Fermat
{
    int array_num;
    int entry_num;
    int decodeflag = 0;
    // hash
    BOBHash32 *hash;
    BOBHash32 *hash_fp;

    uint32_t *table;

    bool use_fing;
    // arrays
    // int array_num;
    // int entry_num;
    int32_t **fingerprint;
    uint32_t **idcpy;
    int32_t **countercpy;
    int32_t **fingcpy;
    // int decodeflag = 0;
    // // hash
    // BOBHash32 *hash;
    // BOBHash32 *hash_fp;
    BOBHash32 *hash_sign;

    // uint32_t *table;

    // bool use_fing;

public:
    uint32_t **id;
    int32_t **counter;

    int get_array_num() override{
        return array_num;
    }
    int get_entry_num() override{
        return entry_num;
    }

    void clear_look_up_table() override
    {
        delete[] table;
    }

    void create_array() override
    {
        pure_cnt = 0;
        // id
        id = new uint32_t *[array_num];
        for (int i = 0; i < array_num; ++i)
        {
            id[i] = new uint32_t[entry_num];
            memset(id[i], 0, entry_num * sizeof(uint32_t));
        }
        // fingerprint
        if (use_fing)
        {
            fingerprint = new int32_t *[array_num];
            for (int i = 0; i < array_num; ++i)
            {
                fingerprint[i] = new int32_t[entry_num];
                memset(fingerprint[i], 0, entry_num * sizeof(int32_t));
            }
        }

        // counter
        counter = new int32_t *[array_num];
        for (int i = 0; i < array_num; ++i)
        {
            counter[i] = new int32_t[entry_num];
            memset(counter[i], 0, entry_num * sizeof(int32_t));
        }
    }

    void clear_array() override
    {
        for (int i = 0; i < array_num; ++i)
            delete[] id[i];
        delete[] id;

        if (fingerprint)
        {
            for (int i = 0; i < array_num; ++i)
                delete[] fingerprint[i];
            delete[] fingerprint;
        }

        for (int i = 0; i < array_num; ++i)
            delete[] counter[i];
        delete[] counter;
    }

    Fermat_Count_IDP_CNTPM(int _a, int _e, bool _fing, uint32_t _init) : array_num(_a), entry_num(_e), use_fing(_fing), fingerprint(nullptr), hash_fp(nullptr)
    {
        // cout << "You are running Fermat_Count_IDP_CNTPM version. Prime for ID: " << PRIME_ID << ", array_num = " << array_num << endl;
        create_array();
        // hash
        if (use_fing)
            hash_fp = new BOBHash32(_init);
        hash = new BOBHash32[array_num];
        hash_sign = new BOBHash32[array_num];
        
        for (int i = 0; i < array_num; ++i)
        {
            hash[i].initialize(_init + (10 * i) + 1);
            hash_sign[i].initialize(_init + (17 * i) + 1);
        }
    }

    Fermat_Count_IDP_CNTPM(int _memory, bool _fing, uint32_t _init) : use_fing(_fing), fingerprint(nullptr), hash_fp(nullptr)
    {
        // printf("You are running Fermat_Count_IDP_CNTPM version. _memory = %d\n", _memory);
        array_num = 3;
        if (use_fing)
            entry_num = _memory / (array_num * 12);
        else
            entry_num = _memory / (array_num * 8);

        // cout << "construct fermat with " << entry_num << " entry" << endl;
        create_array();

        // hash
        if (use_fing)
            hash_fp = new BOBHash32(_init);
        hash = new BOBHash32[array_num];
        hash_sign = new BOBHash32[array_num];
        for (int i = 0; i < array_num; ++i){

            hash[i].initialize(_init + i + 1);
            hash_sign[i].initialize(_init + (17 * i) + 1);
        }
    }

    ~Fermat_Count_IDP_CNTPM() override
    {
        clear_array();
        clear_look_up_table();
        if (hash_fp)
            delete hash_fp;
        delete[] hash;
    }

    int get_sign(int32_t flow_id, int i){
        uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
        uint32_t kk = hash_sign[i].run((char *)&flow_id, sizeof(uint32_t));
        int sign = HASH_TO_SIGN(kk);
        return sign;
    }
    // void Insert(uint32_t flow_id, uint32_t cnt) override {
    //     cout << "You have entered the wrong Insert()!" << endl;
    // }

    void Insert(uint32_t flow_id, uint32_t cnt) override {
        // printf("You are running Fermat Count version.\n");        
        uint32_t flow_id_ = flow_id;
        int cnt_sign = (int)cnt>0?1:-1;
        uint32_t abscnt = abs((int)(cnt));
        // int flow_id_ = (int)flow_id;
        insertedflows[flow_id_]+=cnt;
        // uint32_t flow_id_ = flow_id;
        if (use_fing) {
            uint32_t fing = hash_fp->run((char *)&flow_id, sizeof(uint32_t));
            for (int i = 0; i < array_num; ++i) {
                uint32_t pos = hash[i].run((char *)&flow_id_, sizeof(uint32_t)) % entry_num;
                uint32_t kk = hash_sign[i].run((char *)&flow_id_, sizeof(uint32_t));
                int sign = HASH_TO_SIGN(kk);
                id[i][pos] = ((uint64_t)id[i][pos] + (uint64_t)mulMod32(flow_id_, cnt, PRIME_ID)) % PRIME_ID;
                if(sign > 0){
                    fingerprint[i][pos] = ((int64_t)fingerprint[i][pos] + mulMod32(fing, cnt, PRIME_FING)) % PRIME_FING;
                    counter[i][pos] += cnt;
                }
                else{
                    // id[i][pos] = ((int64_t)id[i][pos] - (int64_t)mulMod32_Cnt(flow_id_, cnt, PRIME_ID)) % PRIME_ID;
                    fingerprint[i][pos] = ((int64_t)fingerprint[i][pos] - mulMod32(fing, cnt, PRIME_FING)) % PRIME_FING;
                    counter[i][pos] -= cnt;
                }
                // id[i][pos] = (mulMod32(flow_id, cnt, PRIME_ID) + (uint64_t)id[i][pos]) % PRIME_ID;
                // fingerprint[i][pos] = ((uint64_t)fingerprint[i][pos] + mulMod32(fing, cnt, PRIME_FING)) % PRIME_FING;
                // counter[i][pos] += cnt;
            }
        }
        else
        {
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id_, sizeof(uint32_t)) % entry_num;
                uint32_t kk = hash_sign[i].run((char *)&flow_id_, sizeof(uint32_t));
                int sign = HASH_TO_SIGN(kk);
                if(cnt_sign > 0){
                    id[i][pos] = ((uint64_t)id[i][pos] + (uint64_t)mulMod32(flow_id_, abscnt, PRIME_ID_IDP_CNTPM)) % (uint64_t)PRIME_ID_IDP_CNTPM;
                }
                else{
                    id[i][pos] = ((uint64_t)PRIME_ID_IDP_CNTPM + (uint64_t)id[i][pos] - (uint64_t)mulMod32_Cnt(flow_id_, abscnt, PRIME_ID_IDP_CNTPM)) % (uint64_t)PRIME_ID_IDP_CNTPM;
                }
                if(sign > 0){
                    counter[i][pos] += cnt;
                }
                else{
                    // cout << "(int64_t)id[" << i << "][" << pos << "]=" << (int64_t)id[i][pos] << " ";
                    // cout << "mulMod32(...)=" << mulMod32(flow_id, cnt, PRIME_ID_COUNT) << " ";
                    // cout << "diff=" << ((int64_t)id[i][pos] - (int64_t)mulMod32(flow_id, cnt, PRIME_ID_COUNT)) << " ";
                    // id[i][pos] = ((int64_t)id[i][pos] - (int64_t)mulMod32_Cnt(flow_id_, cnt, PRIME_ID_COUNT)) % (int64_t)PRIME_ID_COUNT;
                    // cout << "(After module)id[" << i << "][" << pos << "]=" << (int64_t)id[i][pos] << " ";
                    counter[i][pos] -= cnt;
                }
            }
        }
        // printf("Return from Insert in fermat_count\n");
    }

    void Insert_one(uint32_t flow_id) override
    {
        // flow_id should < PRIME_ID
        if (use_fing)
        {
            uint32_t fing = hash_fp->run((char *)&flow_id, sizeof(uint32_t)) % PRIME_FING;
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                id[i][pos] = ((uint64_t)id[i][pos] + (uint64_t)(flow_id % PRIME_ID)) % PRIME_ID;
                fingerprint[i][pos] = ((int64_t)fingerprint[i][pos] + (uint64_t)fing) % PRIME_FING;
                counter[i][pos]++;
            }
        }
        else
        {
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                id[i][pos] = ((uint64_t)id[i][pos] + (uint64_t)(flow_id % PRIME_ID)) % PRIME_ID;
                counter[i][pos]++;
            }
        }
    }

    void Delete_in_one_bucket(int row, int col, int pure_row, int pure_col, int sign = 1) override
    {
        // cout<<"Using Delete_in_one_bucket\n";
        // delete (flow_id, fing, cnt) in bucket (row, col)
        // uint32_t kk = hash_sign[pure_row].run((char *)&flow_id, sizeof(uint32_t));
        // printf("kk = %d\n", kk);
        // printf("I'm in %d\n", __LINE__);
        // int sign = HASH_TO_SIGN(kk);
        // cout << "sign: " << sign <<endl;
        // sign = -1;
        // return;
        // cout << "id[" << row << "][" << col << "]=" << (int64_t)id[row][col] << " (pure)id[" << pure_row << "][" << pure_col << "]=" << (int64_t)id[pure_row][pure_col] << " ";
        id[row][col] = ((uint64_t)PRIME_ID_IDP_CNTPM + (uint64_t)id[row][col] - (uint64_t)id[pure_row][pure_col]) % PRIME_ID_IDP_CNTPM;
        // cout << "ID After minus: " << id[row][col] << endl;
        if (use_fing)
            fingerprint[row][col] = ((int64_t)PRIME_FING + (int64_t)fingerprint[row][col] - (int64_t)fingerprint[pure_row][pure_col]) % PRIME_FING;
        // cout << "counter[" << row << "][" << col << "]=" << counter[row][col] << " (pure)counter[" << pure_row << "][" << pure_col << "]=" << counter[pure_row][pure_col] << " ";
        counter[row][col] -= counter[pure_row][pure_col];
        // cout << "CNT After minus: " << counter[row][col] << endl;
    }

    // bool verify(int row, int col, uint32_t &flow_id, uint32_t &fing) override {
    //     cout << "You are in the wrong verify()!" << endl;
    // }
    int verify(int row, int col, uint32_t &flow_id, uint32_t &fing)
    // bool verify(int row, int col, uint32_t &flow_id, uint32_t &fing)
    {
        
#if DEBUG_F
        ++pure_cnt;
#endif
        uint32_t checked_id = 3458834590;
        int32_t cnt_value = counter[row][col];
        uint32_t id_value = id[row][col];
        uint64_t temp = 0;
        if (cnt_value & 0x80000000){
            temp = checkTable_count((~cnt_value + 1), PRIME_ID_IDP_CNTPM);
            // flow_id = mulMod32(PRIME_ID_COUNT - id_value, temp, PRIME_ID_COUNT);
            // flow_id = mulMod32(PRIME_ID_IDP_CNTPM - id_value, temp, PRIME_ID_IDP_CNTPM);
            flow_id = mulMod32(id_value, temp, PRIME_ID_IDP_CNTPM);
            if(id_value == 3393094456){
                cout << "temp: "<< temp << ", flow_id decoded: " << flow_id << ", row:" << row << ", col:" << col << ", cnt:" << cnt_value << endl;
            }
        }
        else{
            temp = checkTable(abs(cnt_value));
            flow_id = mulMod32(id_value, temp, PRIME_ID_IDP_CNTPM);
        }

        
        if (use_fing)
        {
            fing = powMod32(cnt_value, PRIME_FING - 2, PRIME_FING);
            fing = mulMod32(fingerprint[row][col], fing, PRIME_FING);
        }
        int mapto = hash[row].run((char *)&flow_id, sizeof(int32_t)) % entry_num;
        if (!(mapto == col)){
            // cout << "mapto is wrong!" << endl;
            if(id_value == checked_id){
                cout << "(id_value)False! id_value is " << id_value <<"! temp: "<< temp << ", flow_id decoded: " << flow_id << ", mapto is " << mapto << ", row:" << row << ", col:" << col << ", cnt:" << cnt_value << endl;
            }
            // return false;
            flow_id = mulMod32(PRIME_ID_IDP_CNTPM - id_value, temp, PRIME_ID_IDP_CNTPM);
            mapto = hash[row].run((char *)&flow_id, sizeof(int32_t)) % entry_num;
            if (!(mapto == col)){
                // cout << "mapto is wrong!" << endl;
                if(id_value == checked_id){
                    cout << "(P-id_value)False! id_value is " << id_value <<"! temp: "<< temp << ", flow_id decoded: " << flow_id << ", mapto is " << mapto << ", row:" << row << ", col:" << col << ", cnt:" << cnt_value << endl;
                }
                return false;
            }
            else{
                if(id_value == checked_id || flow_id % checked_id == 0){
                    cout << "(P-id_value)True! id_value is " << id_value << "!" << ", flow_id decoded: " << flow_id << ", mapto is " << mapto << ", row:" << row << ", col:" << col << ", cnt:" << cnt_value << endl;
                }
                return 2;
            }

        }
        if (use_fing && !(hash_fp->run((char *)&flow_id, sizeof(int32_t)) % PRIME_FING == fing)){
            cout << "fing is wrong!" << endl;
            return false;
        }
        if(id_value == checked_id || flow_id % checked_id == 0){
            cout << "(id_value)True! id_value is " << id_value << "!" << ", flow_id decoded: " << flow_id << ", mapto is " << mapto << ", row:" << row << ", col:" << col << ", cnt:" << cnt_value << endl;
        }
        return 1;
    }

    void display() override
    {
        cout << " --- display --- " << endl;
        for (int i = 0; i < array_num; ++i)
        {
            for (int j = 0; j < entry_num; ++j)
            {
                if (counter[i][j])
                {
                    cout << i << "," << j << ":" << counter[i][j] << endl;
                }
            }
        }
    }
    
    int query(const char *key) override
    {
        // cout << "You are in the query()!" << endl;
        uint32_t flow_id = *(uint32_t *)key;
        return abs(counter[0][hash[0].run((char *)&flow_id, sizeof(uint32_t)) % entry_num]);
        uint32_t ret = 1 << 30;
        // cout << "docodeflag = " << decodeflag << endl;
        if (decodeflag)
        {
            for (int i = 0; i < array_num; ++i)
            {
                uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                if(ret > counter[i][pos]) return counter[i][pos];
                else return ret;
                // ret = min(counter[i][pos], ret);
            }
        }
        return (int)ret;
    }

    int query_array(const char *key, int array_index) override
    {
        uint32_t flow_id = *(uint32_t *)key;
        return abs(counter[array_index][hash[array_index].run((char *)&flow_id, sizeof(uint32_t)) % entry_num]);
        uint32_t ret = 1 << 30;
        if (decodeflag)
        {
            uint32_t pos = hash[array_index].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
            return counter[array_index][pos];
        }
        return (int)ret;
    }

    // int query_from_cpy(const char *key) override
    // {
    //     uint32_t flow_id = *(uint32_t *)key;
    //     uint32_t ret = 1 << 30;
    //     if (decodeflag)
    //     {
    //         for (int i = 0; i < array_num; ++i)
    //         {
    //             uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
    //             if(ret > countercpy[i][pos]) return countercpy[i][pos];
    //             else return ret;
    //             // ret = min(counter[i][pos], ret);
    //         }
    //     }
    //     return (int)ret;
    // }
    // int CountMin_query(const char *key)
    // {
    //     uint32_t flow_id = *(uint32_t *)key;
    //     uint32_t ret = 1 << 30;
    //     for (int i = 0; i < array_num; ++i)
    //     {
    //         uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
    //         ret = min((uint32_t)abs(counter[i][pos]), ret);
    //     }
    //     return (int)ret;
    // }

    int undecoded_query(const char *key) override
    {
        // printf("Calculating Medium!\n");
        //if countercpy is not defined
        if (countercpy == NULL)
        {
            cout << "countercpy is not defined!" << endl;
            if(cpy_counters()){
                cout << "countercpy defined!" << endl;
            }
            else{
                cout << "countercpy define failed!" << endl;
                return -1;
            }
        }
        uint32_t flow_id = *(uint32_t *)key;
        std::vector<int32_t> values;
        std::vector<int32_t> values_from_changed_counters;

        for (int i = 0; i < array_num; ++i)
        {
            uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
            
            uint32_t kk = hash_sign[i].run((char *)&flow_id, sizeof(uint32_t));
            int sign = HASH_TO_SIGN(kk);
            values.push_back(countercpy[i][pos]*sign);
            values_from_changed_counters.push_back(counter[i][pos]*sign);
            // cout << counter[i][pos]*sign << " ";
        }
        // cout<<endl;


        // 找到中位数
        size_t median_index = values.size() / 2;
        std::nth_element(values.begin(), values.begin() + median_index, values.end());
        std::nth_element(values_from_changed_counters.begin(), values_from_changed_counters.begin() + median_index, values_from_changed_counters.end());
        int32_t median = values[median_index];
        int32_t median_from = values_from_changed_counters[median_index];

        // if(median != median_from)
        //     cout << "(" << median << ", " << median_from << ") ";

        // 如果数组大小为偶数，则还需要找到下一个元素，取平均值
        if (values.size() % 2 == 0) {
            int32_t next_median = *std::max_element(values.begin(), values.begin() + median_index);
            median = (median + next_median) / 2;
        }

        return (int)median;
    }
    int undecoded_query_before_decoding(const char *key) override
    {
        // printf("Calculating Medium in Fermat_Count_IDP_CNTPM!\n");
        uint32_t flow_id = *(uint32_t *)key;
        std::vector<int32_t> values;

        for (int i = 0; i < array_num; ++i)
        {
            uint32_t pos = hash[i].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
            
            uint32_t kk = hash_sign[i].run((char *)&flow_id, sizeof(uint32_t));
            int sign = HASH_TO_SIGN(kk);
            values.push_back(counter[i][pos]*sign);
            // values_from_changed_counters.push_back(counter[i][pos]*sign);
            cout << counter[i][pos]*sign << " ";
        }

        // cout << "The size of the values in fermat_count: " << values.size() << endl;

        // 找到中位数
        size_t median_index = values.size() / 2;
        std::nth_element(values.begin(), values.begin() + median_index, values.end());
        // std::nth_element(values_from_changed_counters.begin(), values_from_changed_counters.begin() + median_index, values_from_changed_counters.end());
        int32_t median = values[median_index];
        // int32_t median_from = values_from_changed_counters[median_index];

        // if(median != median_from)
        //     cout << "(" << median << ", " << median_from << ") ";

        // 如果数组大小为偶数，则还需要找到下一个元素，取平均值
        if (values.size() % 2 == 0) {
            int32_t next_median = *std::max_element(values.begin(), values.begin() + median_index);
            median = (median + next_median) / 2;
        }

        return (int)median;
    }
    // bool Decode(unordered_map<uint32_t, int> &result) override {
    //     cout << "You are in the wrong Decode()!" << endl;
    // }
    // bool Decode(unordered_map<int32_t, int> &result)
    bool cpy_counters_to_pos(int32_t ***counterdst){
        *counterdst = new int32_t *[array_num];
        for (int i = 0; i < array_num; i++)
        {
            (*counterdst)[i] = new int32_t[entry_num];
            for (int j = 0; j < entry_num; j++)
                (*counterdst)[i][j] = abs(counter[i][j]);
        }
        return true;
    }
    bool cpy_counters(){
        idcpy = new uint32_t *[array_num];
        for (int i = 0; i < array_num; i++)
        {
            idcpy[i] = new uint32_t[entry_num];
            for (int j = 0; j < entry_num; j++)
                idcpy[i][j] = id[i][j];
        }
        if (use_fing)
        {
            fingcpy = new int32_t *[array_num];
            for (int i = 0; i < array_num; i++)
            {
                fingcpy[i] = new int32_t[entry_num];
                for (int j = 0; j < entry_num; j++)
                    fingcpy[i][j] = fingerprint[i][j];
            }
        }
        countercpy = new int32_t *[array_num];
        for (int i = 0; i < array_num; i++)
        {
            countercpy[i] = new int32_t[entry_num];
            for (int j = 0; j < entry_num; j++)
                countercpy[i][j] = counter[i][j];
        }
        return true;
    }
    int get_sign(int array_index, char* flow_id, int size = sizeof(uint32_t)){
        uint32_t kk = hash_sign[array_index].run(flow_id, size);
        int sign = HASH_TO_SIGN(kk);
        return sign;
    }
    bool Decode(DataVariant& data) override
    {
        uint32_t checked_id = 3458834590;
        auto* mapPtr = std::get_if<std::unordered_map<int32_t, int>>(&data);
        if (!mapPtr) {
            return false;  // 如果类型不匹配，则直接返回 false
        }
        
        auto& result = *mapPtr;

        // cout << "The size of the converted Variant map in fermat_count: " << result.size() << endl;
        // for (int i = 0; i < array_num; i++){
        //     for (int j = 0; j < entry_num; j++){
        //         cout << counter[i][j] << " ";
        //     }
        //     cout << endl <<endl;
        // }
        idcpy = new uint32_t *[array_num];
        for (int i = 0; i < array_num; i++)
        {
            idcpy[i] = new uint32_t[entry_num];
            for (int j = 0; j < entry_num; j++)
                idcpy[i][j] = id[i][j];
        }
        if (use_fing)
        {
            fingcpy = new int32_t *[array_num];
            for (int i = 0; i < array_num; i++)
            {
                fingcpy[i] = new int32_t[entry_num];
                for (int j = 0; j < entry_num; j++)
                    fingcpy[i][j] = fingerprint[i][j];
            }
        }
        countercpy = new int32_t *[array_num];
        for (int i = 0; i < array_num; i++)
        {
            countercpy[i] = new int32_t[entry_num];
            for (int j = 0; j < entry_num; j++)
                countercpy[i][j] = counter[i][j];
        }
        decodeflag = 1;
        queue<int> *candidate = new queue<int>[array_num];
        int32_t flow_id = 0;
        int32_t fing = 0;
        // first round
        // std::ofstream outFile("decode_map_track.csv");
        // outFile << "array,entry,mapto,rootcol" << endl;
        for (int i = 0; i < array_num; ++i){
            // uint32_t kk = hash_sign[i].run((char *)&flow_id, sizeof(uint32_t));
            // int sign = HASH_TO_SIGN(kk);
            int sign = 0;
            bool sign_cnt_fetch_pos = 0;
            for (int j = 0; j < entry_num; ++j)
            {
                uint32_t temp_flow_id = 0;
                uint32_t temp_fin = 0;
                // int32_t cnt_value = counter[row][col];
                // uint32_t id_value = id[row][col];
                // if (cnt_value & 0x80000000)
                // {
                //     // cout << "I'm in verify(count) and cnt_value is negative!" << endl;
                //     uint64_t temp = checkTable_count((~cnt_value + 1));
                //     flow_id = mulMod32(PRIME_ID_COUNT - id_value, temp, PRIME_ID_COUNT);
                // }
                // else
                // {
                //     // cout << "I'm in verify(count) and cnt_value is positive!" << endl;
                //     // cout << "\tid: " << id_value << "\tcnt: " << cnt_value << " ";
                //     uint64_t temp = checkTable(abs(cnt_value));
                //     // cout << "\tcnt^P: " << temp << " ";
                //     flow_id = mulMod32(id_value, temp, PRIME_ID);
                //     // cout << "\tflow_id*: " << flow_id;
                // }
                // // flow_id = (id[row][col] * table[counter[row][col] % PRIME_ID]) % PRIME_ID;
                // if (use_fing)
                // {
                //     fing = powMod32(cnt_value, PRIME_FING - 2, PRIME_FING);
                //     fing = mulMod32(fingerprint[row][col], fing, PRIME_FING);
                // }
                // int mapto = hash[row].run((char *)&flow_id, sizeof(int32_t)) % entry_num;
                // outFile << i << "," << j << "," << mapto << "," << col << endl;
                
                if (counter[i][j] == 0)
                {
                    continue;
                }
                // else if((counter[i][j] <= 0 && id[i][j] >= 0) || (counter[i][j] >= 0 && id[i][j] <= 0)){
                //     cout << "counter[i][j] is " << counter[i][j] << " id[i][j] is " << id[i][j] << endl;
                //     continue;
                // }
                else if (verify(i, j, temp_flow_id, temp_fin) == 1)
                {
                    // cout << "Find a pure bucket!" << "i: " << i << " j: " << j << endl;
                    // find pure bucket
                    sign = get_sign(i, (char *)&temp_flow_id, sizeof(uint32_t));
                    if(id[i][j] == checked_id){
                        cout << "First(1)Find a pure bucket!" << "i: " << i << " j: " << j << " counter[i][j] = " << counter[i][j] << " id[i][j] = " << id[i][j] << " sign = " << sign << endl;
                    }
                    flow_id = (int32_t)temp_flow_id;
                    fing = (int32_t)temp_fin;
                    if (result.count(flow_id) != 0)
                    {
                        result[flow_id] += abs(counter[i][j]);
                        //if(counter[i][j] != abs(counter[i][j])) cout<<counter[i][j]<<" "<<abs(counter[i][j])<<" ";
                    }
                    else
                    {
                        result[flow_id] = abs(counter[i][j]);
                    }
                    if(id[i][j] == checked_id){
                        cout << "result[" << (uint32_t)flow_id << "] = " << result[flow_id] << endl;
                    }
                    // delete flow from other rows
                    
                    for (int t = 0; t < array_num; ++t)
                    {
                        if (t == i)
                            continue;
                        uint32_t pos = hash[t].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                        
                        Delete_in_one_bucket(t, pos, i, j, sign);
                        candidate[t].push(pos);
                    }
                    Delete_in_one_bucket(i, j, i, j, sign);
                }
                else if (verify(i, j, temp_flow_id, temp_fin) == 2){
                    // cout << "Not 0 but also not a pure bucket!" << "i: " << i << " j: " << j << endl;
                    sign = get_sign(i, (char *)&temp_flow_id, sizeof(uint32_t));
                    if(id[i][j] == checked_id){
                        cout << "First(2)Find a pure bucket!" << "i: " << i << " j: " << j << " counter[i][j] = " << counter[i][j] << " id[i][j] = " << id[i][j] << " sign = " << sign << endl;
                    }
                    flow_id = (int32_t)temp_flow_id;
                    fing = (int32_t)temp_fin;
                    if (result.count(flow_id) != 0)
                    {
                        // result[flow_id] += abs(counter[i][j]);
                        result[flow_id] += sign * counter[i][j];
                        //if(counter[i][j] != abs(counter[i][j])) cout<<counter[i][j]<<" "<<abs(counter[i][j])<<" ";
                    }
                    else
                    {
                        result[flow_id] = sign * counter[i][j];
                    }
                    // delete flow from other rows
                    
                    for (int t = 0; t < array_num; ++t)
                    {
                        if (t == i)
                            continue;
                        uint32_t pos = hash[t].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                        
                        Delete_in_one_bucket(t, pos, i, j, sign);
                        candidate[t].push(pos);
                    }
                    Delete_in_one_bucket(i, j, i, j, sign);
                }                
            }
        }
        // outFile.close();

        bool pause;
        int acc = 0;
        while (true)
        {
            acc++;
            pause = true;
            for (int i = 0; i < array_num; ++i)
            {
                int sign = get_sign(i, (char *)&flow_id, sizeof(uint32_t));
                if (!candidate[i].empty())
                    pause = false;
                while (!candidate[i].empty())
                {
                    int check = candidate[i].front();
                    candidate[i].pop();
                    uint32_t temp_flow_id = 0;
                    uint32_t temp_fin = 0;
                    if (counter[i][check] == 0)
                    {
                        continue;
                    }
                    else if (verify(i, check, temp_flow_id, temp_fin) == 1)
                    {
                        sign = get_sign(i, (char *)&temp_flow_id, sizeof(uint32_t));
                        if(id[i][check] == checked_id){
                            cout << "(1)Find a pure bucket!" << "i: " << i << " check: " << check << " counter[i][check] = " << counter[i][check] << " id[i][check] = " << " sign = " << sign << endl;
                        }
                        // find pure bucket
                        flow_id = (int32_t)temp_flow_id;
                        fing = (int32_t)temp_fin;
                        if (result.count(flow_id) != 0)
                        {
                            result[flow_id] += abs(counter[i][check]);
                            
                        }
                        else
                        {
                            result[flow_id] = abs(counter[i][check]);
                            if(counter[i][check] != abs(counter[i][check])) cout<<counter[i][check]<<" "<<abs(counter[i][check])<<" ";
                            
                        }
                        // delete flow from other rows
                        for (int t = 0; t < array_num; ++t)
                        {
                            if (t == i)
                                continue;
                            uint32_t pos = hash[t].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                            Delete_in_one_bucket(t, pos, i, check, sign);
                            candidate[t].push(pos);
                        }
                        Delete_in_one_bucket(i, check, i, check, sign);
                    }
                    else if (verify(i, check, temp_flow_id, temp_fin) == 2)
                    {
                        sign = get_sign(i, (char *)&temp_flow_id, sizeof(uint32_t));
                        // find pure bucket
                        if(id[i][check] == checked_id){
                            cout << "(2)Find a pure bucket!" << "i: " << i << " check: " << check << " counter[i][j] = " << counter[i][check] << " id[i][check] = " << " sign = " << sign << endl;
                        }
                        flow_id = (int32_t)temp_flow_id;
                        fing = (int32_t)temp_fin;
                        if (result.count(flow_id) != 0)
                        {
                            result[flow_id] += sign * counter[i][check];
                            
                        }
                        else
                        {
                            result[flow_id] = sign * counter[i][check];
                            // if(counter[i][check] != abs(counter[i][check])) cout<<counter[i][check]<<" "<<abs(counter[i][check])<<" ";
                            
                        }
                        // delete flow from other rows
                        for (int t = 0; t < array_num; ++t)
                        {
                            if (t == i)
                                continue;
                            uint32_t pos = hash[t].run((char *)&flow_id, sizeof(uint32_t)) % entry_num;
                            Delete_in_one_bucket(t, pos, i, check, sign);
                            candidate[t].push(pos);
                        }
                        Delete_in_one_bucket(i, check, i, check, sign);
                    }
                }
            }
            if (pause){
                printf("Break because pauce! acc = %d.\n", acc);
                break;
            }
            if (acc > 100000){
                printf("Break because acc is too big!\n");
                break;
            }
        }

        delete[] candidate;
        bool flag = true;
        for (int i = 0; i < array_num; ++i)
            for (int j = 0; j < entry_num; ++j)
                if (counter[i][j] != 0)
                {
                    // cout << "undecoded  i " << i << " j " << j << ": " << counter[i][j] << endl;
                    flag = false;
                }
        for (auto p : result)
        {
            if (p.second == 0)
            {
                result.erase(p.first);
            }
        }
        return flag;
    }
    int get_id(int n_array, int n){
        if(n_array >=0 && n_array <= array_num && n >= 0 && n <= entry_num){
            return id[n_array][n];
        }
        else{
            cout << "get_id() out of range!" << endl;
            assert(0);
        }
    }
    int get_counter(int n_array, int n){
        if(n_array >=0 && n_array <= array_num && n >= 0 && n <= entry_num){
            return counter[n_array][n];
        }
        else{
            cout << "get_counter() out of range!" << endl;
            assert(0);
        }
    }

    int set_id(int n_array, int n, int value){
        if(n_array >=0 && n_array <= array_num && n >= 0 && n <= entry_num){
            id[n_array][n] = (uint32_t)value;
            return 1;
        }
        else{
            cout << "Hahahaha, set_id() out of range!" << endl;
            cout << "n_array: " << n_array << " n: " << n << " value: " << value << endl;
            assert(0);
            return 0;
        }
    }

    int set_counter(int n_array, int n, int value){
        if(n_array >=0 && n_array <= array_num && n >= 0 && n <= entry_num){
            counter[n_array][n] = value;
            return 1;
        }
        else{
            cout << "set_counter() out of range!" << endl;
            assert(0);
            return 0;
        }
    }
};

#endif