#include <iostream>
#include <gtest/gtest.h>
#include "set.h"
#include <set>
#include <algorithm>
#include <vector>
#include <random>
#include <iterator>
#include <iostream>


template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec) {
    for (const T& elem: vec) {
        out << elem << ' ';
    }
    return out;
}

template <typename Key>
bool operator==(const std::set<Key>& true_s, const Set<Key>& s) {
    return std::equal(s.begin(), s.end(),
                      true_s.begin(), true_s.end());
}

template <class Generator>
std::vector<int> GenerateRandom(int n, int min, int max, Generator& generator) {
    std::uniform_int_distribution<int> dist_size(1, n);
    std::vector<int> array(dist_size(generator));
    for (auto& a : array) {
        std::uniform_int_distribution<int> dist(min, max);
        a = dist(generator);
    }
    return array;
}


std::mt19937 GENERATOR(123);


TEST(SetTest, NullPtrTest) {
    Set<int> s1({-97, -13, -55, 51, 61, -7, 75, -51});
    Set<int> s2({-90, 43, 26, 99, 74, 51, -85, -15, 9, -97, 72, 33});
    Set<int> s3({41, -94, 51, 74, 45, 55, 62, -55, -99, -58, -72, 50, 11, 59});
    Set<int> s4({95, 20, 49, -40, 67, 42, -5, -50, -10, -18, -89, -11, -41, 73, 3});
    EXPECT_EQ(true, true);
}

TEST(SetTest, DefaultConstructorTest) {
    Set<int> s;
    std::set<int> true_s;
    EXPECT_EQ(true_s, s);
}

TEST(SetTest, RangeConstructorTest) {
    auto array = GenerateRandom(100, -100, 100, GENERATOR);
    Set<int> s(array.begin(), array.end());
    std::set<int> true_s(array.begin(), array.end());
    EXPECT_EQ(true_s, s);
}

TEST(SetTest, InitializerListConstructorTest) {
    Set<int> s({1, 2, 3, 55, 135, 284});
    std::set<int> true_s = {1, 2, 3, 55, 135, 284};
    EXPECT_EQ(true_s, s);
}

TEST(SetTest, CopyListConstructorTest) {
    auto array = GenerateRandom(100, -100, 100, GENERATOR);
    Set<int> s_src(array.begin(), array.end());
    Set<int> s = s_src;
    std::set<int> true_s(array.begin(), array.end());
    s_src.insert(20);
    s_src.insert(500);
    EXPECT_EQ(true_s, s);
}

TEST(SetTest, AssignOperatorTest) {
    auto array = GenerateRandom(100, -100, 100, GENERATOR);
    Set<int> s_src(array.begin(), array.end());
    Set<int> s({1, 2, 3, 4, 5});
    s = s_src;
    std::set<int> true_s(array.begin(), array.end());
    s_src.insert(20);
    s_src.insert(500);
    EXPECT_EQ(true_s, s);
}

TEST(SetTest, InsertTest) {
    auto array = GenerateRandom(100, -100, 100, GENERATOR);
    Set<int> s;
    std::set<int> true_s;
    for (auto elem: array)
        s.insert(elem), true_s.insert(elem);
    EXPECT_EQ(true_s, s);
}

TEST(SetTest, EraseTest) {
    auto array = GenerateRandom(100, -100, 100, GENERATOR);
    Set<int> s(array.begin(), array.end());
    std::set<int> true_s(array.begin(), array.end());
    std::uniform_int_distribution<int> dist(0, array.size());
    int num_to_erase = dist(GENERATOR);
    for (int i = 0; i < num_to_erase / 2; ++i)
        s.erase(array[i]), true_s.erase(array[i]);
    for (int i = num_to_erase / 2 + 1; i < num_to_erase; ++i) {
        std::uniform_int_distribution<int> dist(-100, 100);
        int random_num = dist(GENERATOR);
        s.erase(random_num);
        true_s.erase(random_num);
    }
    EXPECT_EQ(true_s, s);
}

TEST(SetTest, SizeTest) {
    auto array = GenerateRandom(100, -100, 100, GENERATOR);
    Set<int> s(array.begin(), array.end());
    std::set<int> true_s(array.begin(), array.end());
    EXPECT_EQ(true_s.size(), s.size());
    s.insert(30);
    s.insert(40);
    true_s.insert(400);
    true_s.insert(20);
    EXPECT_EQ(true_s.size(), s.size());
    s.erase(array[0]);
    true_s.erase(array[0]);
    EXPECT_EQ(true_s.size(), s.size());
    s = {};
    true_s = {};
    EXPECT_EQ(s.size(), true_s.size());
}

TEST(SetTest, EmptyTest) {
    auto array = GenerateRandom(100, -100, 100, GENERATOR);
    Set<int> s(array.begin(), array.end());
    std::set<int> true_s(array.begin(), array.end());
    EXPECT_EQ(true_s.empty(), s.empty());
    s = {};
    true_s = {};
    EXPECT_EQ(s.empty(), true_s.empty());
}

TEST(SetTest, FindTest) {
    auto array = GenerateRandom(100, -100, 100, GENERATOR);
    Set<int> s(array.begin(), array.end());
    std::set<int> true_s(array.begin(), array.end());
    EXPECT_EQ(*true_s.find(array[0]), *s.find(array[0]));
    EXPECT_EQ(true_s.find(150) == true_s.end(), s.find(150) == s.end());
}

TEST(SetTest, LowerBoundTest) {
    while (false) {
        auto array = GenerateRandom(100, -100, 100, GENERATOR);
        std::cout << array << '\n';
        Set<int> s(array.begin(), array.end());
    }
    auto array = GenerateRandom(100, -100, 100, GENERATOR);
    Set<int> s(array.begin(), array.end());
    std::set<int> true_s(array.begin(), array.end());
    s.insert(120);
    true_s.insert(120);
    EXPECT_EQ(*true_s.lower_bound(101), *s.lower_bound(101));
    EXPECT_EQ(*true_s.lower_bound(array[0]), *s.lower_bound(array[0]));
}

TEST(SetTest, IteratorsTest) {
    auto array = GenerateRandom(100, -100, 100, GENERATOR);
    Set<int> s(array.begin(), array.end());
    std::set<int> true_s(array.begin(), array.end());
    s.insert(3), s.insert(120), s.insert(12), s.insert(440), s.insert(8);
    true_s.insert(3), true_s.insert(120), true_s.insert(12), true_s.insert(440), true_s.insert(8);
    EXPECT_EQ(*true_s.begin(), *s.begin());
    EXPECT_EQ(*++true_s.begin(), *++s.begin());
    EXPECT_EQ(*std::next(true_s.begin(), 3), *std::next(s.begin(), 3));
    EXPECT_EQ(*--true_s.end(), *--s.end());
    EXPECT_EQ(*std::prev(true_s.end(), 4), *std::prev(s.end(), 4));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
