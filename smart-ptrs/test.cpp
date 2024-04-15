#include <smart_ptr.h>

#include <string>
#include <ranges>
#include <vector>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Empty shared") {
    SharedPtr a;
    SharedPtr b;
    b = a;
    auto c = a;
    b = std::move(c);
    REQUIRE(a.Get() == nullptr);
    REQUIRE(b.Get() == nullptr);
}

TEST_CASE("Reset shared") {
    auto* ptr1 = new std::string{"aba"};
    auto* ptr2 = new std::string{"caba"};
    SharedPtr a{ptr1};
    SharedPtr b{ptr2};
    REQUIRE(a.Get() == ptr1);
    REQUIRE(&*b == ptr2);
    REQUIRE(*b == *ptr2);
    a.Reset(nullptr);
    b = a;
}

TEST_CASE("Copy/move shared") {
    SharedPtr a{new std::string{"aba"}};
    std::string* ptr;
    {
        const auto b = a;
        REQUIRE(&*a == &*b);
        auto c = a;
        ptr = c.Get();
    }
    REQUIRE(ptr == a.Get());
    REQUIRE(ptr == &*a);
    REQUIRE(*ptr == "aba");

    SharedPtr b{new std::string{"caba"}};
    {
        auto c = b;
        auto d = b;
        d = std::move(a);
        REQUIRE(*c == "caba");
        REQUIRE(*d == "aba");
        b.Reset(new std::string{"test"});
        REQUIRE(*c == "caba");
    }
    REQUIRE(*b == "test");

    SharedPtr end;
    {
        SharedPtr d{new std::string{"delete"}};
        d = b;
        auto c = std::move(b);
        REQUIRE(*d == "test");
        REQUIRE(*c == "test");
        const auto& r = d;
        d = r;
        c = end;
        d.Reset(new std::string{"delete"});
        end = d;
    }

    {
        SharedPtr a{new std::string{"move"}};
        auto& r = a;
        a = std::move(r);
        REQUIRE(*r == "move");
    }

    REQUIRE(*end == "delete");
}

TEST_CASE("Empty weak") {
    WeakPtr a;
    WeakPtr b;
    a = b;
    auto c = a;
    b = std::move(c);

    auto shared = b.Lock();
    REQUIRE(shared.Get() == nullptr);
}

TEST_CASE("Weak expiration") {
    WeakPtr* a;
    {
        SharedPtr b{new std::string{"aba"}};
        auto c = b;
        a = new WeakPtr{c};
        auto test = a->Lock();
        REQUIRE(*test == "aba");
        REQUIRE_FALSE(a->IsExpired());
    }
    REQUIRE(a->IsExpired());
    delete a;
}

TEST_CASE("Weak extends shared") {
    auto* b = new SharedPtr{new std::string{"aba"}};
    WeakPtr c{*b};
    auto a = c.Lock();
    delete b;
    REQUIRE_FALSE(c.IsExpired());
    REQUIRE(*a == "aba");
}

TEST_CASE("Copy/move weak") {
    SharedPtr a{new std::string{"aba"}};
    WeakPtr b{a};
    WeakPtr empty;
    auto c = b;
    WeakPtr d{a};
    REQUIRE_FALSE(c.IsExpired());
    c = empty;
    REQUIRE(c.IsExpired());

    b = std::move(c);
    auto locked = d.Lock();
    REQUIRE(*locked == "aba");

    WeakPtr start{a};
    {
        auto a2 = a;
        WeakPtr f{a2};
        auto cur_lock = f.Lock();
        REQUIRE(cur_lock.Get() == SharedPtr{start}.Get());
    }
}

TEST_CASE("Shared from weak") {
    auto* x = new SharedPtr{new std::string{"aba"}};
    WeakPtr y{*x};
    delete x;
    REQUIRE(y.IsExpired());
    SharedPtr z{y};
    REQUIRE(z.Get() == nullptr);
}

TEST_CASE("Speed") {
    constexpr auto kRange = std::views::iota(0, 1'000'000);

    auto* ptr = new std::string;
    std::vector v = {SharedPtr{ptr}};
    WeakPtr weak{v[0]};
    for ([[maybe_unused]] auto i : kRange) {
        const auto& last = v.back();
        last->push_back('1');
        v.emplace_back(last);
    }

    REQUIRE(ptr->size() == kRange.size());
    REQUIRE_FALSE(weak.IsExpired());
    v.clear();
    REQUIRE(weak.IsExpired());
}
