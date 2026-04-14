// ============================================================
// media/config.hpp
// ------------------------------------------------------------
// TOML 設定ファイルの読み込みと Config 構造体の定義。
//
// この章で学ぶ主要概念:
//   - std::expected<T, E>: エラーを値として返す（C++23）
//   - toml++ ライブラリの使い方
//   - 結果型パターン（例外を使わないエラー伝搬）
//
// TS 対比:
//   - std::expected<Config, ConfigError> ≈ Result<Config, ConfigError>
//     (Rust の Result / TypeScript の Either モナド的パターン)
//   - try-catch を使わずにエラーを「値」として表現する設計
//
// 深掘り: docs/deepdive/07-expected-vs-exceptions.md
// ============================================================
#pragma once

#include "media/color_tracker.hpp" // HsvRange

#include <string>
#include <string_view>

// std::expected の可用性チェック
// Clang 21 では <expected> が使えるため標準版を使う
#include <expected>

namespace media {

// ============================================================
// Config — アプリケーション設定
// ------------------------------------------------------------
// TOML ファイルから読み込む設定値をまとめた構造体。
// ============================================================
struct Config {
    int camera_id{0};           // カメラデバイスID
    HsvRange hsv{               // HSV 色域（デフォルト: 青系）
        cv::Scalar(100, 80, 80),
        cv::Scalar(130, 255, 255)
    };
    std::string osc_host{"127.0.0.1"}; // OSC 送信先ホスト
    int osc_port{9000};                // OSC 送信先ポート
};

// ============================================================
// ConfigError — 設定読み込みエラー
// ============================================================
struct ConfigError {
    std::string message; // エラーメッセージ
};

// ============================================================
// load_config: TOML ファイルを読み込んで Config を返す
// ============================================================
// [[nodiscard]]: 戻り値を無視したらコンパイル警告
// std::expected<Config, ConfigError>:
//   成功時は Config を保持
//   失敗時は ConfigError を保持
//   呼び出し側は has_value() / error() でどちらかを確認
//
// TS 対比:
//   function loadConfig(path: string): Config | ConfigError
//   ではなく、成功/失敗を型で明確に区別する。
//   Rust の Result<Config, ConfigError> に相当。
[[nodiscard]] std::expected<Config, ConfigError> load_config(std::string_view path);

// ============================================================
// make_default_config: デフォルト設定を返す
// ============================================================
[[nodiscard]] Config make_default_config();

} // namespace media
