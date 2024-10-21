# DaVinci Sketch: A Versatile Sketch for Efficient and  Comprehensive Set Measurements
## File Path
Our implementation is under Tasks/Heavy-hitter_detection_and_others/src/Davinci
## Davinci
- Initialize
```c++
unique_ptr<DaVinci<bucket_num>> davinci0 = make_unique<DaVinci<bucket_num>>(total_mem, fermat_mem, heavy_bucket_num, tower_mem, fermat_count, usefing, init_seed);
```
- Insert a flow
```c++
davinci->insert((const char *)(traces[0][i].key), 1);
```
- Union of 2 Sketches
```c++
Union<bucket_num>(*davinci0, *davinci1, *davinci_union_result, seed);
```
- Difference of 2 Sketches
```c++
Difference<bucket_num>(*davinci0, *davinci1, *davinci_diff_result, seed);
```

- Inner Product
```c++
long double innerpresult = InnerProduct<bucket_num>(*davinci0, *davinci1, seed);// will decode davinci0 and davinci1
```
- Get Distribution
```c++
vector<double> dist_result(10, 0);
davinci0->get_distribution(dist_result, 0); // need decode
```
- Get Entropy
```c++
double entropy_result = davinci0->get_entropy(dist_result); // need distribution
```

- Get Cardinality
```c++
int cardinality_result = davinci0->get_cardinality(); // need decode
```

- Get Heavy Hitters
```c++
set<uint32_t> davinci_hh;
davinci0->get_heavy_hitters(davinci_hh);
```

- Query Frequency
```c++
davinci0->query((const char *)flow_address, 1);
```
## How to run

```bash
$ cd Tasks/Heavy-hitter_detection_and_others/
$ make [TaskName]
$ ./build/[TaskName].out
```

[TaskName] can be checked in Tasks/Heavy-hitter_detection_and_others/Makefile

## Available Tasks
|TaskName|Description|
|-----------|-----------|
|countheap|Heavy hitters & heavy changers detection with CountHeap|
|cmsketch|Frequency estimation with CMSketch|
|coco|Heavy hitters & heavy changers detection with Coco|
|cusketch|Frequency estimation with CUSketch|
|elasticp4|Frequency estimation & heavy hitters & heavy changers & cardinality & distribution & entropy with ElasticP4|
|fcm|Frequency estimation & heavy hitters & heavy changers & cardinality & distribution & entropy with FCM|
|fermat|Frequency estimation & heavy hitters & heavy changers & cardinality & distribution & entropy with FermatTower|
|hashpipe|Heavy hitters with hashpipe|
|mrac|Distribution & entropy with MRAC|
|univmon|Heavy hitters & heavy changers & cardinality & entropy with Univmon|
|davinci|Frequency estimation with DaVinci|
|find_best_davinci|Find best parameters for DaVinci|
|davinci_uniontest|Union with DaVinci|
|davinci_difftest|Difference with DaVinci|
|davinci_innerproducttest|Innerproduct with DaVinci|
|davinci_distribution|Distribution with DaVinci|
|davinci_cardinality|Element Cardinality with DaVinci|
|davinci_entropy|Entropy with DaVinci|
|davinci_hc|Heavy changers with DaVinci|
|cm_union|Union with CMSketch|
|fermat_union|Union with Fermat|
|fermat_diff|Diiference with Fermat|
|flowradar_diff|Difference with FlowRadar|
|lossradar_diff|Difference with LossRadar|
|cm_innerp|Inner Product with CMSketch|
|fagms_innerp|Inner Product with FAGMS|
|joinsketch_innerp|Inner Product with JoinSketch|
|skim_innerp|Inner Product with SkimSketch|
|p4_union|Union with ElasticP4|
|lossradar_diff700|Difference with LossRadar|
|hashpipe_hh|Heavy hitter with HashPipe|
|fcm_fz|Frequency estimation with FCM|
|joinsketch_innerp700|Inner Product with JoinSktech|
|davinci_int|Integrated Tasks with Davinci|
|csoa1|Integrated Tasks with CSOA1|
|csoa2|Integrated Tasks with CSOA2|
|memory|Memory Access Test with Davinci|
|mat_csoa0|Memory Access Test with CSOA0|
|mat_csoa1|Memory Access Test with CSOA1|
|mat_csoa2|Memory Access Test with CSOA2|
|davinci_union_hh|Heavy hitters with union of Davinci|