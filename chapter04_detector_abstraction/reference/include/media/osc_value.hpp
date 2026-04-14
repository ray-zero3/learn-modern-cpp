// ============================================================
// media/osc_value.hpp
// ------------------------------------------------------------
// OSC メッセージの値型を std::variant で表現する。
//
// OSC (Open Sound Control) プロトコルは以下の型をサポート:
//   int32, float32, string, bool (拡張), blob など
//
// この章で学ぶ主要概念:
//   std::variant<T1, T2, ...> : 複数型のうち「どれか1つ」を保持する型
//   std::visit               : variant の型によって処理を振り分ける
//
// TS 対比:
//   OscValue = int | float | string | bool に相当するが、
//   C++ では実行時型判定が型安全に行える（TS の discriminated union）。
//   TS の switch(typeof value) に相当するのが std::visit + overload。
//
// 深掘り: docs/deepdive/03-variant-vs-union.md
// ============================================================
#pragma once

#include <string>
#include <variant>

namespace media {

// ============================================================
// OscValue — OSC 引数の型
// ------------------------------------------------------------
// std::variant<int, float, std::string, bool>
//   これが TS の `number | string | boolean` に相当。
//   ただし C++ の variant は「どの型が入っているか」を
//   型安全に管理し、間違った型でアクセスしようとすると
//   例外か（std::get）、コンパイルエラーになる。
// ============================================================
using OscValue = std::variant<int, float, std::string, bool>;

} // namespace media
