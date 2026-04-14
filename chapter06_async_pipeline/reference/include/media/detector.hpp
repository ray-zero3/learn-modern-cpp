// ============================================================
// media/detector.hpp
// ------------------------------------------------------------
// Detector concept と DetectorOutput 型の定義。
//
// この章で学ぶ主要概念:
//   - C++20 concept: 型の「要件」をコンパイル時に検査する仕組み
//   - requires 節: concept の条件を記述する構文
//   - 静的ディスパッチ vs 動的ディスパッチ（仮想関数との比較）
//
// TS 対比:
//   C++ の concept は TS の interface / structural typing に近い。
//   TS は「プロパティが一致すれば OK」（structural typing）だが、
//   C++ の concept は「式が有効かどうか」をコンパイル時にチェックする。
//   より明示的で、失敗時のエラーメッセージも分かりやすい。
//
// 深掘り: docs/deepdive/04-concepts-vs-virtual.md
// ============================================================
#pragma once

#include "media/color_tracker.hpp" // TrackResult を使う

#include <optional>
#include <variant>

#include <opencv2/core.hpp>

namespace media {

// DetectorOutput, TrackResult, FlowVec は color_tracker.hpp で定義済み。
// detector.hpp は color_tracker.hpp をインクルードして使う。

// ============================================================
// Detector concept
// ------------------------------------------------------------
// 型 T が Detector を満たすには:
//   t.process(const cv::Mat& frame) が
//   std::optional<DetectorOutput> を返せること。
//
// TS interface に相当する宣言:
//   interface Detector {
//     process(frame: Mat): DetectorOutput | null;
//   }
//
// TS の structural typing との違い:
//   TS は「型の形状が一致するか」を確認するが、
//   C++ の concept は「その式がコンパイルできるか」を確認する。
//   戻り値の型変換可能性も concept の条件に含められる。
// ============================================================
template <typename T>
concept Detector = requires(T t, const cv::Mat& frame) {
    // t.process(frame) が呼べること、かつ
    // その戻り値が std::optional<DetectorOutput> に変換できること。
    { t.process(frame) } -> std::same_as<std::optional<DetectorOutput>>;
};

// ============================================================
// static_assert でコンパイル時に concept 充足を確認する
// ------------------------------------------------------------
// ColorTracker が Detector を満たすかどうかをコンパイル時にチェック。
// 満たさない場合はエラーメッセージ付きでコンパイル失敗する。
// (このファイルより後で ColorTracker に process() が定義される必要がある)
// ============================================================
// 注: 前方宣言のため、ColorTracker の定義がない段階では確認できない。
// 実際の static_assert はテストファイルに置く。

} // namespace media
