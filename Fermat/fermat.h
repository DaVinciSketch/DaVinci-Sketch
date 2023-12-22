#ifndef _FERMAT_H_
#define _FERMAT_H_

#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <queue>
#include <cstring>
#include "../common/BOBHash32.h"
#include "util/mod.h"
#include "util/prime.h"
#include "murmur3.h"
#include <vector>
#include <algorithm>

using namespace std;

#define HASH_TO_SIGN(hash_value) (((hash_value) & 1) ? -1 : 1)
#define DEBUG_F 0

// fingprint no used

// use a 16-bit prime, so 2 * a mod PRIME will not overflow
static const uint32_t PRIME_ID = MAXPRIME[32]; //PRIME for Fermat_sketch
static const uint32_t PRIME_ID_COUNT = MAXPRIME[31]; //PRIME for Fermat_count
static const uint32_t PRIME_FING = MAXPRIME[32];

inline uint64_t checkTable(uint64_t pos)
{
    // cout << "Using in checkTable PRIME_ID: " << PRIME_ID << endl;
    return powMod32(pos, PRIME_ID - 2, PRIME_ID);
}

inline uint64_t checkTable_count(uint64_t pos)
{
    // cout << "Using in checkTable_count PRIME_ID_COUNT: " << PRIME_ID_COUNT << endl;
    return powMod32(pos, PRIME_ID_COUNT - 2, PRIME_ID_COUNT);
}

class Fermat
{
    // bool use_fing;
    

public:

    int pure_cnt;
    unordered_map<uint32_t, int> insertedflows;

    

    virtual void clear_look_up_table() = 0;
    virtual void create_array() = 0;
    virtual void clear_array() = 0;

    virtual void Insert(uint32_t flow_id, uint32_t cnt) = 0;
    // virtual void Insert() = 0;
    virtual void Insert_one(uint32_t flow_id) = 0;

    virtual void Delete_in_one_bucket(int row, int col, int pure_row, int pure_col, int sign = 1) = 0;

    virtual bool verify(int row, int col, uint32_t &flow_id, uint32_t &fing) = 0;

    virtual void display() = 0;
    virtual int query(const char *key) = 0;
    virtual int undecoded_query(const char *key) = 0;
    virtual bool Decode(unordered_map<uint32_t, int> &result) = 0;
    virtual int get_id(int n_array, int n) = 0;
    virtual int get_counter(int n_array, int n) = 0;
    

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
    uint32_t **fingerprint;
    uint32_t **counter;
    uint32_t **idcpy, **fingcpy, **countercpy;
    // int decodeflag = 0;
    // // hash
    // BOBHash32 *hash;
    // BOBHash32 *hash_fp;

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

    bool verify(int row, int col, uint32_t &flow_id, uint32_t &fing) override
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
    bool Decode(unordered_map<uint32_t, int> &result) override
    {
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
        printf("1You are getting out Fermat Count version.\n");
    }

    Fermat_Count(int _memory, bool _fing, uint32_t _init) : use_fing(_fing), fingerprint(nullptr), hash_fp(nullptr)
    {
        printf("You are running Fermat Count version.\n");
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
        printf("2You are getting out Fermat Count version.\n");
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
    bool verify(int row, int col, uint32_t &flow_id, uint32_t &fing)
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
    // bool Decode(unordered_map<uint32_t, int> &result) override {
    //     cout << "You are in the wrong Decode()!" << endl;
    // }
    bool Decode(unordered_map<uint32_t, int> &result)
    {
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
                            if(counter[i][check] != abs(counter[i][check])) cout<<counter[i][check]<<" "<<abs(counter[i][check])<<" ";
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
};

#endif