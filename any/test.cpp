#include <any.h>

#include <string>
#include <vector>
#include <typeinfo>
#include <ranges>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

using Catch::Matchers::Equals;

namespace {

template <class T>
void RequireEq(const Any& any, const T& value) {
    REQUIRE_FALSE(any.Empty());
    REQUIRE(any.GetValue<T>() == value);
}

template <class T>
void RequireEq(const Any& lhs, const Any& rhs) {
    REQUIRE_FALSE(lhs.Empty());
    REQUIRE_FALSE(rhs.Empty());
    REQUIRE(lhs.GetValue<T>() == rhs.GetValue<T>());
}

}  // namespace

TEST_CASE("Simple") {
    Any a = 5;
    RequireEq(a, 5);

    Any b = std::string{"abacaba"};
    RequireEq(b, std::string{"abacaba"});

    Any c;
    REQUIRE(c.Empty());
    c = 7.;
    RequireEq(c, 7.);

    Any d;
    int* ptr = nullptr;
    d = ptr;
    RequireEq(d, ptr);
}

TEST_CASE("Empty") {
    Any a;
    REQUIRE(a.Empty());

    std::vector t = {1, 2};
    Any b = t;
    RequireEq(b, t);
    a.Swap(b);

    RequireEq(a, t);
    REQUIRE(b.Empty());

    a.Clear();
    REQUIRE(a.Empty());

    a.Swap(b);
    REQUIRE(a.Empty());
    REQUIRE(b.Empty());

    Any c;
    auto d = c;
    REQUIRE(d.Empty());

    a = 5;
    RequireEq(a, 5);
    a = c;
    REQUIRE(a.Empty());

    Any e = nullptr;
    RequireEq(e, nullptr);
    REQUIRE_THROWS_AS(e.GetValue<void*>(), std::bad_cast);
}

TEST_CASE("Copy") {
    struct SomeStruct {
        int x;
        bool operator==(const SomeStruct&) const = default;
    };

    Any a = 5;
    auto b = a;

    RequireEq<int>(a, b);
    REQUIRE_FALSE(&a.GetValue<int>() == &b.GetValue<int>());

    Any c;
    c = b;

    RequireEq<int>(b, c);
    REQUIRE_FALSE(&b.GetValue<int>() == &c.GetValue<int>());
    b.Clear();
    REQUIRE(b.Empty());
    RequireEq(c, 5);

    Any d = SomeStruct{3};
    RequireEq(d, SomeStruct{3});

    d = std::string{"check"};
    RequireEq(d, std::string{"check"});

    const auto& str = std::string{"dorou"};
    Any e = str;
    const auto& r = e;
    e = r;
    RequireEq(e, str);

    a.Swap(e);
    RequireEq(a, str);
    RequireEq(e, 5);
}

TEST_CASE("Any can change type") {
    Any a = 5;
    RequireEq(a, 5);

    a = 2.3;
    RequireEq(a, 2.3);

    a = {};
    REQUIRE(a.Empty());

    a = std::vector{1, 2, 3};
    RequireEq(a, std::vector{1, 2, 3});
}

TEST_CASE("Any throws") {
    Any a = std::string{"just a test"};
    REQUIRE_THROWS_AS(a.GetValue<int>(), std::bad_cast);

    Any b = std::vector{1, 2, 3};
    RequireEq(b, std::vector{1, 2, 3});
    REQUIRE_THROWS_AS(b.GetValue<std::string>(), std::bad_cast);
    REQUIRE_THROWS_AS(b.GetValue<std::vector<double>>(), std::bad_cast);

    Any c;
    REQUIRE_THROWS_AS(c.GetValue<double>(), std::bad_cast);
    c = &c;
    RequireEq(c, &c);

    Any d = 2;
    REQUIRE_THROWS_AS(d.GetValue<double>(), std::bad_cast);

    d = static_cast<int*>(nullptr);
    RequireEq<int*>(d, nullptr);
    REQUIRE_THROWS_AS(d.GetValue<double*>(), std::bad_cast);
    REQUIRE_THROWS_AS(d.GetValue<void*>(), std::bad_cast);
    REQUIRE_THROWS_AS(d.GetValue<unsigned*>(), std::bad_cast);
}

TEST_CASE("Move") {
    Any a = 2.2;
    auto b = std::move(a);
    REQUIRE(a.Empty());
    RequireEq(b, 2.2);

    Any c;
    c = std::move(b);
    REQUIRE(b.Empty());
    RequireEq(c, 2.2);

    std::vector v = {5, 1, -34};
    auto expected = v;
    a = v;
    REQUIRE_THAT(v, Equals(expected));

    const auto* p = &v[1];
    Any d = std::move(v);
    RequireEq(d, expected);
    REQUIRE(v.empty());
    REQUIRE(&d.GetValue<std::vector<int>>()[1] == p);

    v = {1, 4, 1, 2};
    p = &v[3];
    d.Clear();
    REQUIRE(d.Empty());
    d = std::move(v);
    REQUIRE(v.empty());
    REQUIRE(&d.GetValue<std::vector<int>>()[3] == p);

    Any e = 5;
    e.Swap(e);
    RequireEq(e, 5);
}

TEST_CASE("Fast swap") {
    std::string str1(1'000'000, 'a');
    std::string str2(500'000, 'b');
    const auto& expected1 = str1;
    auto expected2 = str2;
    const auto* p = &str2[0];

    Any a = std::as_const(str1);
    Any b = std::move(str2);
    for (auto i = 0; i < 1'000'001; ++i) {
        std::swap(a, b);
        a.Swap(b);
        b.Swap(a);
    }
    RequireEq(a, expected2);
    RequireEq(b, expected1);
    REQUIRE(&a.GetValue<std::string>()[0] == p);
}

TEST_CASE("Vector of Any") {
    constexpr auto kRange = std::views::iota(0, 10'000);

    std::vector<Any> a;
    for (auto i : kRange) {
        a.emplace_back(i);
        a.emplace_back(a.back());
        a.emplace_back(3.14);
        a.emplace_back(true);
    }

    auto b = a;
    for (auto i : kRange) {
        RequireEq(b[4 * i], i);
        RequireEq(b[4 * i + 1], i);
        RequireEq(b[4 * i + 2], 3.14);
        RequireEq(b[4 * i + 3], true);
    }
}

TEST_CASE("Any inside Any") {
    struct X {
        Any x;
    };

    constexpr auto kDepth = 1'000;

    Any a = 5;
    for (auto i = 0; i < kDepth; ++i) {
        a = X{std::move(a)};
    }
    auto b = a;

    for (auto i = 0; i < kDepth; ++i) {
        a = a.GetValue<X>().x;
    }
    RequireEq(a, 5);

    for (auto i = 0; i < kDepth; ++i) {
        b = std::move(b.GetValue<X>().x);
    }
    RequireEq(b, 5);
}

TEST_CASE("Self-assignment") {
    constexpr auto kSize = 1'000'000;

    Any a = std::vector<int>(kSize);
    auto& r = a;
    for (auto i = 0; i < 100'000; ++i) {
        a = r;
    }
    REQUIRE(a.GetValue<std::vector<int>>().size() == kSize);

    Any b;
    auto& rb = b;
    b = rb;
    REQUIRE(b.Empty());
}

TEST_CASE("No default constructor") {
    struct NoDefault {
        NoDefault(int) {
        }
    };

    Any a = NoDefault{3};
    REQUIRE_NOTHROW(a.GetValue<NoDefault>());
    a = NoDefault{4};
    REQUIRE_FALSE(a.Empty());

    a.Clear();
    REQUIRE(a.Empty());
    REQUIRE_THROWS_AS(a.GetValue<NoDefault>(), std::bad_cast);

    Any b = NoDefault{6};
    a = b;
    REQUIRE_NOTHROW(a.GetValue<NoDefault>());
}
