#ifndef _COMMON_FUNC_H_
#define _COMMON_FUNC_H_

#include <iostream>
#include <utility>
#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <arpa/inet.h>
#include <cstring>
#include <random>
#include <stdexcept>

using std::make_pair;
using std::map;
using std::pair;
using std::set;
using std::string;
using std::unordered_map;
using std::vector;

/************************** Loading Traces ******************************/

#define NUM_TRACE 12               // Number of traces in DATA directory
#define TIMES 10                   // Times of each algorithm measuring the same
                                   // trace using different hash function
#define DATA_ROOT_15s "/data/data" // NUM_TRACE = 1, CAIDA
#define MY_RANDOM_SEED 813

int prime_seeds[] = {37, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, \
                      103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, \
                      163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, \
                      227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, \
                      281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, \
                      353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, \
                      421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, \
                      487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, \
                      569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, \
                      631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, \
                      701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, \
                      773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, \
                      857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, \
                      937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, \
                      1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, \
                      1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, \
                      1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, \
                      1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, \
                      1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, \
                      1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, \
                      1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, \
                      1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, \
                      1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, \
                      1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637,};




struct SRCIP_TUPLE
{
  char key[13];
};
struct REST_TUPLE
{
  char key[9];
};

typedef vector<SRCIP_TUPLE> TRACE;

TRACE traces[NUM_TRACE];

uint32_t ReadNTraces(int n)
{
    if (n < 0 || n > 10) {
        printf("[ERROR] Invalid value of n. It should be between 0 and 10.\n");
        return 0;
    }

    uint32_t total_pck_num = 0;
    char tmp[13] = {0};

    for (int i = 0; i <= n; i++) {
        string filename = "/home/FLC/ElasticSketchCode/data/" + to_string(i) + ".dat";
        FILE *file = fopen(filename.c_str(), "r");
        if (file == NULL) {
            printf("[ERROR] File %s not open\n", filename.c_str());
            continue;
        }

        SRCIP_TUPLE key;
        int window = 0;

        if (!fread(tmp, 13, 1, file)) {
            printf("[ERROR] File %s error!\n", filename.c_str());
            fclose(file);
            continue;
        }

        while (fread(tmp, 13, 1, file)) {
            memcpy(&key, tmp, 4);
            traces[window].push_back(key);
            total_pck_num++;
        }

        fclose(file);
        printf("[INFO] File %s scanned, packets number: %ld\n", filename.c_str(), traces[window].size());
    }

    printf("[INFO] Total packets number: %u\n", total_pck_num);
    return total_pck_num;
}

uint32_t myReadTraces()
{
  uint32_t total_pck_num = 0;
  string filename130000 = "/home/FLC/ERSketch/datasample/0.dat";
  char tmp[13] = {0};
  FILE *file1 = fopen(filename130000.c_str(), "r");
  if (file1 == NULL)
  {
    printf("[ERROR] file not open\n");
    exit(0);
  }
  SRCIP_TUPLE key;
  int window = 0;
  if (!fread(tmp, 13, 1, file1)){
    printf("[ERROR] file error!\n");
    exit(0);
  }
  while (fread(tmp, 13, 1, file1))
  {
    memcpy(&key, tmp, 4);
    traces[window].push_back(key);
    total_pck_num++;
  }
  printf("[INFO] Scanned, packets number:\n");
  fclose(file1);
  int i = 0;
  printf("[INFO] window %02d has %ld packets\n", i, traces[i].size());
  return total_pck_num;
}

uint32_t ReadTwoWindows()
{
  TRACE all_packets;
  uint32_t total_pck_num = 0;
  string filename130000 = "/home/FLC/ERSketch/datasample/0.dat";
  char tmp[13] = {0};
  FILE *file1 = fopen(filename130000.c_str(), "r");
  if (file1 == NULL)
  {
    printf("[ERROR] file not open\n");
    exit(0);
  }
  SRCIP_TUPLE key;
  int window = 0;
  if (!fread(tmp, 13, 1, file1)){
    printf("[ERROR] file error!\n");
    exit(0);
  }
  while (fread(tmp, 13, 1, file1))
  {
    memcpy(&key, tmp, 4);
    all_packets.push_back(key);
    total_pck_num++;
  }
  for(int i = 0; i < total_pck_num / 2; i++){
    key = all_packets[i];
    traces[0].push_back(key);
  }
  for(int i = total_pck_num / 2; i < total_pck_num; i++){
    key = all_packets[i];
    traces[1].push_back(key);
  }
  printf("[INFO] Scanned, packets number:\n");
  fclose(file1);
  printf("[INFO] window %02d has %ld packets\n", 0, traces[0].size());
  printf("[INFO] window %02d has %ld packets\n", 1, traces[1].size());
  return total_pck_num;
}

uint32_t ReadTraces()
{
  double starttime, nowtime;
  uint32_t total_pck_num = 0;
  string filename130000 = "/home/FLC/ERSketch/datasample/0.dat";
  // string filename130000 = "../data/130000.dat";
  char tmp[21] = {0};
  FILE *file1 = fopen(filename130000.c_str(), "r");
  if (file1 == NULL)
  {
    printf("[ERROR] file not open\n");
    exit(0);
  }
  SRCIP_TUPLE key;
  int window = 0;
  if (!fread(tmp, 21, 1, file1)){
    printf("[ERROR] file error!\n");
    exit(0);
  }
  starttime = *(double *)(tmp + 13);
  while (fread(tmp, 21, 1, file1))
  {
    nowtime = *(double *)(tmp + 13);
    if (nowtime - starttime >= 5.0)
    {
      window++;
      starttime = nowtime;
    }
    memcpy(&key, tmp, 4);
    traces[window].push_back(key);
    total_pck_num++;
  }
  printf("[INFO] 12 windows scanned, packets number:\n");
  for (int i = 0; i < 12; i++)
    printf("[INFO] window %02d has %ld packets\n", i, traces[i].size());
  printf("\n\n");
  return total_pck_num;
}
// uint32_t ReadTwoWindows()
// {
//   double starttime, nowtime;
//   uint32_t total_pck_num = 0;
//   string filename1 = "/home/FLC/ERSketch/datasample/0.dat";
//   // string filename1 = "/home/FLC/ERSketch/datasample/1.dat";
//   // string filename130000 = "../data/130000.dat";
//   char tmp[13] = {0};
//   // FILE *file1 = fopen(filename0.c_str(), "r");
//   FILE *file1 = fopen(filename1.c_str(), "r");
//   if (file1 == NULL)
//   {
//     printf("[ERROR] file not open\n");
//     exit(0);
//   }
//   SRCIP_TUPLE key;
//   int window = 0;
//   if (!fread(tmp, 13, 1, file1)){
//     printf("[ERROR] file error!\n");
//     exit(0);
//   }
//   starttime = *(double *)(tmp + 13);
//   while (fread(tmp, 13, 1, file1))
//   {
//     nowtime = *(double *)(tmp + 13);
//     if (nowtime - starttime >= 5.0)
//     {
//       cout << "Cumulated packets: " << total_pck_num << endl;
//       window++;
//       starttime = nowtime;
//     }
//     memcpy(&key, tmp, 4);
//     traces[window].push_back(key);
//     total_pck_num++;
//   }
//   window++;
//   cout << "total_packet_num: " << total_pck_num << endl;
//   cout << "window" << window <<endl;
//   printf("[INFO] 12 windows scanned, packets number:\n");
//   int cur_index = 0;
//   for(int i = 1; i < window; i++){
//     int cur_num = traces[i].size();
//     for(int j = 0; j < cur_num; j++){
//       key = traces[i][j];
//       traces[cur_index].push_back(key);
//       if(traces[cur_index].size() >= total_pck_num/2){
//         cur_index++;
//       }
//     }
//   } 
//   for (int i = 0; i < window; i++)
//     printf("[INFO] window %02d has %ld packets\n", i, traces[i].size());
//   printf("\n\n");
//   return total_pck_num;
// }
/************************** PREDEFINED NUMBERS***********************/
#define HH_THRESHOLD 500 // 20,000,000 * 0.0005 (0.05%)
// #define HH_THRESHOLD 350 // 20,000,000 * 0.0005 (0.05%)
#define HC_THRESHOLD 250
// #define HC_THRESHOLD 125
// #define HC_THRESHOLD 500
// #define HC_THRESHOLD 200
// #define HC_THRESHOLD 20
#define TOT_MEM 500
/************************** COMMON FUNCTIONS*************************/
#define ROUND_2_INT(f) ((int)(f >= 0.0 ? (f + 0.5) : (f - 0.5)))

/********************************************************************/

// Fermat_tower
// #define TOT_MEMORY TOT_MEM * 1024 
#define TOT_MEMORY 500 * 1024 
#define ELE_BUCKET 2500
#define ELE_THRESHOLD 250
#define USE_FING 0
#define INIT ((uint32_t)random() % 800)
#define FERMAT_EM_ITER 15
/********************************************************************/

// FCM+TopK (16-ary)
// Here, we consider the actual hardware implementation on Tofino.
// The actual register size of each bucket in each Top-K entry is (8 * 3 + 4) = 28 Byte,
// which is composed of 1 val_all (4B) + 3 key-value pairs (4 + 4) = 28 Byte.

#define JUDGE_IF_SWAP_FCMPLUS_P4(min_val, guard_val) ((guard_val >> 5) >= min_val)
#define FCMPLUS_DEPTH 2     // number of trees
#define FCMPLUS_LEVEL 3     // number of layer in trees
#if TOT_MEM >= 200
#define FCMPLUS_BUCKET 3072 // 2^12, num of entries for key-value pairs
#elif TOT_MEM == 150
#define FCMPLUS_BUCKET 1536
#elif TOT_MEM == 100
#define FCMPLUS_BUCKET 1024
#elif TOT_MEM == 50
#define FCMPLUS_BUCKET 512
#endif
#define FCMPLUS_K_ARY 16    // k-ary tree

#if FCMPLUS_K_ARY == 2
#define FCMPLUS_K_POW 1 // 2^1 = 2
#elif FCMPLUS_K_ARY == 4
#define FCMPLUS_K_POW 2 // 2^2 = 4
#elif FCMPLUS_K_ARY == 8
#define FCMPLUS_K_POW 3 // 2^3 = 8
#elif FCMPLUS_K_ARY == 16
#define FCMPLUS_K_POW 4 // 2^4 = 16
#elif FCMPLUS_K_ARY == 32
#define FCMPLUS_K_POW 5 // 2^5 = 32
#endif

// Config using 1.25MB
#define FCMPLUS_HEAVY_STAGE 4
#if TOT_MEM == 1000
#define FCMPLUS_WL1 384000 // width of layer 1 (number of registers)
#define FCMPLUS_WL2 24000   // width of layer 2 (number of registers)
#define FCMPLUS_WL3 1500    // width of layer 3 (number of registers)
#elif TOT_MEM == 800
#define FCMPLUS_WL1 294400
#define FCMPLUS_WL2 18400
#define FCMPLUS_WL3 1150
#elif TOT_MEM == 600
#define FCMPLUS_WL1 204800
#define FCMPLUS_WL2 12800
#define FCMPLUS_WL3 800
#elif TOT_MEM == 500
#define FCMPLUS_WL1 160000
#define FCMPLUS_WL2 10000
#define FCMPLUS_WL3 625
#elif TOT_MEM == 400
#define FCMPLUS_WL1 115200
#define FCMPLUS_WL2 7200
#define FCMPLUS_WL3 450
#elif TOT_MEM == 300
#define FCMPLUS_WL1 70400
#define FCMPLUS_WL2 4400
#define FCMPLUS_WL3 275
#elif TOT_MEM == 200
#define FCMPLUS_WL1 25600
#define FCMPLUS_WL2 1600
#define FCMPLUS_WL3 100
#elif TOT_MEM == 150
#define FCMPLUS_WL1 35072
#define FCMPLUS_WL2 2192
#define FCMPLUS_WL3 137
#elif TOT_MEM == 100
#define FCMPLUS_WL1 23296
#define FCMPLUS_WL2 1456
#define FCMPLUS_WL3 91
#elif TOT_MEM == 50
#define FCMPLUS_WL1 11776
#define FCMPLUS_WL2 736
#define FCMPLUS_WL3 46
#endif

typedef uint8_t FCMPLUS_C1; // 8-bit
#define FCMPLUS_THL1 254
typedef uint16_t FCMPLUS_C2; // 16-bit
#define FCMPLUS_THL2 65534
typedef uint32_t FCMPLUS_C3; // 32-bit
#define FCMPLUS_EM_ITER 15   // Num.iteration of EM-Algorithm. You can control.
/********************************************************************/

#define ELASTIC_BUCKET 3072
#define ELASTIC_HEAVY_STAGE 4
#define ELASTIC_WL TOT_MEM * 1024 - ELASTIC_BUCKET * ELASTIC_HEAVY_STAGE * 12
#define ELASTIC_TOFINO 0 
#define JUDGE_IF_SWAP_ELASTIC_P4(min_val, guard_val) ((guard_val >> 5) >= min_val)
#define ELASTIC_EM_ITER 15 // Num.iteration of EM-Algorithm. You can control.
// struct Bucket
// {
//   uint32_t key;
//   uint32_t val;
//   uint32_t guard_val;
// };
/********************************************************************/

// Count-Min
#define CM_BYTES TOT_MEM * 1024 
#define CM_DEPTH 3       // depth of CM
/********************************************************************/

// MRAC
#define MRAC_BYTES TOT_MEM * 1024 
#define MRAC_EM_ITER 15   // Num.iteration of EM-Algorithm. You can control.
/********************************************************************/

// HYPERLOGLOG
#define HLL_B 20       
#define HLL_REG_SIZE 8 // 8-bit register size
/********************************************************************/

// CUSKETCH (Count-Min + Conservative Update scheme)
#define CU_BYTES TOT_MEM * 1024 
#define CU_DEPTH 3       // depth of CU
/********************************************************************/

// PyramidSketch + Count-Min (PCMSketch)
#define MAX_HASH_NUM 20
#define LOW_HASH_NUM 4 // depth of PCM
typedef long long lint;
typedef unsigned int uint;
#define PCM_BYTES 1572864 // 1.5 * 1024 * 1024 = 1.5MB
/********************************************************************/

// UnivMon
#define UNIV_BYTES TOT_MEM * 1024 // 1.5 * 1024 * 1024 = 1.5MB
#define UNIV_BYTES_HC TOT_MEM * 1024
#define UNIV_LEVEL 14
#define UNIV_K 1000
#define UNIV_ROW 5
//#define UNIV_BYTES 1048576

/********************************************************************/

// Sieving
#define SIEVING_MEM TOT_MEM * 1024

/********************************************************************/
#endif
