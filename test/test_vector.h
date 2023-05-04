#pragma once
#include <vector>
#include <gtest/gtest.h>
#include "vector.h"

namespace test
{
namespace ds_vector
{

TEST(VectorTests, TestSingleInsert)
{
    ds::vector<int> v;
    v.insert(v.begin(), 2);
    EXPECT_EQ(v.size(), 1);
    EXPECT_EQ(*(v.begin()), 2);
    EXPECT_EQ(v.empty(), false);

    v.push_back(56);
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v.back(), 56);
    EXPECT_EQ(v.empty(), false);

    v.push_back(22);
    EXPECT_EQ(v.back(), 22);

    int val{33};
    v.push_back(33);
    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v.back(), 33);
}

TEST(VectorTests, TestImplicitValueCastInsert)
{
    ds::vector<std::size_t> v;
    v.push_back(2.00);
    EXPECT_EQ(v.front(), 2);
    EXPECT_EQ(v.empty(), false);
    EXPECT_EQ(v.size(), 1);

    v.insert(v.end(), 12);
//    EXPECT_EQ(v.back(), 12);
    EXPECT_EQ(v.size(), 2);
}

class TestInitializedVector : public ::testing::Test
{
protected:
    TestInitializedVector() :
        v1_{318, 32, 245, 9010, 916, 24, 0, -122, -2, 554, 3, 681, 7318, -512}
    {}

protected:
    ds::vector<int> v1_;
};

TEST_F(TestInitializedVector, TestInitializerListConstructor)
{
    EXPECT_EQ(v1_.size(), 14);
}

TEST_F(TestInitializedVector, TestVectorContainsPassedItems)
{
    auto it = std::find(std::begin(v1_), std::end(v1_), -512);
    ASSERT_NE(std::end(v1_), it);
    EXPECT_EQ(std::end(v1_), v1_.end());
    EXPECT_EQ(*it, -512);

    it = std::find(std::begin(v1_), std::end(v1_), -123);
    EXPECT_EQ(std::end(v1_), it);

    it = std::find(v1_.begin(), v1_.end(), 318);
    ASSERT_NE(std::end(v1_), it);
    EXPECT_EQ(*it, 318);

    it = std::find(v1_.begin(), v1_.end(), 0);
    ASSERT_NE(std::end(v1_), it);
    EXPECT_EQ(*it, 0);
}

TEST(VectorTests, TestEmptyVector)
{
    ds::vector<std::string> v;
    EXPECT_EQ(v.empty(), true);
    EXPECT_EQ(v.size(), 0);
    EXPECT_EQ(std::begin(v), std::end(v));
}

TEST_F(TestInitializedVector, TestCopyConstructor)
{
    ds::vector<int> v2(v1_);
    EXPECT_EQ(v2.size(), v1_.size());
    EXPECT_EQ(v2.empty(), false);
    EXPECT_NE(std::end(v2), std::find(std::begin(v2), std::end(v2), 3));

    auto it = std::find(v2.begin(), v2.end(), 554);
    EXPECT_EQ(*it, 554);
    EXPECT_EQ(v2.front(), 318);
    EXPECT_EQ(v2.back(), -512);

    it = std::find(v2.begin(), v2.end(), 331);
    EXPECT_EQ(*(std::end(v2)), *it);
}

TEST_F(TestInitializedVector, TestMoveConstructor)
{
    ds::vector<int> v2(v1_);
    ds::vector<int> v3(std::move(v1_));
    EXPECT_EQ(v2.size(), v3.size());
    EXPECT_EQ(v3.empty(), false);
    EXPECT_EQ(v1_.empty(), true);
    
    EXPECT_NE(std::end(v3), std::find(std::begin(v3), std::end(v3), 7318));

    auto it = std::find(v3.begin(), v3.end(), -2);
    EXPECT_EQ(*it, -2);
    EXPECT_EQ(*(std::begin(v3)), *(std::begin(v2)));
    EXPECT_EQ(v3.back(), v2.back());

    it = std::find(v3.begin(), v3.end(), 25);
    EXPECT_EQ(it, std::end(v3));
}

TEST_F(TestInitializedVector, TestCopyAssignment)
{
    ds::vector<int> v2;
    v2 = v1_;

    EXPECT_EQ(v2.size(), v1_.size());
    for (unsigned i = 0; i < v2.size(); ++i)
    {
        EXPECT_EQ(v2[i], v1_[i]);
    }
}

TEST_F(TestInitializedVector, TestMoveAssignemnt)
{
    ds::vector<int> v2(v1_);
    ds::vector<int> v3;
    v3 = std::move(v1_);

    EXPECT_EQ(v2.size(), v3.size());
    for (unsigned i = 0; i < v2.size(); ++i)
    {
        EXPECT_EQ(v2[i], v3[i]);
    }

    EXPECT_EQ(v1_.size(), 0);
    EXPECT_EQ(v1_.empty(), true);
    EXPECT_NE(v3.empty(), true);

    auto it = std::find(v3.begin(), v3.end(),  7318);
    ASSERT_NE(v3.end(), it);
}

TEST_F(TestInitializedVector, TestSwap)
{
    ds::vector<int> v2{134, 211, 0, 12, -234, 582, 92, 16, -3, -890, 512};
    auto v2Size{v2.size()};
    auto v1Size{v1_.size()};
    v2.swap(v1_);

    EXPECT_EQ(v1_.size(), v2Size);
    EXPECT_EQ(v1_.empty(), false);
    EXPECT_EQ(v1_.front(), 134);
    EXPECT_EQ(v1_.back(), 512);

    EXPECT_EQ(v2.size(), v1Size);
    EXPECT_EQ(v2.empty(), false);
    EXPECT_EQ(v2.front(), 318);
    EXPECT_EQ(v2.back(), -512);

    auto it1 = std::find(v1_.begin(), v1_.end(), 513);
    EXPECT_EQ(it1, v1_.end());

    it1 = std::find(v1_.begin(), v1_.end(), 12);
    ASSERT_NE(it1, v1_.end());
    EXPECT_EQ(*it1, 12);

    auto it2 = std::find(std::begin(v2), std::end(v2), 682);
    EXPECT_EQ(it2, v2.end());

    it2 = std::find(v2.begin(), v2.end(), 554);
    ASSERT_NE(it2, std::end(v2));
    EXPECT_EQ(*it2, 554);
}

TEST(VectorTests, TestInputIteratorsInsertion)
{
    std::vector<std::size_t> inputVector{99, 23, 124, 76, 553};
    ds::vector<int> destVector;
    destVector.insert(std::begin(destVector), std::begin(inputVector), std::end(inputVector));

    EXPECT_EQ(destVector.size(), inputVector.size());
    EXPECT_NE(destVector.empty(), true);
    for (unsigned i = 0; i < destVector.size(); ++i)
    {
        EXPECT_EQ(destVector[i], inputVector[i]);
    }
}

TEST(VectorTests, TestIteratorIncrement)
{
    ds::vector<std::string> v{"hello", "from", "the", "other", "side"};
    unsigned i{0};

    for (auto it = v.begin(); it < v.end() && i < v.size(); ++it)
    {
        EXPECT_EQ(*it, v[i++]);
    }
}

TEST(VectorTests, TestIteratorDecrement)
{
    ds::vector<double> v1{ 2.0, 1.3, 2.56, 33, 8.1};
    ds::vector<double> v2{v1};

    auto it2{v2.end()};
    for (auto it1 = v1.end(); it1 >= v1.begin();)
    {
        EXPECT_EQ(--it2, --it1);
    }
}

TEST(VectorTests, TestVectorOfStrings)
{
    ds::vector<std::string> v1;
    std::string val{"valerica"};
    std::string hei{"hei"};
    v1.push_back(val);

    EXPECT_EQ(v1.back(), val);

    v1.insert(v1.begin(), "hei");
    EXPECT_EQ(v1.front(), hei);

    v1.reserve(15);
    v1.insert(v1.end(),{"15", "16"});
    EXPECT_EQ(v1.back(), "16");
}

TEST_F(TestInitializedVector, TestInsertAtPosition)
{
    auto pos = std::find(v1_.begin(), v1_.end(), 916);
    auto index = std::distance(v1_.begin(), pos);
    v1_.insert(pos, 71);

    ASSERT_GT(index, 0);
    ASSERT_LT(index, v1_.size());
    EXPECT_EQ(v1_[index], 71);
}

TEST_F(TestInitializedVector, TestClear)
{
    auto it = std::find(v1_.begin(), v1_.end(), 24);
    ASSERT_NE(it, std::end(v1_));
    EXPECT_EQ(*it, 24);

    v1_.erase(std::find(std::begin(v1_), std::end(v1_), 24));
    it = std::find(v1_.begin(), v1_.end(), 24);
    EXPECT_EQ(it, std::end(v1_));

    v1_.clear();
    EXPECT_EQ(v1_.size(), 0);
    EXPECT_EQ(v1_.empty(), true);
    EXPECT_EQ(v1_.begin(), v1_.end());
}

TEST_F(TestInitializedVector, TestPopBack)
{
    auto initialSize{v1_.size()};
    EXPECT_EQ(v1_.size(), initialSize);
    EXPECT_EQ(v1_.back(), -512);

    v1_.pop_back();
    EXPECT_EQ(v1_.size(), initialSize-1);
    EXPECT_EQ(v1_.back(), 7318);
}

}//ds_vector
}//test

