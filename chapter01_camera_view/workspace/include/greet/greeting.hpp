// ============================================================
// greet/greeting.hpp
// ------------------------------------------------------------
// ヘッダ (= 宣言側)。このファイルは「APIの見取り図」。
// 実装は src/greeting.cpp にある。
//
// TS対比:
//   ヘッダ = .d.ts に近いが、テンプレート等は定義もここに書く。
//   `#pragma once` は TS でいう「このモジュールは1度だけ評価」
//   の保証に似ているが、目的はもっと低レベル: 同じヘッダが
//   同一コンパイル単位で複数回展開されるのを防ぐ。
//   (C++ のコンパイルは、ヘッダをテキストとしてその場に貼り込む)
// ============================================================
#pragma once

#include <string>
#include <string_view>

namespace greet {

// TSで言う `export function greet(name: string): string` に相当。
// `std::string_view` は「所有しない文字列ビュー」。`const string&`
// の代替として関数引数で使うのがモダン C++ の定番。
// 呼び出し側は string / const char* / 部分文字列 いずれも渡せる。
[[nodiscard]] std::string greet(std::string_view name);

// 時刻帯別のバリアント。戻り値の所有は呼び出し側に移るため、
// std::string（= 所有する文字列）で返す。
enum class TimeOfDay { Morning, Afternoon, Evening, Night };

[[nodiscard]] std::string greet_at(std::string_view name, TimeOfDay tod);

// 0-23 の時刻から TimeOfDay を決める純粋関数。テストしやすい。
[[nodiscard]] TimeOfDay classify_hour(int hour_0_23);

} // namespace greet
