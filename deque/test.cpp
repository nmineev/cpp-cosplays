#include <deque.h>
#include <util.h>

#include <string>
#include <vector>
#include <random>
#include <deque>
#include <ranges>
#include <cstddef>

#include <catch2/catch_test_macros.hpp>

void Check(const Deque& actual, const std::vector<int>& expected) {
    REQUIRE(actual.Size() == expected.size());
    for (auto i : std::views::iota(size_t{0}, expected.size())) {
        if (actual[i] != expected[i]) {
            FAIL(actual[i] << " != " << expected[i]);
        }
    }
}

void CheckEq(const Deque& a, const Deque& b) {
    REQUIRE(a.Size() == b.Size());
    for (auto i : std::views::iota(size_t{0}, a.Size())) {
        if (a[i] != b[i]) {
            FAIL(a[i] << " != " << b[i]);
        }
    }
}

TEST_CASE("Deque has constructors") {
    {
        Deque a;
        REQUIRE(a.Size() == 0);
    }
    {
        Deque a = {1, 2, 3, 4};
        Check(a, {1, 2, 3, 4});
    }
    {
        Deque a(5);
        Check(a, std::vector<int>(5));
    }
}

TEST_CASE("Basic methods") {
    Deque a = {1, 3, 5};
    Check(a, {1, 3, 5});

    a.PopBack();
    Check(a, {1, 3});
    a.PopFront();
    Check(a, {3});
    a.PushFront(5);
    Check(a, {5, 3});
    a.PushBack(1);
    Check(a, {5, 3, 1});

    a.Clear();
    Check(a, {});

    a.PushBack(3);
    Deque b{2, 4};
    a.Swap(b);
    Check(a, {2, 4});
    Check(b, {3});
}

TEST_CASE("Modifications with []") {
    Deque a = {9, 1, 1};
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    Check(a, {1, 2, 3});
}

TEST_CASE("Memory layout") {
    Deque a(129);
    for (auto i : std::views::iota(0, 127)) {
        REQUIRE(&a[i] + 1 == &a[i + 1]);
    }
}

TEST_CASE("Reallocations") {
    constexpr auto kRange = std::views::iota(0, 1'000'000);

    std::mt19937 gen{9'354};
    std::bernoulli_distribution dist;
    Deque a;
    std::deque<int*> addr;

    for (auto i : kRange) {
        if (dist(gen)) {
            a.PushBack(i);
            addr.push_back(&a[a.Size() - 1]);
        } else {
            a.PushFront(i);
            addr.push_front(&a[0]);
        }
    }

    for (auto i : kRange) {
        REQUIRE(&a[i] == addr[i]);
    }
}

TEST_CASE("Copy correctness") {
    Deque a;
    auto b = a;
    b.PushBack(1);
    Check(a, {});
    Check(b, {1});

    const auto& r = b;
    b = r;
    Check(b, {1});
    a = b;
    Check(a, {1});
    a.PushFront(3);
    a.PushFront(-3);
    a.PushBack(4);

    b = std::move(a);
    Check(b, {-3, 3, 1, 4});

    auto c = std::move(b);
    Check(c, {-3, 3, 1, 4});

    Deque d = {3, 4, 5};
    auto e = d;
    Check(e, {3, 4, 5});
    d.Swap(c);
    Check(e, {3, 4, 5});
    Check(d, {-3, 3, 1, 4});
    Check(c, {3, 4, 5});
}

TEST_CASE("Move speed") {
    constexpr auto kSize = 1'000'000;

    Deque a;
    Deque b;
    for (auto i = 0; i < kSize; ++i) {
        a.PushBack(i);
        if (i % 2) {
            b.PushBack(i);
        }
    }

    for (auto i = 0; i < 100'001; ++i) {
        std::swap(a, b);
    }
    CHECK(a.Size() == kSize / 2);
    CHECK(b.Size() == kSize);
}

TEST_CASE("Stress") {
    constexpr auto kNumIterations = 1'000'000;

    std::mt19937 gen{735'675};
    std::uniform_int_distribution dist{1, 10};
    Deque a;
    std::deque<int> b;
    std::deque<int*> p;
    for (auto i : std::views::iota(0, kNumIterations)) {
        if (dist(gen) > 1) {
            a.PushFront(i);
            b.push_front(i);
            p.push_front(&a[0]);
        } else {
            a.PushBack(i);
            b.push_back(i);
            p.push_back(&a[i]);
        }
    }

    for (auto i = 0; i < kNumIterations; ++i) {
        auto code = dist(gen);
        auto value = static_cast<int>(gen());
        if (code == 1) {
            a.PushFront(value);
            b.push_front(value);
            p.push_front(&a[0]);
        } else if (code == 2) {
            a.PushBack(value);
            b.push_back(value);
            p.push_back(&a[a.Size() - 1]);
        } else if (code < 6) {
            a.PopFront();
            b.pop_front();
            p.pop_front();
        } else if (code < 9) {
            a.PopBack();
            b.pop_back();
            p.pop_back();
        } else {
            auto index = value % a.Size();
            REQUIRE(a[index] == b[index]);
            REQUIRE(&a[index] == p[index]);
        }
        REQUIRE(a.Size() == b.size());
        if (i % (kNumIterations / 10) == 0) {
            auto c = a;
            auto d = std::move(a);
            a = c;
            c = std::move(d);
            std::swap(a, c);
            CheckEq(a, c);
        }
    }
    Check(a, {b.begin(), b.end()});
    REQUIRE(a.Size() == p.size());
    for (size_t i = 0; i < a.Size(); ++i) {
        CHECK(&a[i] == p[i]);
    }
}

void CheckEmptyCorrectness(void (Deque::*push)(int), void (Deque::*pop)()) {
    constexpr auto kTestSize = 1'000'000;

    Deque a;
    for (auto i = 0; i < kTestSize; ++i) {
        (a.*push)(i);
    }
    for (auto i = 0; i < kTestSize; ++i) {
        (a.*pop)();
    }
    Check(a, {});
}

TEST_CASE("Empty correctness") {
    // There are some ways to make deque empty
    // We should test them all
    // In some ways we can cause memory leak
    CheckEmptyCorrectness(&Deque::PushBack, &Deque::PopBack);
    CheckEmptyCorrectness(&Deque::PushBack, &Deque::PopFront);
    CheckEmptyCorrectness(&Deque::PushFront, &Deque::PopBack);
    CheckEmptyCorrectness(&Deque::PushFront, &Deque::PopFront);
}

TEST_CASE("Fast self-assignment") {
    constexpr auto kRange = std::views::iota(0, 1'000'000);

    Deque a;
    auto& r = a;
    for (auto i : kRange) {
        a.PushBack(i);
        a = r;
    }
    Check(a, {kRange.begin(), kRange.end()});
}

#ifdef __linux__

TEST_CASE("Memory usage", "[.][memory][no_asan]") {
    {
        auto before = GetMemoryUsage();
        std::vector<Deque> v(1'000);
        for (auto& d : v) {
            for (auto i : std::views::iota(0, 200)) {
                d.PushBack(i);
                d.PushFront(i);
            }
        }
        CHECK(before + 10'000 > GetMemoryUsage());
    }

    Deque a(1'000'000);
    CHECK(GetMemoryUsage() < 100'000);

    a.Clear();
    for (auto i : std::views::iota(0, 10'000'000)) {
        a.PushFront(i);
    }
    auto memory = GetMemoryUsage();
    CHECK(memory >= 40'000);
    CHECK(memory < 256'000);

    a.Clear();
    for (auto i : std::views::iota(0, 80'000'000)) {
        a.PushBack(i);
    }
    memory = GetMemoryUsage();
    CHECK(memory >= 320'000);
    CHECK(memory < 1'500'000);
}

#endif
