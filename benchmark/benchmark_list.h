#pragma once
#include <list>
#include <random>
#include <benchmark/benchmark.h>
#include "list.h"
#include "collection_tools.hxx"

namespace bm
{
namespace ds_list
{

constexpr std::size_t L1_SIZE{1024*192};
constexpr std::size_t L2_SIZE{1024*1024+1024*512};
constexpr std::size_t L3_SIZE{1024*1024*9};
constexpr std::size_t MAX_SIZE{6500};

static_assert(sizeof(uint8_t) == 1);

}//ds_list
}//bm


inline void bm_listDsPushBack(benchmark::State & state)
{
    ds::list<int> l;
    for (auto _ : state)
    {
        l.pushBack(0);
    }
}

inline void bm_listStdPushBack(benchmark::State & state)
{
    std::list<int> l;
    for (auto _ : state)
    {
        l.push_back(0);
    }
}

inline void bm_listDsPushFront(benchmark::State & state)
{
    ds::list<int> l;
    for (auto _ : state)
    {
        l.pushFront(1);
    }
}


inline void bm_listStdPushFront(benchmark::State & state)
{
    std::list<int> l;
    for (auto _ : state)
    {
        l.push_front(1);
    }
}

inline void bm_listDsInsertEnd(benchmark::State & state)
{
    ds::list<int> l;
    for (auto _ : state)
    {
        l.insert(l.end(), 0);
    }
}

inline void bm_listStdInsertEnd(benchmark::State & state)
{
    std::list<int> l;
    for (auto _ : state)
    {
        l.insert(l.end(), 0);
    }
}

inline void bm_listDsCopy(benchmark::State & state)
{ 
    auto arr = ts::generate_fix_sized_array<int, bm::ds_list::MAX_SIZE>();
    ds::list<int> l;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(std::copy(arr.begin(), arr.end(), std::back_inserter(l)));
    }
}

inline void bm_listStdCopy(benchmark::State & state)
{ 
    auto arr = ts::generate_fix_sized_array<int, bm::ds_list::MAX_SIZE>();
    std::list<int> l;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(std::copy(arr.begin(), arr.end(), std::back_inserter(l)));
    }
}

inline void bm_listDsIteration(benchmark::State & state)
{
}

class BM_DSInitializedListFixture : public benchmark::Fixture
{
public:
    void SetUp(const ::benchmark::State & state)
    {
        auto arr = ts::generate_fix_sized_array<uint8_t, bm::ds_list::L1_SIZE>();
        l_.insert(l_.begin(), arr.begin(), arr.end());
    }

    void TearDown(const ::benchmark::State & state)
    {}

protected:
    std::default_random_engine generator_;
    std::uniform_int_distribution<int> distribution_{0, bm::ds_list::L1_SIZE-1};
    ds::list<uint8_t> l_;
};

#if defined (RUN_LIST_BENCHMARK)
BENCHMARK_F(BM_DSInitializedListFixture, DsListIteration)(benchmark::State & state)
{
    for (auto _ : state)
    {
        int random_index = distribution_(generator_);
        auto it{l_.begin()};
        for (auto i = 0; i < random_index; ++i)
        {
            ++it;
        }

    }
}
#endif

#if defined (RUN_LIST_BENCHMARK)
BENCHMARK(bm_listDsPushBack);
BENCHMARK(bm_listStdPushBack);
BENCHMARK(bm_listDsPushFront);
BENCHMARK(bm_listStdPushFront);
BENCHMARK(bm_listDsInsertEnd);
BENCHMARK(bm_listStdInsertEnd);
BENCHMARK(bm_listDsCopy);
BENCHMARK(bm_listStdCopy);
#endif
