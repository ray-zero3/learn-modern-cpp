// ============================================================
// src/config.cpp
// ------------------------------------------------------------
// Config 読み込みの実装。toml++ を使う。
//
// toml++ の基本的な使い方:
//   auto tbl = toml::parse_file("config.toml");
//   int val = tbl["section"]["key"].value_or(default_val);
// ============================================================
#include "media/config.hpp"

// toml++ ヘッダ（ヘッダオンリーライブラリ）
// TOML_HEADER_ONLY=1 の場合は単一ヘッダで完結する。
#include <toml++/toml.hpp>

#include <fstream>
#include <iostream>

namespace media {

// ============================================================
// make_default_config
// ============================================================
Config make_default_config() {
    return Config{};
}

// ============================================================
// load_config: TOML ファイルを読み込む
// ============================================================
std::expected<Config, ConfigError> load_config(std::string_view path) {
    // -------------------------------------------------------
    // ファイル存在チェック
    // -------------------------------------------------------
    {
        // most vexing parse を避けるため、変数で一度保持する。
        // std::ifstream file(std::string(path)) は「関数宣言」として解釈される場合がある。
        std::string path_str(path);
        std::ifstream file(path_str);
        if (!file.good()) {
            return std::unexpected(ConfigError{
                "設定ファイルが見つかりません: " + path_str
            });
        }
    }

    // -------------------------------------------------------
    // TOML パース
    // -------------------------------------------------------
    // toml::parse_file は例外を投げる可能性がある。
    // std::expected を返すために try-catch でラップする。
    // 例外→結果型の変換の典型パターン。
    toml::table tbl;
    try {
        tbl = toml::parse_file(std::string(path));
    } catch (const toml::parse_error& e) {
        return std::unexpected(ConfigError{
            "TOML パースエラー: " + std::string(e.description())
        });
    }

    // -------------------------------------------------------
    // 設定値の取り出し
    // -------------------------------------------------------
    Config config; // デフォルト値で初期化

    // [camera] セクション
    config.camera_id = tbl["camera"]["id"].value_or(0);

    // [hsv] セクション
    config.hsv.lower = cv::Scalar(
        tbl["hsv"]["lower"][0].value_or(100.0),
        tbl["hsv"]["lower"][1].value_or(80.0),
        tbl["hsv"]["lower"][2].value_or(80.0)
    );
    config.hsv.upper = cv::Scalar(
        tbl["hsv"]["upper"][0].value_or(130.0),
        tbl["hsv"]["upper"][1].value_or(255.0),
        tbl["hsv"]["upper"][2].value_or(255.0)
    );

    // [osc] セクション
    config.osc_host = tbl["osc"]["host"].value_or(std::string("127.0.0.1"));
    config.osc_port = tbl["osc"]["port"].value_or(9000);

    return config;
}

} // namespace media
