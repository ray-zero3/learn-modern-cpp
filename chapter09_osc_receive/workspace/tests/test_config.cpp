// ============================================================
// tests/test_config.cpp
// ------------------------------------------------------------
// Config 読み込みの単体テスト。
//
// テスト設計の方針:
//   - 正常 TOML: デフォルト設定と同じ値が読み込まれる
//   - 存在しないファイル: ConfigError が返る
//   - 型エラー (TOML 構文エラー): ConfigError が返る
//   - std::expected の .value() / .error() アクセステスト
// ============================================================
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "media/config.hpp"

#include <fstream>
#include <string>

// ============================================================
// ヘルパー: テスト用の一時 TOML ファイルを作成
// ============================================================
static void write_temp_file(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    f << content;
}

TEST_CASE("load_config: 存在しないファイルは ConfigError を返す") {
    auto result = media::load_config("/tmp/nonexistent_modern_cpp_config.toml");

    CHECK(result.has_value() == false);
    // エラーの確認: error() で ConfigError にアクセス
    CHECK(result.error().message.find("見つかりません") != std::string::npos);
}

TEST_CASE("load_config: 正常な TOML から Config を読み込める") {
    const std::string path = "/tmp/test_modern_cpp_config.toml";
    write_temp_file(path, R"toml(
[camera]
id = 1

[hsv]
lower = [10, 50, 50]
upper = [20, 200, 200]

[osc]
host = "192.168.1.1"
port = 8000
)toml");

    auto result = media::load_config(path);

    REQUIRE(result.has_value() == true);

    const auto& cfg = result.value();
    CHECK(cfg.camera_id == 1);
    CHECK(cfg.osc_host == "192.168.1.1");
    CHECK(cfg.osc_port == 8000);

    // HSV 値の確認（OpenCV の Scalar は [0]=H, [1]=S, [2]=V）
    CHECK(cfg.hsv.lower[0] == doctest::Approx(10.0));
    CHECK(cfg.hsv.lower[1] == doctest::Approx(50.0));
    CHECK(cfg.hsv.upper[2] == doctest::Approx(200.0));
}

TEST_CASE("load_config: TOML 構文エラーは ConfigError を返す") {
    const std::string path = "/tmp/test_modern_cpp_bad.toml";
    write_temp_file(path, "invalid toml content [[[");

    auto result = media::load_config(path);

    CHECK(result.has_value() == false);
    // パースエラーのメッセージが含まれていること
    CHECK(result.error().message.find("パースエラー") != std::string::npos);
}

TEST_CASE("load_config: 一部のキーが欠落している場合はデフォルト値を使う") {
    const std::string path = "/tmp/test_modern_cpp_partial.toml";
    // camera セクションだけ指定、他はデフォルト
    write_temp_file(path, R"toml(
[camera]
id = 2
)toml");

    auto result = media::load_config(path);

    REQUIRE(result.has_value() == true);

    const auto& cfg = result.value();
    CHECK(cfg.camera_id == 2);
    // デフォルト値が使われる
    CHECK(cfg.osc_host == "127.0.0.1");
    CHECK(cfg.osc_port == 9000);
}

TEST_CASE("make_default_config: デフォルト設定が返る") {
    auto cfg = media::make_default_config();
    CHECK(cfg.camera_id == 0);
    CHECK(cfg.osc_host == "127.0.0.1");
    CHECK(cfg.osc_port == 9000);
}

TEST_CASE("std::expected: value_or パターン") {
    // std::expected には value_or がない（std::optional と違う）が、
    // 同様のパターンを示す教育テスト
    auto result = media::load_config("/tmp/nonexistent.toml");

    // エラー時にデフォルト設定を使う
    media::Config cfg = result.value_or(media::make_default_config());
    CHECK(cfg.camera_id == 0); // デフォルト値
}
