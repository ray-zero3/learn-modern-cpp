// ============================================================
// tests/test_osc_sender.cpp
// ------------------------------------------------------------
// OscSender の単体テスト。
//
// テスト設計の方針:
//   - 実ネットワークを使わずに variant / visitor のロジックをテスト
//   - OscSender の接続状態テスト（正常な 127.0.0.1:9000 へのバインド）
//   - send() の戻り値テスト
//
// 注意: 実際の UDP 送信はテストしない（受信側が不在では成功/失敗を
//   判定できない）。代わりに接続状態と戻り値の整合性をテストする。
// ============================================================
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "media/osc_value.hpp"
#include "media/osc_sender.hpp"

#include <variant>
#include <string>

// ============================================================
// OscValue (std::variant) のテスト
// ============================================================

TEST_CASE("OscValue: int を保持できる") {
    media::OscValue val = 42;
    // std::get<T>: 指定した型で取り出す（型が違う場合は例外）
    CHECK(std::get<int>(val) == 42);
    // std::holds_alternative<T>: 型チェック（TS の typeof に相当）
    CHECK(std::holds_alternative<int>(val) == true);
    CHECK(std::holds_alternative<float>(val) == false);
}

TEST_CASE("OscValue: float を保持できる") {
    media::OscValue val = 3.14F;
    CHECK(std::holds_alternative<float>(val) == true);
    CHECK(std::get<float>(val) == doctest::Approx(3.14F));
}

TEST_CASE("OscValue: string を保持できる") {
    media::OscValue val = std::string("hello");
    CHECK(std::holds_alternative<std::string>(val) == true);
    CHECK(std::get<std::string>(val) == "hello");
}

TEST_CASE("OscValue: bool を保持できる") {
    media::OscValue val = true;
    CHECK(std::holds_alternative<bool>(val) == true);
    CHECK(std::get<bool>(val) == true);
}

TEST_CASE("OscValue: std::visit で型に応じた処理ができる") {
    // std::visit + overload パターン（TS の switch(typeof) に相当）
    // variant の実際の型に応じてラムダを選択して呼び出す
    auto get_type_name = [](const media::OscValue& v) -> std::string {
        return std::visit([](const auto& val) -> std::string {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, int>)         return "int";
            if constexpr (std::is_same_v<T, float>)       return "float";
            if constexpr (std::is_same_v<T, std::string>) return "string";
            if constexpr (std::is_same_v<T, bool>)        return "bool";
            return "unknown";
        }, v);
    };

    CHECK(get_type_name(42)              == "int");
    CHECK(get_type_name(3.14F)           == "float");
    CHECK(get_type_name(std::string("x")) == "string");
    CHECK(get_type_name(true)            == "bool");
}

// ============================================================
// OscSender のテスト
// ============================================================

TEST_CASE("OscSender: 有効な接続先で is_connected が true") {
    // ローカルの 9000 番ポートへの UDP 送信ソケット生成。
    // 受信側がいなくても UDP ソケット生成自体は成功するはず。
    media::OscSender sender("127.0.0.1", 9000);
    CHECK(sender.is_connected() == true);
}

TEST_CASE("OscSender: send が true を返す（ソケットが有効な場合）") {
    media::OscSender sender("127.0.0.1", 9000);

    // 送信（受信側がいなくても UDP は成功する）
    std::vector<media::OscValue> args = {
        0.5F,                    // centroid x
        0.3F,                    // centroid y
        1500.0F,                 // area
    };
    bool ok = sender.send("/tracker/centroid", args);
    CHECK(ok == true);
}

TEST_CASE("OscSender: 複数の型の引数を送れる") {
    media::OscSender sender("127.0.0.1", 9001);

    std::vector<media::OscValue> args = {
        42,                      // int
        3.14F,                   // float
        std::string("hello"),    // string
        true,                    // bool
    };
    bool ok = sender.send("/test/mixed", args);
    CHECK(ok == true);
}
