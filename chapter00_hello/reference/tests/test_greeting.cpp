// ============================================================
// tests/test_greeting.cpp
// ------------------------------------------------------------
// doctest のテストファイル。ヘッダ1枚 + マクロで動く。
//
// DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN をこのファイルで定義すると
// doctest が自動で main() を生成してくれる (= 手書き不要)。
// ============================================================
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "greet/greeting.hpp"

TEST_CASE("greet: 名前を受け取って挨拶文を返す") {
    CHECK(greet::greet("World") == "Hello, World!");
    CHECK(greet::greet("Rei")   == "Hello, Rei!");
}

TEST_CASE("greet: 空文字でも例外を投げない") {
    // `string_view` は空でも有効な状態。ここで UB にならないこと。
    CHECK(greet::greet("") == "Hello, !");
}

TEST_CASE("classify_hour: 時刻帯の境界値") {
    using greet::TimeOfDay;
    using greet::classify_hour;

    SUBCASE("朝 (5-11)") {
        CHECK(classify_hour(5)  == TimeOfDay::Morning);
        CHECK(classify_hour(8)  == TimeOfDay::Morning);
        CHECK(classify_hour(11) == TimeOfDay::Morning);
    }
    SUBCASE("昼 (12-17)") {
        CHECK(classify_hour(12) == TimeOfDay::Afternoon);
        CHECK(classify_hour(17) == TimeOfDay::Afternoon);
    }
    SUBCASE("夕方 (18-21)") {
        CHECK(classify_hour(18) == TimeOfDay::Evening);
        CHECK(classify_hour(21) == TimeOfDay::Evening);
    }
    SUBCASE("夜 (それ以外)") {
        CHECK(classify_hour(0)  == TimeOfDay::Night);
        CHECK(classify_hour(4)  == TimeOfDay::Night);
        CHECK(classify_hour(22) == TimeOfDay::Night);
        CHECK(classify_hour(23) == TimeOfDay::Night);
    }
}

TEST_CASE("greet_at: 時刻帯別の挨拶") {
    using greet::TimeOfDay;
    CHECK(greet::greet_at("Rei", TimeOfDay::Morning)   == "Good morning, Rei!");
    CHECK(greet::greet_at("Rei", TimeOfDay::Afternoon) == "Good afternoon, Rei!");
    CHECK(greet::greet_at("Rei", TimeOfDay::Evening)   == "Good evening, Rei!");
    CHECK(greet::greet_at("Rei", TimeOfDay::Night)     == "Good night, Rei!");
}
