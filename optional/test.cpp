#include <optional.h>

#include <vector>
#include <list>
#include <string>
#include <type_traits>
#include <deque>
#include <memory>
#include <array>
#include <exception>
#include <cstddef>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

using Catch::Matchers::Equals;

template <class T>
void CheckValue(const Optional<T>& opt, const std::type_identity_t<T>& value) {
    CHECK(opt.HasValue());
    CHECK(*opt == value);

    const auto* begin = static_cast<const void*>(&opt);
    const auto* end = static_cast<const void*>(&opt + 1);
    const auto* p = static_cast<const void*>(&*opt);

    CHECK(std::greater_equal{}(p, begin));
    CHECK(std::less{}(p, end));
}

template <class T>
void CheckEmpty(const Optional<T>& opt) {
    CHECK_FALSE(opt.HasValue());
    CHECK_THROWS_AS(opt.Value(), std::exception);
}

TEST_CASE("Constructors") {
    Optional<int> a;
    CHECK_FALSE(a.HasValue());

    // Class template argument deduction (CTAD)
    // Optional<int>
    Optional b = 3;
    CheckValue(b, 3);

    int x = 2;
    Optional c = x;
    CheckValue(c, x);
}

TEST_CASE("No default constructor") {
    struct NoDefault {
        explicit NoDefault(int x) : x{x} {};
        bool operator==(const NoDefault&) const = default;

        int x;
    };

    Optional a = NoDefault{3};
    CheckValue(a, NoDefault{3});

    struct ThrowInDefault {
        ThrowInDefault() {
            throw std::runtime_error{"Don't call me"};
        }
    };
    CHECK_NOTHROW(Optional<ThrowInDefault>{});
}

TEST_CASE("Memory layout") {
    STATIC_CHECK(alignof(Optional<int>) >= alignof(int));
    STATIC_CHECK(sizeof(Optional<int>) >= sizeof(int));
    STATIC_CHECK(sizeof(Optional<int>) < 16);
    STATIC_CHECK(alignof(Optional<std::vector<int>>) >= alignof(std::vector<int>));
    STATIC_CHECK(sizeof(Optional<std::vector<int>>) >= sizeof(std::vector<int>));

    struct alignas(64) X {};
    STATIC_CHECK(alignof(Optional<X>) >= alignof(X));
    STATIC_CHECK(sizeof(Optional<X>) >= sizeof(X));

    struct Y {
        char a;
        Optional<int> b;
    };

    Y y{.a = {}, .b = 3};
    CheckValue(y.b, 3);

    STATIC_CHECK(sizeof(Optional<std::array<int, 1'000>>) >= 4'000);
}

TEST_CASE("Non trivial type") {
    std::vector v = {1, 2, 3};
    const auto expected = v;

    Optional a = v;
    CheckValue(a, expected);
    REQUIRE_THAT(v, Equals(expected));

    Optional b = std::move(v);
    REQUIRE(v.empty());
    CheckValue(b, expected);

    Optional<std::vector<int>> c;
    CHECK_FALSE(c.HasValue());
}

TEST_CASE("operator*") {
    Optional a = 2.2;
    CheckValue(a, 2.2);
    *a = 3.5;
    CheckValue(a, 3.5);

    Optional b = std::vector{1, 2, 3};
    CheckValue(b, {1, 2, 3});
    (*b).push_back(4);
    CheckValue(b, {1, 2, 3, 4});

    auto v = *std::move(b);
    CHECK((*b).empty());

    struct X {
        std::vector<int> v;
    };

    Optional c = X{{4, 5, 6}};
    REQUIRE_THAT((*c).v, Equals(std::vector{4, 5, 6}));
    auto v2 = (*std::move(c)).v;
    CHECK((*c).v.empty());
}

TEST_CASE("operator->") {
    Optional a = std::vector{1, 2, 3};
    a->push_back(4);
    CheckValue(a, std::vector{1, 2, 3, 4});

    Optional b = false;
    *(b.operator->()) = true;
    CheckValue(b, true);

    const Optional c = std::string{"abacaba"};
    CHECK(c->size() == 7);
}

TEST_CASE("Value") {
    Optional<int> a;
    CHECK_THROWS_AS(a.Value(), std::exception);

    Optional b = std::string{"abacaba"};
    CHECK(b.Value() == "abacaba");
    b.Value() = "foo";
    CHECK(b.Value() == "foo");

    const Optional c = 42;
    CHECK(c.Value() == 42);

    Optional d = std::vector{1, 2, 3};
    auto v = std::move(d).Value();
    CHECK_THAT(v, Equals(std::vector{1, 2, 3}));
    CHECK(d.Value().empty());
}

TEST_CASE("Copy constructor") {
    SECTION("int") {
        Optional a = 1;
        auto b = a;
        CheckValue(b, 1);
        *a = 2;
        CheckValue(b, 1);
    }
    SECTION("std::vector<int>") {
        Optional a = std::vector{1, 2, 3};
        auto b = a;
        CheckValue(b, {1, 2, 3});
        a->push_back(4);
        CheckValue(b, {1, 2, 3});
        b->push_back(5);
        CheckValue(b, {1, 2, 3, 5});
    }
    SECTION("Empty") {
        Optional<std::list<double>> a;
        auto b = a;
        CheckEmpty(a);
        CheckEmpty(b);
    }
}

TEST_CASE("Move constructor") {
    SECTION("std::list<int>") {
        Optional a = std::list{1, 2, 3};
        auto b = std::move(a);
        CheckValue(b, {1, 2, 3});
        CheckValue(a, {});
    }
    SECTION("Empty") {
        Optional<std::vector<std::string>> a;
        auto b = std::move(a);
        CheckEmpty(a);
        CheckEmpty(b);
    }
    SECTION("std::unique_ptr<std::vector<int>>") {
        Optional a = std::make_unique<std::vector<int>>(std::vector{1, 2, 3});
        auto b = std::move(a);
        CheckValue(a, nullptr);
        CHECK(b.Value() != nullptr);
    }
}

TEST_CASE("Reset") {
    Optional<int> a;
    a.Reset();
    CheckEmpty(a);

    Optional b = std::list{false, true, false};
    CheckValue(b, {false, true, false});
    b.Reset();
    CheckEmpty(b);

    Optional c = std::vector{1, 2, 3};
    c.Reset();
    auto d = c;
    CheckEmpty(d);
    auto e = std::move(c);
    CheckEmpty(e);
}

TEST_CASE("Copy assignment") {
    SECTION("int") {
        Optional a = 3;
        Optional b = 2;
        a = b;
        CheckValue(a, 2);
        CheckValue(b, 2);
    }
    SECTION("std::list<float>") {
        Optional a = std::list{1.f, 2.2f};
        Optional b = std::list{3.9f};
        a = b;
        CheckValue(a, {3.9f});
        CheckValue(b, {3.9f});
    }
    SECTION("Empty") {
        Optional a = std::vector{4, 1, -3};
        decltype(a) b;
        a = b;
        CheckEmpty(a);
        CheckEmpty(b);
    }
    SECTION("With value") {
        Optional a = std::string{"aba"};
        a = std::string{"caba"};
        CheckValue(a, "caba");
    }
    SECTION("Self-assignment") {
        Optional a = std::deque{15, 20, -18};
        const auto& r = a;
        a = r;
        CheckValue(a, {15, 20, -18});
    }
    SECTION("Tricky") {
        struct X {
            std::shared_ptr<Optional<X>> a;
        };
        Optional x = X{};
        x->a = std::make_shared<Optional<X>>(X{});
        x = *x->a;
        CHECK(x->a == nullptr);
    }
}

TEST_CASE("Move assignment") {
    SECTION("int") {
        Optional a = 3;
        Optional b = 2;
        a = std::move(b);
        CheckValue(a, 2);
        CheckValue(b, 2);
    }
    SECTION("std::vector<std::string>") {
        std::vector<std::string> expected = {"foo", "bar"};
        Optional a = expected;
        decltype(a) b;
        b = std::move(a);
        CheckValue(b, expected);
        CheckValue(a, {});
    }
    SECTION("Empty") {
        Optional<std::deque<int>> a;
        decltype(a) b;
        b = std::move(a);
        CheckEmpty(a);
        CheckEmpty(b);
    }
    SECTION("Tricky") {
        struct X {
            std::unique_ptr<Optional<X>> a;
        };
        Optional x = X{};
        x->a = std::make_unique<Optional<X>>(X{});
        x = std::move(*x->a);
        CHECK(x->a == nullptr);
    }
}

TEST_CASE("Get address") {
    struct X {
        X() = default;
        ~X() {
            CHECK(self == this);
        }
        X(const X&) {
        }
        const X* self = this;
    };

    Optional a = X{};
    CHECK(a->self == &*a);

    auto b = a;
    CHECK(b->self == &*b);
    CHECK(a->self != b->self);

    Optional<X> c;
    c = a;
    CHECK(c->self == &*c);
    CHECK(a->self != c->self);
}
