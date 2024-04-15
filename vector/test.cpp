#include <vector.h>

#include <vector>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <iterator>
#include <cstddef>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

using Catch::Matchers::Equals;
using Catch::Matchers::RangeEquals;

void Check(const Vector& actual, const std::vector<int>& expected) {
    REQUIRE(actual.Size() == expected.size());
    for (size_t i = 0; i < actual.Size(); ++i) {
        if (actual[i] != expected[i]) {
            FAIL(actual[i] << " != " << expected[i] << " index (" << i << ")");
        }
    }
}

TEST_CASE("Vector has constructors") {
    {
        Vector a;
        CHECK(a.Size() == 0);
        CHECK(a.Capacity() == 0);
    }
    {
        Vector a = {1, 2, 3, 4};
        CHECK(a.Capacity() == 4);
        Check(a, {1, 2, 3, 4});
    }
    {
        Vector a(5);
        CHECK(a.Capacity() == 5);
        Check(a, std::vector<int>(5));
    }
}

TEST_CASE("Basic methods") {
    Vector a = {1, 3, 5};
    REQUIRE(a.Capacity() == 3);
    Check(a, {1, 3, 5});

    a.PopBack();
    Check(a, {1, 3});
    a.Clear();
    REQUIRE(a.Capacity() == 3);
    Check(a, {});

    a.PushBack(6);
    Check(a, {6});

    Vector b = {3, 4};
    a.Swap(b);
    Check(a, {3, 4});
    Check(b, {6});
}

TEST_CASE("Modifications with []") {
    Vector a = {3, 7, 8};
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    Check(a, {1, 2, 3});
}

TEST_CASE("Vector iterators 1") {
    STATIC_CHECK(std::random_access_iterator<Vector::Iterator>);

    Vector a = {0, 1, 2, 3, 4};
    auto first = a.begin();
    auto last = a.end();
    REQUIRE(last - first == 5);
    REQUIRE(first + 5 == last);
    REQUIRE(5 + first == last);
    REQUIRE(last - 5 == first);
    REQUIRE(last + (-5) == first);

    REQUIRE_FALSE(first == last);
    REQUIRE(first != last);
    REQUIRE(first < last);
    REQUIRE(first <= last);
    REQUIRE_FALSE(first >= last);
    REQUIRE_FALSE(first > last);

    REQUIRE(*first == 0);
    ++first;
    REQUIRE(*first == 1);
    auto tmp = first++;
    REQUIRE(*tmp == 1);
    REQUIRE(*first == 2);

    auto end = last--;
    REQUIRE(end == a.end());
    REQUIRE(*last == 4);
    --last;
    REQUIRE((first += 1) == last);
    last -= 3;
    REQUIRE(*last == 0);

    first = a.begin();
    REQUIRE(*(first + 2) == 2);
    REQUIRE(*(2 + first) == 2);
    REQUIRE(first[2] == 2);

    last = a.end();
    REQUIRE(*(last - 2) == 3);
    REQUIRE(*(-2 + last) == 3);
    REQUIRE(last[-2] == 3);

    *first = 10;
    REQUIRE(a[0] == 10);
    *--last = -10;
    REQUIRE(a[4] == -10);
}

TEST_CASE("Vector iterators 2") {
    Vector a = {0, 1, 2, 3, 4};
    auto first = a.begin();
    auto size = static_cast<int>(a.Size());
    for (auto i : std::views::iota(0, size)) {
        for (auto diff : std::views::iota(-2, 3)) {
            if (i + diff >= 0 && i + diff < size) {
                REQUIRE(*(first + i + diff) == i + diff);
            }
        }
    }
}

TEST_CASE("Vector iterators 3") {
    Vector a = {1, 3, 5};
    *(a.begin().operator->()) = 2;
    *((--a.end()).operator->()) = 4;

    Check(a, {2, 3, 4});

    Vector::Iterator it;
    it = a.begin() + 1;
    REQUIRE(*it == 3);
}

TEST_CASE("Range-based for") {
    Vector a(5);
    for (size_t i = 0; i < a.Size(); ++i) {
        a[i] = i;
    }
    for (auto cur = 0; auto x : a) {
        REQUIRE(cur++ == x);
    }
    for (auto& x : a) {
        ++x;
    }
    for (auto cur = 1; const auto& x : a) {
        REQUIRE(cur++ == x);
    }
}

TEST_CASE("Algorithms") {
    CHECK(std::ranges::max(Vector{2, 1, -3, 4, 5}) == 5);
    CHECK(std::ranges::min(Vector{-3, 0, -3, -4, 7}) == -4);
    {
        Vector v = {4, 0, -3, 4, 1, 2};
        CHECK(std::ranges::find(v, 1) == v.begin() + 4);
    }
    {
        Vector v(5);
        std::iota(v.begin(), v.end(), 3);
        Check(v, {3, 4, 5, 6, 7});
    }
    {
        Vector v = {4, 0, -3, 4, 1, 2};
        std::ranges::reverse(v);
        Check(v, {2, 1, 4, -3, 0, 4});
    }
    {
        Vector v = {4, 0, -3, 4, 1, 2};
        std::ranges::rotate(v, v.begin() + 2);
        Check(v, {-3, 4, 1, 2, 4, 0});
    }
    {
        Vector v = {4, 0, -3, 4, 1, 2};
        std::ranges::sort(v);
        Check(v, {-3, 0, 1, 2, 4, 4});
    }
    {
        Vector v1 = {4, 0, -3, 4, 1, 2};
        Vector v2(v1.Size());
        std::ranges::copy_backward(v1, v2.end());
        Check(v2, {4, 0, -3, 4, 1, 2});
    }
}
/*
TEST_CASE("Adaptors") {
    constexpr auto kIsOdd = [](int x) -> bool { return x % 2; };

    Vector v = {4, 0, -3, 4, 1, 2};
    CHECK_THAT(std::views::take(v, 4), RangeEquals(std::array{4, 0, -3, 4}));
    CHECK_THAT(std::views::drop(v, 3), RangeEquals(std::array{4, 1, 2}));
    CHECK_THAT(std::views::reverse(v), RangeEquals(std::array{2, 1, 4, -3, 0, 4}));
    CHECK_THAT(std::views::filter(v, kIsOdd), RangeEquals(std::array{-3, 1}));

    std::vector<std::vector<int>> result;
    for (auto subrange : std::views::split(v, 4)) {
        result.emplace_back(subrange.begin(), subrange.end());
    }
    std::vector<std::vector<int>> expected = {{}, {0, -3}, {1, 2}};
    CHECK_THAT(result, Equals(expected));
}
*/
TEST_CASE("Reallocations") {
    Vector data;
    REQUIRE(data.Capacity() == 0);
    data.PushBack(0);
    REQUIRE(data.Capacity() == 1);
    const auto* prev_p = &data[0];
    data.Clear();

    for (auto step : std::views::iota(0, 16)) {
        std::vector<int> ok_data;
        for (auto i : std::views::iota(0, (1 << step) + 1)) {
            data.PushBack(i);
            ok_data.push_back(i);
        }
        Check(data, ok_data);
        REQUIRE(prev_p != &data[0]);
        prev_p = &data[0];
        auto expected = static_cast<size_t>(1 << (step + 1));
        REQUIRE(data.Capacity() == expected);
        data.Clear();
        REQUIRE(data.Capacity() == expected);
    }
}

TEST_CASE("Reserve") {
    Vector a;
    a.Reserve(5);
    REQUIRE(a.Size() == 0);
    REQUIRE(a.Capacity() == 5);

    for (auto i : std::views::iota(0, 5)) {
        a.PushBack(i);
    }

    REQUIRE(a.Capacity() == 5);
    a.PushBack(4);
    REQUIRE(a.Capacity() == 10);
    a.Reserve(3);
    REQUIRE(a.Capacity() == 10);
    a.Clear();
    REQUIRE(a.Size() == 0);
    REQUIRE(a.Capacity() == 10);
}

TEST_CASE("Copy correctness") {
    Vector a;
    auto b = a;
    b.PushBack(1);
    b.PushBack(2);
    REQUIRE(a.Size() == 0);
    Check(b, {1, 2});

    const auto& r = b;
    b = r;
    Check(b, {1, 2});
    a = b;
    Check(a, {1, 2});

    b = std::move(a);
    Check(b, {1, 2});
    auto c = std::move(b);
    Check(c, {1, 2});

    Vector d = {3, 4, 5};
    auto e = d;
    Check(e, {3, 4, 5});
    d.Swap(c);
    Check(e, {3, 4, 5});
    Check(d, {1, 2});
    Check(c, {3, 4, 5});
}

TEST_CASE("Assign and emptiness") {
    {
        Vector a;
        Vector b = {4, 3};
        b = a;
        REQUIRE(b.Size() == 0);
    }
    {
        Vector a = {4, 3};
        Vector b;
        b = a;
        Check(b, {4, 3});
    }
}

TEST_CASE("Valid after move") {
    {
        Vector a = {1, 2};
        auto b = std::move(a);
        a.Clear();
        REQUIRE(a.Size() == 0);
        a.PushBack(1);
        a.PushBack(4);
        a.PushBack(3);
        Check(a, {1, 4, 3});
    }
    {
        Vector a = {1, 2};
        Vector b;
        b = std::move(a);
        a.Clear();
        REQUIRE(a.Size() == 0);
        a.PushBack(1);
        a.PushBack(4);
        a.PushBack(3);
        Check(a, {1, 4, 3});
    }
    {
        Vector a = {-34, 10};
        auto b = std::move(a);
        auto size = a.Size();
        for (auto i : std::views::iota(size_t{0}, size)) {
            a[i] = i;
        }
        for (auto i = 0; auto& x : a) {
            x = i++;
        }
        a.PushBack(3);
        a.PushBack(4);
        a.PushBack(5);
        CHECK(a[size] == 3);
        CHECK(a[size + 1] == 4);
        CHECK(a[size + 2] == 5);
        CHECK(a.Size() == size + 3);
    }
    {
        Vector a = {-34, 10};
        Vector b = {1, 2, 3};
        b = std::move(a);
        auto size = a.Size();
        for (auto i : std::views::iota(size_t{0}, size)) {
            a[i] = i;
        }
        for (auto i = 0; auto& x : a) {
            x = i++;
        }
        a.PushBack(3);
        a.PushBack(4);
        a.PushBack(5);
        CHECK(a[size] == 3);
        CHECK(a[size + 1] == 4);
        CHECK(a[size + 2] == 5);
        CHECK(a.Size() == size + 3);
    }
}

TEST_CASE("Move speed") {
    Vector v1, v2;
    for (auto i : std::views::iota(0, 3'000'000)) {
        v1.PushBack(i);
        v2.PushBack(-i);
    }
    for (auto i = 0; i < 100'000; ++i) {
        std::swap(v1, v2);
    }

    std::vector<Vector> vectors(100'000);
    vectors.front() = v1;
    for (size_t i = 1; i < vectors.size(); ++i) {
        vectors[i] = std::move(vectors[i - 1]);
    }
}

TEST_CASE("Self-assignment") {
    {
        Vector a;
        auto& r = a;
        a = r;
        Check(a, {});
    }
    {
        Vector a = {1, 2, 3};
        auto& r = a;
        a = r;
        Check(a, {1, 2, 3});
    }
}
