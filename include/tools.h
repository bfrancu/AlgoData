#pragma once
#include <random>
#include <limits>

#define likely(EXPR) __builtin_expect((bool)(EXPR), true)
#define unlikely(EXPR) __builtin_expect((bool)(EXPR), false)

namespace ts
{

template<typename IntType>
IntType getRandomNumberInRange(IntType lower, IntType upper)
{
    IntType result{0};
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<IntType> distr(lower, upper);
    result = distr(generator);
    return result;
}

template<typename IntType>
IntType getRandomNumberInMinMaxRange()
{
    return getRandomNumberInRange(std::numeric_limits<IntType>::min(),
                                  std::numeric_limits<IntType>::max());
}

inline std::size_t nextLargerPowerOf2(std::size_t val)
{
    std::size_t pow{1};
    std::size_t base{2};

    while (val > base)
    {
        base *= 2;
        ++pow;
    }
    return base;
}

/*
 * https://stackoverflow.com/questions/30052316/find-next-prime-number-algorithm
 * Based on the Sieve of Eratosthenes
 * Assuming every prime number comes in form of 6k +- 1
 * we test only divisibility only by 2,3 and numbers in form 6+-1
 */
inline bool isPrime(std::size_t val)
{
    if(val == 2 || val == 3)
    {
        return true;
    }

    if(val % 2 == 0 || val % 3 == 0)
    {
        return false;
    }

    std::size_t divisor{6};
    /*
     * we loop until we reach sqrt(val)
     * (x - 1)^2 = x^2 - 2*x + 1
     */
    while(divisor * divisor - 2 * divisor + 1 <= val)
    {
        if (val % (divisor - 1) == 0)
        {
            return false;
        }

        if (val % (divisor + 1) == 0)
        {
            return false;
        }

        divisor += 6;
    }

    return true;
}

inline std::size_t nextPrime(std::size_t val)
{
    while(!isPrime(++val))
    {}
    return val;
}

template<typename T>
bool belongsToRange(T *p, T *regionStart, std::size_t regionSize)
{
    return (ptrdiff_t)p >= (ptrdiff_t)regionStart &&
           (ptrdiff_t)p < (ptrdiff_t)regionStart + (ptrdiff_t)regionSize*sizeof(T);
}

}//ts
