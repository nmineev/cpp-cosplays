#include <intrusive_list.h>

#include <string>
#include <utility>
#include <concepts>
#include <deque>
#include <ranges>
#include <random>
#include <algorithm>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

using Catch::Matchers::RangeEquals;

struct Item : public ListHook {
    explicit Item(int i) : i{i}, blob('a', 100) {
        blob += std::to_string(i);
    }

    int i;
    std::string blob;  // put nontrivial field here, hopefully incorrect code
                       // will hit double free

    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;
};

template <class... Args>
auto MakeList(Args*... args) {
    List<std::common_type_t<Args...>> list;
    (list.PushBack(args), ...);
    return list;
}

TEST_CASE("Empty list works") {
    List<Item> l1;
    REQUIRE(l1.IsEmpty());
    REQUIRE(l1.Size() == 0);

    Item i1{1};
    REQUIRE_FALSE(i1.IsLinked());
}

TEST_CASE("Back works") {
    List<Item> l;
    Item i1{1}, i2{2}, i3{3};

    l.PushBack(&i1);
    REQUIRE(l.Back().i == 1);

    l.PushBack(&i2);
    REQUIRE(l.Back().i == 2);
    REQUIRE(l.Size() == 2);
    REQUIRE_FALSE(l.IsEmpty());

    REQUIRE(i1.IsLinked());
    REQUIRE(i2.IsLinked());

    l.PopBack();
    REQUIRE_FALSE(l.IsEmpty());
    REQUIRE(l.Size() == 1);
    REQUIRE(l.Back().i == 1);

    l.PopBack();
    REQUIRE(l.IsEmpty());
    REQUIRE(l.Size() == 0);

    REQUIRE_FALSE(i1.IsLinked());
    REQUIRE_FALSE(i2.IsLinked());
}

TEST_CASE("Front works") {
    List<Item> l;
    Item i1{1}, i2{2}, i3{3};

    l.PushFront(&i1);
    REQUIRE(l.Front().i == 1);
    REQUIRE(l.Back().i == 1);

    l.PushFront(&i2);
    REQUIRE(l.Back().i == 1);
    REQUIRE(l.Front().i == 2);
    REQUIRE(l.Size() == 2);
    REQUIRE_FALSE(l.IsEmpty());

    l.PopFront();
    REQUIRE_FALSE(l.IsEmpty());
    REQUIRE(l.Size() == 1);
    REQUIRE(l.Front().i == 1);

    l.PopBack();
    REQUIRE(l.IsEmpty());
    REQUIRE(l.Size() == 0);
}

TEST_CASE("Const methods") {
    List<Item> l;
    Item i1{1}, i2{2}, i3{3};
    l.PushFront(&i1);
    l.PushBack(&i2);
    REQUIRE(l.Front().i == 1);
    REQUIRE(l.Back().i == 2);

    const auto& cl = l;
    REQUIRE(cl.Front().i == 1);
    REQUIRE(cl.Back().i == 2);
    REQUIRE_FALSE(cl.IsEmpty());
    REQUIRE(cl.Size() == 2);
}

TEST_CASE("Static checks") {
    STATIC_CHECK_FALSE(std::default_initializable<ListHook>);
    STATIC_CHECK_FALSE(std::copy_constructible<ListHook>);
    STATIC_CHECK_FALSE(std::move_constructible<ListHook>);
    STATIC_CHECK_FALSE(std::assignable_from<ListHook&, const ListHook&>);
    STATIC_CHECK_FALSE(std::assignable_from<ListHook&, ListHook&&>);

    using L = List<Item>;
    STATIC_CHECK_FALSE(std::copy_constructible<L>);
    STATIC_CHECK(std::move_constructible<L>);
    STATIC_CHECK_FALSE(std::assignable_from<L&, const L&>);
    STATIC_CHECK(std::assignable_from<L&, L&&>);
    STATIC_CHECK(std::bidirectional_iterator<L::Iterator>);
}

TEST_CASE("Iteration") {
    Item i1{1}, i2{2}, i3{3};
    auto l = MakeList(&i1, &i2, &i3);

    auto i = 0;
    for (auto it = l.Begin(); it != l.End(); ++it) {
        REQUIRE(it->i == ++i);
    }
    REQUIRE(i == 3);

    i = 0;
    for (const auto& v : l) {
        REQUIRE(v.i == ++i);
    }
    REQUIRE(i == 3);

    i = 0;
    for (auto& v : l) {
        REQUIRE(v.i++ == ++i);
    }
    REQUIRE(i == 3);
}

TEST_CASE("Unlink works") {
    Item i1{1}, i2{2}, i3{3};
    auto l = MakeList(&i1, &i2, &i3);

    i3.Unlink();
    REQUIRE(l.Front().i == 1);
    REQUIRE(l.Back().i == 2);
    REQUIRE(l.Size() == 2);
    REQUIRE_FALSE(l.IsEmpty());

    i1.Unlink();
    REQUIRE(l.Front().i == 2);
    REQUIRE(l.Back().i == 2);
    REQUIRE(l.Size() == 1);
    REQUIRE_FALSE(l.IsEmpty());

    i2.Unlink();
    REQUIRE(l.Size() == 0);
    REQUIRE(l.IsEmpty());
}

TEST_CASE("Destructor works") {
    Item i1{1}, i2{2}, i3{3};
    {
        auto l = MakeList(&i1, &i2, &i3);
        REQUIRE(i1.IsLinked());
        REQUIRE(i2.IsLinked());
        REQUIRE(i3.IsLinked());
    }
    REQUIRE_FALSE(i1.IsLinked());
    REQUIRE_FALSE(i2.IsLinked());
    REQUIRE_FALSE(i3.IsLinked());
}

TEST_CASE("Item destructor works") {
    List<Item> l;
    {
        Item i1{1}, i2{2}, i3{3};
        l.PushBack(&i1);
        l.PushBack(&i2);
        l.PushBack(&i3);
    }
    REQUIRE(l.IsEmpty());
    REQUIRE(l.Size() == 0);
}

TEST_CASE("IteratorTo works") {
    Item i1{1}, i2{2}, i3{3};
    auto l = MakeList(&i1, &i2, &i3);
    auto it = l.IteratorTo(&i2);
    REQUIRE(it++->i == 2);
    REQUIRE(it++ == l.IteratorTo(&i3));
    REQUIRE(it == l.End());
}

TEST_CASE("Iterator operations") {
    Item i1{10}, i2{20}, i3{30}, i4{40};
    auto l = MakeList(&i1, &i2, &i3, &i4);
    REQUIRE(std::next(l.Begin(), 4) == l.End());
    REQUIRE(std::prev(l.End(), 4) == l.Begin());

    auto it = l.Begin();
    REQUIRE((it++)->i == 10);
    REQUIRE((++it)->i == 30);

    it = std::prev(l.End());
    REQUIRE((it--)->i == 40);
    REQUIRE((--it)->i == 20);

    it = l.Begin();
    Item front{0};
    l.PushFront(&front);
    REQUIRE(std::prev(it) == l.Begin());

    it = std::prev(l.End());
    Item back{0};
    l.PushBack(&back);
    REQUIRE(std::next(it) == std::prev(l.End()));
}

TEST_CASE("Iterator validation") {
    Item i1{10}, i2{20}, i3{30}, i4{40}, i5{50};
    auto l = MakeList(&i1, &i2, &i3, &i4, &i5);
    auto it1 = l.Begin();
    auto it2 = std::next(it1);
    auto it5 = std::prev(l.End());
    auto it4 = std::prev(it5);
    auto it3 = std::next(it2);

    REQUIRE(it2 == std::prev(it3));
    REQUIRE(it3 == std::next(it2));

    for (auto i = 0; const auto& it : {it1, it2, it3, it4, it5}) {
        REQUIRE(it->i == 10 * ++i);
        REQUIRE((*it).i == 10 * i);
    }

    i3.Unlink();
    REQUIRE(l.Size() == 4);
    REQUIRE(it2 == std::prev(it4));
    REQUIRE(it4 == std::next(it2));

    auto l2 = std::move(l);
    i1.Unlink();
    REQUIRE(l2.Size() == 3);
    REQUIRE(it2 == l2.Begin());
    REQUIRE(it4 == std::next(it2));
    REQUIRE(it4 == std::prev(l2.End(), 2));

    l = std::move(l2);
    i5.Unlink();
    REQUIRE(l.Size() == 2);
    REQUIRE(it2 == l.Begin());
    REQUIRE(it4 == std::prev(l.End()));

    it2->Unlink();
    i4.Unlink();
    REQUIRE(l.Size() == 0);
    REQUIRE(l.Begin() == l.End());
    for (const auto* p : {&i1, &i2, &i3, &i4, &i5}) {
        REQUIRE_FALSE(p->IsLinked());
    }
}

TEST_CASE("Move works") {
    Item i1{1}, i2{2}, i3{3};
    auto l1 = MakeList(&i1, &i2, &i3);

    auto l2 = std::move(l1);
    REQUIRE_FALSE(l2.IsEmpty());
    REQUIRE(l1.IsEmpty());

    l1 = std::move(l2);
    REQUIRE_FALSE(l1.IsEmpty());
    REQUIRE(l2.IsEmpty());
}

TEST_CASE("Swap works") {
    Item i1{1}, i2{2}, i3{3}, i4{4}, i5{5};
    auto l1 = MakeList(&i1, &i2, &i3);
    auto l2 = MakeList(&i4, &i5);

    auto i = 0;
    for (auto it = l1.Begin(); it != l1.End(); ++it) {
        REQUIRE(it->i == ++i);
    }
    REQUIRE(i == 3);
    for (auto it = l2.Begin(); it != l2.End(); ++it) {
        REQUIRE(it->i == ++i);
    }
    REQUIRE(i == 5);

    std::swap(l1, l2);
    i = 0;
    for (auto it = l2.Begin(); it != l2.End(); ++it) {
        REQUIRE(it->i == ++i);
    }
    REQUIRE(i == 3);
    for (auto it = l1.Begin(); it != l1.End(); ++it) {
        REQUIRE(it->i == ++i);
    }
    REQUIRE(i == 5);
}

TEST_CASE("Some move test 1") {
    Item i1{1}, i2{2}, i3{3};
    auto l1 = MakeList(&i1, &i2);
    auto l2 = MakeList(&i3);

    l1 = std::move(l2);
    REQUIRE_FALSE(i1.IsLinked());
    REQUIRE_FALSE(i2.IsLinked());
    REQUIRE(i3.IsLinked());

    auto i = 2;
    for (auto it = l1.Begin(); it != l1.End(); ++it) {
        REQUIRE(it->i == ++i);
    }
    REQUIRE(i == 3);
    REQUIRE(l2.Begin() == l2.End());
}

TEST_CASE("Some move test 2") {
    Item i1{1}, i2{2}, i3{3};
    List<Item> l1;
    l1 = MakeList(&i1, &i2, &i3);
    auto i = 0;
    for (auto it = l1.Begin(); it != l1.End(); ++it) {
        REQUIRE(it->i == ++i);
    }
    REQUIRE(i == 3);
}

class Int : public ListHook {
public:
    Int(int x) : x_{x} {
    }
    operator int() const {
        return x_;
    }

private:
    int x_;
};

TEST_CASE("Big random") {
    constexpr auto kRange = std::views::iota(0, 1'000'000);
    std::deque<Int> items(kRange.begin(), kRange.end());
    for (auto x : kRange) {
        items.emplace_back(x);
    }

    std::mt19937 gen{563'144};
    std::vector v_indexes(kRange.begin(), kRange.end());
    std::ranges::shuffle(v_indexes, gen);

    List<Int> list;
    std::deque<int> indexes;
    for (std::bernoulli_distribution dist; auto i : v_indexes) {
        if (dist(gen)) {
            list.PushBack(&items[i]);
            indexes.push_back(i);
        } else {
            list.PushFront(&items[i]);
            indexes.push_front(i);
        }
    }

    for (auto i : kRange) {
        if (i % 100'000 == 0) {
            REQUIRE_THAT(list, RangeEquals(indexes | std::views::drop(i)));
        }
        if (auto& item = items[indexes[i]]; item.IsLinked()) {
            list.IteratorTo(&item)->Unlink();
        } else {
            FAIL("Item is not linked");
        }
    }

    for (auto& item : items) {
        if (item.IsLinked()) {
            FAIL("Item is linked");
        }
    }
    CHECK(list.Size() == 0);
    CHECK(list.Begin() == list.End());
}
