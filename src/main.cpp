#include <iostream>
#include <string_view>
#include <random>
#include <cassert>

//#include "collection_tools.hxx"
#include "avl_tree.h"
#include "list.h"
#include "vector.h"
#include "vector_v1.h"
#include "hash_table.h"
#include "search.h"
#include "tools.h"

namespace
{
constexpr std::size_t L1_SIZE{1024*192};
constexpr std::size_t L2_SIZE{1024*1024+1024*512};
constexpr std::size_t L3_SIZE{1024*1024*9};
constexpr std::size_t MAX_SIZE{6500};
}

template<typename Col>
void print(const Col & coll)
{
    for (auto & item : coll)
    {
        std::cout << item << " ";
    }
    std::cout << "\n";
}

template<typename T>
void printStats(const ds::vector<T> & v, std::string_view name)
{
    std::cout << name << " is empty: " << std::boolalpha << v.empty() << "\n";
    std::cout << name << " size: " << v.size() << "\n";
    std::cout << name << " capacity: " << v.capacity() << "\n";
}

void testListIteration()
{
    constexpr auto maxSize{L1_SIZE};
    auto maxIterations{10000};
//    auto maxIterations{100};
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution{0, maxSize-1};
    ds::list<uint8_t> l1;

    auto arr = ts::generate_fix_sized_array<uint8_t, maxSize>();
    l1.insert(l1.begin(), arr.begin(), arr.end());

    for (unsigned i = 0; i < maxIterations; ++i)
    {
        int random_index = distribution(generator);
        auto it{l1.begin()};
        for (auto index = 0; index < random_index; ++index)
        {
            ++it;
        }
    }
}

void profileVectorRandomPositionInsert()
{
    auto maxIterations{10000};
    constexpr auto maxSize{L1_SIZE};

    auto arr = ts::generate_fix_sized_array<uint8_t, maxSize>();
    ds::vector<uint8_t> v1;
    v1.insert(v1.begin(), arr.begin(), arr.end());

    assert(v1.size() == maxSize);

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution{0, maxSize-1};
    for (std::size_t i = 0; i < maxIterations; ++i)
    {
        int random_index = distribution(generator);
        v1.insert((v1.begin() + random_index), 0);
    }
}

void profileVectorFrontPositionInsert()
{
    auto maxIterations{10000};
    constexpr auto maxSize{L1_SIZE};

    auto arr = ts::generate_fix_sized_array<uint8_t, maxSize>();
    ds::vector<uint8_t> v1;
    v1.insert(v1.begin(), arr.begin(), arr.end());

    assert(v1.size() == maxSize);

    for (std::size_t i = 0; i < maxIterations; ++i)
    {
        v1.insert(v1.begin(), 0);
    }
}

void profileVectorPushBack()
{
    auto maxIterations{100000000};
    constexpr auto maxSize{L1_SIZE};

//    auto arr = ts::generate_fix_sized_array<uint8_t, maxSize>();
    ds::vector<uint8_t> v1;

    for (std::size_t i = 0; i < maxIterations; ++i)
    {
        v1.push_back(0);
    }
}

void testRowMajorTraversal()
{
    auto maxIterations{100};
    constexpr auto rowSize{10000};
    constexpr auto columnSize{rowSize};
//    std::array<std::array<uint8_t, columnSize>, rowSize> 2d_arr = ts::generate_fix_sized_2d_array<uint8_t, rowSize, columnSize>();
//    auto arr{ts::generate_fix_sized_2d_array<uint8_t, rowSize, columnSize>()};
    auto arr{ts::generate_sorted_2d_vector<uint8_t>(rowSize, columnSize)};

    for (auto i = 0; i< maxIterations; ++i)
    {
        for (unsigned row = 0; row < rowSize; ++row)
        {
            for (unsigned column = 0; column < columnSize; ++column)
            {
                arr[row][column] = row + column;
            }
        }
    }
}

void testColumnMajorTraversal()
{
    auto maxIterations{100};
    constexpr auto rowSize{10000};
    constexpr auto columnSize{rowSize};
//    std::array<std::array<uint8_t, columnSize>, rowSize> 2d_arr = ts::generate_fix_sized_2d_array<uint8_t, rowSize, columnSize>();
//    auto arr{ts::generate_fix_sized_2d_array<uint8_t, rowSize, columnSize>()};
    auto arr{ts::generate_sorted_2d_vector<uint8_t>(rowSize, columnSize)};

    for (auto i = 0; i< maxIterations; ++i)
    {
        for (unsigned column = 0; column < columnSize; ++column)
        {
            for (unsigned row = 0; row < rowSize; ++row)
            {
                arr[row][column] = row + column;
            }
        }
    }
}

void testSearch()
{
    using namespace algo;
    auto v = ts::generate_sorted_vector<int>(4096);
//    auto [found, res] = branchyBinarySearch(v, 1501);
//    auto [found, res] = branchFreeBinarySearch(v, 1500);
    std::vector<int> dst;
    dst.resize(4097);
    eytzinger(v, dst);

    auto [found, res] = eytzingerSearch(dst, 1500);
    std::cout << " found: " << std::boolalpha << found << "; pos: " << res << "\n";
}

void testEytzinger()
{
    using namespace algo;
//    auto src = ts::generate_sorted_vector<int>(9);
    std::vector<int> src {1, 2, 3, 4, 5, 6, 7, 8};
//    std::vector<int> src {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    std::vector<int> dst;
//    dst.resize(16);
    dst.resize(9);
    eytzinger(src, dst);
    ts::print_vector(src);
    ts::print_vector(dst);
}

void testPrimes()
{
    for (auto i : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97})
    {
        if (!ts::isPrime(i))
        {
            std::cout << i << " is not prime\n";
        }
    }
    std::cout << " next prime of " << 37 << " is " << ts::nextPrime(37) << "\n";
    std::cout << " next prime of " << 43 << " is " << ts::nextPrime(43) << "\n";
    std::cout << " next prime of " << 71 << " is " << ts::nextPrime(71) << "\n";
    std::cout << " next prime of " << 89 << " is " << ts::nextPrime(89) << "\n";
    std::cout << " next prime of " << 97 << " is " << ts::nextPrime(97) << "\n";
    std::cout << " next prime of " << 258 << " is " << ts::nextPrime(258) << "\n";
}

void testList()
{
    using namespace ds;
    list<int> l;
    std::vector<int> v{5, 6, 9};

    auto it = l.begin();
    assert(l.begin() == l.end());
    assert(l.empty());
    l.push_back(1);
    l.insert(l.end(), 4);
    print(l);
    std::cout << l.front() << "\n";
    std::cout << l.back() << "\n";
    l.back() = 6;
    std::cout << l.back() << "\n";
    auto const_it = l.cbegin();
    ++const_it;
    std::cout << *const_it << "\n";

    l.insert(l.end(), v.begin(), v.end());
    print(l);
    
    assert(l.begin() != l.end());
    assert(!l.empty());

    list<int>l1{4, 2, 1, 12, 36, 78, 21, 320, 917, 0, -45, 789, -621, 431};
    auto initialSize(l1.size());;
    list<int>l2{91, 27, 9, 3, 1, 0};
    l2 = std::move(l1);
    std::cout << "printing l2\n";
    print(l2);
    std::cout << "l1 size: " << l1.size() << "\n";
    std::cout << "l1 empty: " << l1.empty() << "\n";
    std::cout << "l2 size: " << l2.size() << " initial size: " << initialSize << "\n";
}

void testVector()
{
    using namespace ds;
    ds::vector<std::size_t> v;
//    v.push_back(2.00);
//    printStats(v, "v after push_back");
//    print(v);
    v.insert(v.end(), 3);
    printStats(v, "v after first insert");
    print(v);
    v.insert(v.end(), 88);
    printStats(v, "v after insert");
    print(v);

    /*
    ds::vector<int> v;
    v.push_back(1);
    print(v);
    v.push_back(56);
    print(v);
    v.push_back(33);
    printStats(v, "v after push_back");
    print(v);
    */
    
    /*
    v.pop_back();
    printStats(v, "v after pop_back");
    v.reserve(50);
    printStats(v, "v after reserve");
    v.insert(v.begin(), 2);
    std::cout << v.front() << "\n";
    std::cout << *(v.begin()) << "\n";
    printStats(v, "v after insert at begin");
    v.clear();
    printStats(v, "v after clear");
    v.insert(v.end(), 3);
    std::cout << *(v.begin()) << "\n";
    std::cout << v.back() << "\n";
    printStats(v, "v after insert at end");
    ds::vector<double> vd{2.0, 3.65, 8.11, 56.21, 32.44};
    printStats(vd, "vd after construction");
    vector<long> vl;
    vl.insert(vl.end(), {41, 38, 7890, 6779, 21351, -1});
    printStats(vl, "vl after insertion");
    assert(vl.back() == -1);
    assert(vd.front() == 2.0);
    assert(*(vd.begin()) == 2.0);

    vector<char> vc;
    vc.insert(vc.end(), vl.begin(), vl.end());
    printStats(vc, "vc after insertion");
    print(vl);
    while(!vl.empty())
    {
        vl.pop_back();
    }
    printStats(vl, "vl after successive pop_back");
    */

    /*
     * move vector element construction to separate method
        T *raw_p = std::addressof(m_pArray[size]);
        alloc_traits::construct(getAlloc(), raw_p, std::move(value));
        ++size;
     * benchmark against both versions of vector; notice differences
     */
}

void testUnorderedSet()
{
    ds::unordered_set<int> table;
    /*
    std::cout << table.contains(0) << "\n";
    table.insert(50);
    std::cout << table.contains(50) << "\n";
    table.remove(50);
    std::cout << table.contains(50) << "\n";
    table.insert(49);
    table.insert(48);
    table.insert(47);
    table.insert(102);
    table.insert(1);
    table.insert(0);
    table.insert(101);
    std::cout << table.contains(0) << "\n";
    std::cout << table.contains(1) << "\n";
    std::cout << table.contains(101) << "\n";
    std::cout << table.contains(102) << "\n";


    ds::unordered_set<std::string> str_table;
    str_table.insert("ce");
    str_table.insert("mai");
    str_table.insert("faci");
    std::cout << str_table.contains("ce") << "\n";
    std::cout << str_table.contains("mai") << "\n";
    std::cout << str_table.contains("faci") << "\n";
    auto v = ts::generate_sorted_vector<int>(101);
    for (auto el : v)
    {
        table.insert(el);
    }
    table.insert(235);
    table.insert(236);
    */
    
}

int main()
{
//      testEytzinger();
    testUnorderedSet();
//    profileVectorFrontPositionInsert();
//    profileVectorRandomPositionInsert();
//    profileVectorPushBack();
    return 0;
}
