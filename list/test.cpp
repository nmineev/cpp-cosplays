#include <list.h>

#include <string>
#include <vector>
#include <random>
#include <memory>
#include <list>
#include <iterator>
#include <ranges>
#include <algorithm>
#include <type_traits>
#include <numeric>
#include <cstdint>
#include <array>
#include <deque>
#include <tuple>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

using Catch::Matchers::RangeEquals;

template <class... Args>
auto MakeList(Args&&... args) {
    List<std::common_type_t<Args...>> list;
    (list.PushBack(std::forward<Args>(args)), ...);
    return list;
}

template <class... Args>
auto MakeListOfShared(Args&&... args) {
    return MakeList(std::make_shared<Args>(std::forward<Args>(args))...);
}

template <class... Args>
auto MakeListOfUnique(Args&&... args) {
    return MakeList(std::make_unique<Args>(std::forward<Args>(args))...);
}

void CheckEquals(List<int>& l, const auto& expected) {
    REQUIRE(l.Size() == expected.size());
    CHECK_THAT(l, RangeEquals(expected));
}

TEST_CASE("Empty") {
    List<int> l1;
    List<std::string> l2;
    List<std::vector<int>> l3;

    REQUIRE(l1.IsEmpty());
    REQUIRE(l2.IsEmpty());
    REQUIRE(l3.IsEmpty());

    REQUIRE(l1.Size() == 0);
    REQUIRE(l2.Size() == 0);
    REQUIRE(l3.Size() == 0);
}

TEST_CASE("Back and PushBack") {
    List<int> l;
    l.PushBack(0);
    REQUIRE(l.Back() == 0);
    l.PushBack(1);
    REQUIRE(l.Back() == 1);
    REQUIRE(l.Size() == 2);
    REQUIRE_FALSE(l.IsEmpty());
    l.PopBack();
    REQUIRE_FALSE(l.IsEmpty());
    REQUIRE(l.Size() == 1);
    REQUIRE(l.Back() == 0);
    l.PopBack();
    REQUIRE(l.IsEmpty());
    REQUIRE(l.Size() == 0);
}

TEST_CASE("Front and PushFront") {
    List<int> l;
    l.PushFront(0);
    REQUIRE(l.Front() == 0);
    REQUIRE(l.Back() == 0);
    l.PushFront(1);
    REQUIRE(l.Back() == 0);
    REQUIRE(l.Front() == 1);
    REQUIRE(l.Size() == 2);
    REQUIRE_FALSE(l.IsEmpty());
    l.PopFront();
    REQUIRE_FALSE(l.IsEmpty());
    REQUIRE(l.Size() == 1);
    REQUIRE(l.Front() == 0);
    l.PopBack();
    REQUIRE(l.IsEmpty());
    REQUIRE(l.Size() == 0);
}

struct MoveOnly {
    MoveOnly() = delete;
    explicit MoveOnly(int) {
    }
    MoveOnly(MoveOnly&&) = default;
    MoveOnly(const MoveOnly&) = delete;

    ~MoveOnly() {
        ++num_deleted;
    }
    MoveOnly& operator=(MoveOnly&&) = default;
    MoveOnly& operator=(const MoveOnly&) = delete;

    inline static int num_deleted;
};

TEST_CASE("Move-only type") {
    REQUIRE(MoveOnly::num_deleted == 0);
    {
        List<MoveOnly> l;
        REQUIRE(l.IsEmpty());
        {
            MoveOnly m{2};
            l.PushBack(std::move(m));
        }
        REQUIRE(MoveOnly::num_deleted == 1);
        REQUIRE(l.Size() == 1);

        auto l2 = std::move(l);
        REQUIRE(MoveOnly::num_deleted == 1);

        REQUIRE(l.Size() == 0);
        REQUIRE(l2.Size() == 1);
    }
    REQUIRE(MoveOnly::num_deleted == 2);
}

TEST_CASE("Const") {
    List<int> l;
    l.PushFront(0);
    l.PushBack(1);
    REQUIRE(l.Front() == 0);
    REQUIRE(l.Back() == 1);
    const auto& cl = l;
    REQUIRE(cl.Front() == 0);
    REQUIRE(cl.Back() == 1);
    REQUIRE_FALSE(cl.IsEmpty());
    REQUIRE(cl.Size() == 2);
}

TEST_CASE("Iteration") {
    STATIC_CHECK(std::bidirectional_iterator<List<int>::Iterator>);

    auto l = MakeList(1, 2, 3, 4);
    auto i = 0;
    for (auto it = l.Begin(); it != l.End(); ++it) {
        REQUIRE(*it == ++i);
    }
    REQUIRE(i == 4);
    i = 0;
    for (auto v : l) {
        REQUIRE(v == ++i);
    }
}

TEST_CASE("Iteration 2") {
    auto l = MakeList(1, 2, 3, 4, -7);
    for (auto& x : l) {
        x += 4;
    }
    CHECK_THAT(l, RangeEquals(std::array{5, 6, 7, 8, -3}));
}

TEST_CASE("Behaves like a std::list") {
    constexpr auto kNumIterations = 100'000;

    std::mt19937 gen{6'022'345};
    std::uniform_int_distribution random_int{0, 1000};
    std::uniform_int_distribution random_case{0, 6};

    List<int> list;
    std::list<int> sl;
    for (auto i : std::views::iota(0, 10'000)) {
        if (random_case(gen) < 3) {
            list.PushBack(i);
            sl.push_back(i);
        } else {
            list.PushFront(i);
            sl.push_front(i);
        }
    }

    for (auto i = 0; i < kNumIterations; i++) {
        if (auto code = random_case(gen); code == 0) {
            list.PushBack(random_int(gen));
            sl.push_back(list.Back());
        } else if (code == 1) {
            list.PushFront(random_int(gen));
            sl.push_front(list.Front());
        } else if (code < 4) {
            if (!list.IsEmpty()) {
                list.PopBack();
                sl.pop_back();
            }
        } else if (code < 6) {
            if (!list.IsEmpty()) {
                list.PopFront();
                sl.pop_front();
            }
        } else {
            REQUIRE_THAT(list, RangeEquals(sl));
        }
    }
}

TEST_CASE("Copy") {
    {
        auto l1 = MakeList(1, 2, 3, 4, 5);
        auto l2 = std::as_const(l1);
        l1.PushFront(0);
        l2.PopFront();
        CheckEquals(l1, std::array{0, 1, 2, 3, 4, 5});
        CheckEquals(l2, std::array{2, 3, 4, 5});

        l1 = std::as_const(l2);
        l1.PopFront();
        l2.PushFront(1);
        CheckEquals(l1, std::array{3, 4, 5});
        CheckEquals(l2, std::array{1, 2, 3, 4, 5});
    }
    {
        auto l1 = MakeListOfShared(0, 1, 2);
        auto l2 = l1;
        List<std::shared_ptr<int>> l3;
        l3 = l1;
        REQUIRE(l3.Size() == 3);
        REQUIRE(l2.Size() == 3);
        REQUIRE(l1.Front().use_count() == 3);
    }
}

void CheckValid(List<int>* p) {
    auto& l = *p;
    auto size = l.Size();

    l.PushFront(0);
    auto it = l.Begin();
    REQUIRE(l.Front() == 0);
    l.PushBack(-1);
    REQUIRE(l.Back() == -1);
    REQUIRE(l.Size() == size + 2);

    REQUIRE(*it == 0);
    REQUIRE(*std::prev(l.End()) == -1);
}

TEST_CASE("Move") {
    {
        auto l1 = MakeList(1, 2, 3);
        auto l2 = std::move(l1);
        CheckEquals(l2, std::array{1, 2, 3});
        CheckValid(&l1);

        auto l3 = MakeList(4, 5);
        l3 = std::move(l2);
        CheckEquals(l3, std::array{1, 2, 3});
        CheckValid(&l2);
    }
    {
        auto l1 = MakeListOfUnique(0, 1, 2);
        auto l2 = std::move(l1);
        REQUIRE(l2.Size() == 3);
        REQUIRE(*l2.Front() == 0);

        auto l3 = MakeListOfUnique(4, 5);
        l3 = std::move(l2);
        REQUIRE(l3.Size() == 3);
        REQUIRE(*l3.Front() == 0);
    }
}

TEST_CASE("Iterators") {
    auto l = MakeList(0, 1, 2);
    auto i = l.Begin();
    REQUIRE(*i == 0);
    REQUIRE(*(++i) == 1);
    REQUIRE(*(++i) == 2);

    REQUIRE(*(i++) == 2);
    REQUIRE(i == l.End());

    REQUIRE(*(--i) == 2);
    REQUIRE(*(--i) == 1);
    REQUIRE(*(i--) == 1);

    REQUIRE(i == l.Begin());

    i++;
    REQUIRE(i == ++l.Begin());
}

TEST_CASE("Iterators 2") {
    List<int> l;
    REQUIRE(l.Begin() == l.End());
    l.PushBack(4);
    REQUIRE(l.Begin() != l.End());
    REQUIRE(++l.Begin() == l.End());
    REQUIRE(l.Begin() == --l.End());
    REQUIRE(*l.Begin() == 4);
    REQUIRE(*--l.End() == 4);
}

TEST_CASE("Iterators and move") {
    auto l1 = MakeList(1, 2, 3);
    auto it = std::next(l1.Begin(), 2);
    auto l2 = std::move(l1);

    CHECK(*it == 3);
    CHECK(std::next(it) == l2.End());
    CHECK(std::prev(it) == std::next(l2.Begin()));
    CHECK(*std::prev(it) == 2);
}

TEST_CASE("Move speed") {
    constexpr auto kRange = std::views::iota(0, 1'000'000);
    List<int> a;
    List<int> b;
    for (auto i : kRange) {
        a.PushBack(i);
        if (i % 2) {
            b.PushBack(i);
        }
    }
    for ([[maybe_unused]] auto i : std::views::iota(0, 100'000)) {
        std::swap(a, b);
    }
    CHECK(a.Size() == kRange.size());
    CHECK(b.Size() == kRange.size() / 2);
}

TEST_CASE("Algorithms") {
    {
        auto list = MakeList(5, 2, 1, 0, -3);
        auto result = std::accumulate(list.Begin(), list.End(), 0);
        CHECK(result == 5);
    }
    {
        auto list = MakeList(std::string{"a"}, "b", "c", "d", "e", "f");
        std::ranges::reverse(list);
        CHECK_THAT(list, RangeEquals(std::vector{"f", "e", "d", "c", "b", "a"}));
    }
    {
        auto list = MakeList(false, false, false, true, true, false);
        auto it = std::ranges::find(list, true);
        CHECK(it == std::next(list.Begin(), 3));
        CHECK(std::ranges::count(list, true) == 2);
    }
    {
        auto list = MakeList(4, 5, -3, 1, 2, 4);
        std::ranges::rotate(list, std::next(list.Begin(), 4));
        CHECK_THAT(list, RangeEquals(std::vector{2, 4, 4, 5, -3, 1}));
    }
}

void CheckView(auto&& range, const auto& expected) {
    CHECK_THAT(range, RangeEquals(expected));
}

TEST_CASE("Views") {
    auto list = MakeList(2, 4, -4, 1, 0, -8);
    CheckView(list | std::views::take(4), std::array{2, 4, -4, 1});
    CheckView(list | std::views::drop(2), std::array{-4, 1, 0, -8});
    CheckView(std::views::reverse(list), std::array{-8, 0, 1, -4, 4, 2});

    constexpr auto kIsEven = [](int x) { return x % 2 == 0; };
    CheckView(list | std::views::filter(kIsEven), std::array{2, 4, -4, 0, -8});

    auto drop_even = list | std::views::drop_while(kIsEven);
    auto reversed = drop_even | std::views::reverse;
    CheckView(reversed, std::array{-8, 0, 1});
}

TEST_CASE("Fast move insert") {
    constexpr auto kNumIterations = 100'000;
    constexpr auto kSize = 1'000'000;

    auto l = MakeList(std::vector<int>(kSize));
    for (auto i = 0; i < kNumIterations; ++i) {
        l.PushFront(std::move(l.Back()));
        l.PushBack(std::move(l.Front()));
    }
    REQUIRE(l.Size() == 2 * kNumIterations + 1);
    auto view = l | std::views::reverse | std::views::drop(1);
    REQUIRE(std::ranges::distance(view.begin(), view.end()) == 2 * kNumIterations);
    for (auto& v : view) {
        REQUIRE(v.empty());
    }
    REQUIRE((l.Back().size() == kSize));
}

TEST_CASE("Erase") {
    auto l = MakeList(1, 2, 3, 4, 5);
    auto it1 = l.Begin();
    auto it2 = std::next(it1);
    auto it3 = std::next(it2);
    auto it4 = std::next(it3);
    auto it5 = std::next(it4);

    l.Erase(it3);
    CheckEquals(l, std::array{1, 2, 4, 5});
    CHECK(*std::next(it2) == 4);
    CHECK(*std::prev(it4) == 2);

    l.Erase(it1);
    CheckEquals(l, std::array{2, 4, 5});
    CHECK(l.Begin() == it2);

    l.Erase(it5);
    CheckEquals(l, std::array{2, 4});
    CHECK(*std::prev(l.End()) == 4);

    l.Erase(it4);
    CheckEquals(l, std::array{2});

    l.Erase(it2);
    CheckEquals(l, std::array<int, 0>{});
}

TEST_CASE("Big Erase") {
    std::deque<std::tuple<int*, List<int>::Iterator, std::list<int>::iterator>> elements;
    std::list<int> sl;
    List<int> l;

    std::mt19937 gen{43'539'124};
    std::bernoulli_distribution dist;
    for (auto i : std::views::iota(0, 100'000)) {
        if (dist(gen)) {
            l.PushBack(i);
            sl.push_back(i);
            elements.emplace_back(&l.Back(), std::prev(l.End()), std::prev(sl.end()));
        } else {
            l.PushFront(i);
            sl.push_front(i);
            elements.emplace_front(&l.Front(), l.Begin(), sl.begin());
        }
    }

    std::ranges::shuffle(elements, gen);
    for (auto i = 0; const auto& [p, it, s_it] : elements) {
        REQUIRE(*it == *s_it);
        REQUIRE(&*it == p);
        l.Erase(it);
        sl.erase(s_it);
        if (++i % 1'000 == 0) {
            CheckEquals(l, sl);
        }
    }
}
