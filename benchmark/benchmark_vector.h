#pragma once
//#include <vector>
#include <benchmark/benchmark.h>
#include "vector.h"
#include "vector_v1.h"

inline void bm_vectorDsPushBack(benchmark::State & state)
{
    ds::vector<int> v;
    for (auto _ : state)
    {
        v.push_back(0);
    }
}

inline void bm_vectorV1DsPushBack(benchmark::State & state)
{
    ds_1::vector<int> v;
    for (auto _ : state)
    {
        v.push_back(0);
    }
}


inline void bm_vectorDsPushBackWithReserve(benchmark::State & state)
{
    ds::vector<int> v;
    v.reserve(state.max_iterations);
    for (auto _ : state)
    {
        v.push_back(0);
    }
}

inline void bm_vectorV1DsPushBackWithReserve(benchmark::State & state)
{
    ds_1::vector<int> v;
    v.reserve(state.max_iterations);
    for (auto _ : state)
    {
        v.push_back(0);
    }
}


#if defined(RUN_VECTOR_BENCHMARK)
BENCHMARK(bm_vectorDsPushBack);
BENCHMARK(bm_vectorV1DsPushBack);
BENCHMARK(bm_vectorV1DsPushBackWithReserve);
BENCHMARK(bm_vectorDsPushBackWithReserve);
#endif
