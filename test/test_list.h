#pragma once
#include <list>
#include <gtest/gtest.h>
#include "list.h"

namespace test
{
namespace ds_list
{

TEST(ListTests, TestSingleInsert)
{
    ds::list<int> l;
    l.insert(l.begin(), 2);
    EXPECT_EQ(l.size(), 1);
    EXPECT_EQ(*(l.begin()), 2);
    EXPECT_EQ(l.empty(), false);

    ds::list<int> l2;
    l2.pushFront(151);
    EXPECT_EQ(*(l2.begin()), 151);

    ds::list<long> l3;
    l3.pushBack(-421);
    EXPECT_EQ(l3.back(), -421);
    EXPECT_EQ(l3.front(), -421);
}

TEST(ListTests, NonDefaultConstructor)
{
    struct A
    {
        A(int val): value(val) {}
        int value;
    };

    A a(2);
    ds::list<A> l;
    l.pushBack(a);
    EXPECT_EQ(l.front().value, 2);
}

TEST(ListTests, TestImplicitValueCastInsert)
{
    ds::list<std::size_t> l;
    l.insert(l.begin(), 3.00);
    EXPECT_EQ(l.size(), 1);
    EXPECT_EQ(*(l.begin()), 3);
    EXPECT_EQ(l.empty(), false);
}

class TestInitializedList : public ::testing::Test
{
protected:
    TestInitializedList() :
        l1_{4, 2, 1, 12, 36, 78, 21, 320, 917, 0, -45, 789, -621, 431}
    {}
protected:
    ds::list<int> l1_;
};

TEST_F(TestInitializedList, TestInitializerListConstructor)
{
    EXPECT_EQ(l1_.size(), 14);
}

TEST_F(TestInitializedList, TestListContainsPassedItems)
{
    auto it = std::find(l1_.begin(), l1_.end(), 789);
    ASSERT_NE(l1_.end(), it);
    EXPECT_EQ(*it, 789);

    it = std::find(l1_.begin(), l1_.end(), 4);
    ASSERT_NE(l1_.end(), it);
    EXPECT_EQ(*it, 4);

    it = std::find(l1_.begin(), l1_.end(), 431);
    EXPECT_EQ(*it, 431);
    ASSERT_NE(l1_.end(), it);
}

TEST(ListTests, TestEmptyList)
{
    ds::list<std::string> l;
    EXPECT_EQ(l.empty(), true);
    EXPECT_EQ(l.size(), 0);
    EXPECT_EQ(std::begin(l), std::end(l));
}

TEST_F(TestInitializedList, TestCopyConstructor)
{
    ds::list<int> l2(l1_);
    EXPECT_EQ(l2.size(), l1_.size());
    EXPECT_EQ(l2.empty(), false);

    auto it = std::find(l2.begin(), l2.end(), -621);
    ASSERT_NE(l2.end(), it);
    EXPECT_EQ(*it, -621);

    it = std::find(l2.begin(), l2.end(), 4);
    ASSERT_NE(l2.end(), it);
    EXPECT_EQ(*it, 4);

    it = std::find(l2.begin(), l2.end(), 431);
    EXPECT_EQ(*it, 431);
    ASSERT_NE(l2.end(), it);
}

TEST_F(TestInitializedList, TestMoveConstructor)
{
    ds::list<int> l2(l1_);
    ds::list<int> l3(std::move(l2));

    EXPECT_EQ(l3.size(), l1_.size());
    EXPECT_EQ(l2.empty(), true);
    EXPECT_EQ(l3.empty(), false);

    auto it = std::find(l3.begin(), l3.end(), 917);
    ASSERT_NE(l3.end(), it);
    EXPECT_EQ(*it, 917);

    it = std::find(l3.begin(), l3.end(), 4);
    ASSERT_NE(l3.end(), it);
    EXPECT_EQ(*it, 4);

    it = std::find(l3.begin(), l3.end(), 431);
    EXPECT_EQ(*it, 431);
    ASSERT_NE(l3.end(), it);
}

TEST_F(TestInitializedList, TestCopyAssignment)
{
    ds::list<int> l2{55, 11};
    l2 = l1_;

    EXPECT_NE(l2.size(), 2);
    EXPECT_EQ(l2.size(), l1_.size());
    auto it = std::find(l2.begin(), l2.end(), 55);
    EXPECT_EQ(l2.end(), it);

    it = std::find(l2.begin(), l2.end(), 320);
    ASSERT_NE(l2.end(), it);
    EXPECT_EQ(*it, 320);

    it = std::find(l2.begin(), l2.end(), 4);
    ASSERT_NE(l2.end(), it);
    EXPECT_EQ(*it, 4);

    it = std::find(l2.begin(), l2.end(), 431);
    EXPECT_EQ(*it, 431);
    ASSERT_NE(l2.end(), it);
}

TEST_F(TestInitializedList, TestMoveAssignemnt)
{
    ds::list<int> l2(l1_);
    ds::list<int> l3{34, 23, 56};
    l3 = std::move(l2);

    EXPECT_EQ(l3.size(), l1_.size());
    EXPECT_NE(l3.size(), 3);
    EXPECT_EQ(l2.empty(), true);
    EXPECT_EQ(l3.empty(), false);

    auto it = std::find(l3.begin(), l3.end(), 917);
    ASSERT_NE(l3.end(), it);
    EXPECT_EQ(*it, 917);

    it = std::find(l3.begin(), l3.end(), 4);
    ASSERT_NE(l3.end(), it);
    EXPECT_EQ(*it, 4);

    it = std::find(l3.begin(), l3.end(), 431);
    EXPECT_EQ(*it, 431);
    ASSERT_NE(l3.end(), it);
}

TEST_F(TestInitializedList, TestSwap)
{
    ds::list<int> l2{713, 294, -639, 420, 11, 92, 47, 5, 31, 67};
    l2.swap(l1_);

    auto l1_it = std::find(std::begin(l1_), std::end(l1_), 420);
    ASSERT_NE(l1_.end(), l1_it);
    EXPECT_EQ(*l1_it, 420);

    l1_it = std::find(std::begin(l1_), std::end(l1_), 713);
    ASSERT_NE(l1_.end(), l1_it);
    EXPECT_EQ(*l1_it, 713);

    l1_it = std::find(std::begin(l1_), std::end(l1_), 67);
    ASSERT_NE(l1_.end(), l1_it);
    EXPECT_EQ(*l1_it, 67);

    auto l2_it = std::find(l2.begin(), l2.end(), 294);
    EXPECT_EQ(l2.end(), l2_it);

    l2_it = std::find(l2.begin(), l2.end(), 320);
    ASSERT_NE(l2.end(), l2_it);
    EXPECT_EQ(*l2_it, 320);

    l2_it = std::find(l2.begin(), l2.end(), 4);
    ASSERT_NE(l2.end(), l2_it);
    EXPECT_EQ(*l2_it, 4);

    l2_it = std::find(l2.begin(), l2.end(), 431);
    EXPECT_EQ(*l2_it, 431);
    ASSERT_NE(l2.end(), l2_it);
}

TEST(ListTests, TestInputIteratorsInsertion)
{
    std::list<int> inputList{5, 6, 7, 8};
    ds::list<int> destList;
    destList.insert(std::begin(destList), inputList.begin(), inputList.end());
    EXPECT_EQ(destList.size(), inputList.size());

    auto it = destList.begin();
    EXPECT_EQ(*it, 5);
    ++it;
    EXPECT_EQ(*it, 6);
    ++it;
    EXPECT_EQ(*it, 7);
    ++it;
    EXPECT_EQ(*it, 8);
}

TEST(ListTests, TestIteratorIncrement)
{
    std::list<char> l{'k', 'a', 'l', 'e', 'i', 'd', 'o', 's', 'c', 'o', 'p'};
    std::string str{"kaleidoscop"};
    std::size_t i{0};
    for (auto it = l.begin(); it != l.end(); ++it)
    {
        EXPECT_EQ(*it, str[i++]);
    }
}

TEST(ListTests, TestIteratorDecrement)
{
    ds::list<double> l;
    std::array<double, 4> arr{2.0, 3.1, 5.67, 7.91};
    l.insert(l.end(), arr.begin(), arr.end());
    std::size_t i{arr.size()};
    for (auto it = l.end(); it != l.begin();)
    {
        EXPECT_EQ(*(--it), arr[--i]);
    }
}

TEST(ListTests, TestListOfStrings)
{
    ds::list<std::string> l;
    std::string val{"val"};
    l.pushBack(val);
    EXPECT_EQ(l.back(), val);
    std::string val2{"char literal"};
    l.pushFront(val2.c_str());
    EXPECT_EQ(l.front(), val2);
    l.pushFront(std::move(val));
    EXPECT_EQ(l.front(), std::string{"val"});

}

TEST_F(TestInitializedList, TestInsertAtPosition)
{
    auto it = std::find(l1_.begin(), l1_.end(), 21);
    ASSERT_NE(l1_.end(), it);
    l1_.insert(it, 19);
    it = std::find(l1_.begin(), l1_.end(), 19);
    ASSERT_NE(l1_.end(), it);
    EXPECT_EQ(*it, 19);
    ++it;
    EXPECT_EQ(*it, 21);
}

TEST_F(TestInitializedList, TestErase)
{
    auto it = std::find(l1_.begin(), l1_.end(), 917);
    l1_.erase(it);
    it = std::find(l1_.begin(), l1_.end(), 917);
    EXPECT_EQ(it, l1_.end());
}

TEST_F(TestInitializedList, TestPopBack)
{
    l1_.popBack();
    auto it = std::find(l1_.begin(), l1_.end(), 431);
    EXPECT_EQ(it, l1_.end());
}

TEST_F(TestInitializedList, TestPopFront)
{
    l1_.popFront();
    auto it = std::find(l1_.begin(), l1_.end(), 4);
    EXPECT_EQ(it, l1_.end());
}

}//ds_list
}//test
