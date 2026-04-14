// ============================================================
// tests/test_ui_state.cpp
// ------------------------------------------------------------
// UiState のテスト（GUI を開かずにスレッドセーフな状態管理をテスト）。
// ============================================================
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "media/ui_state.hpp"

TEST_CASE("UiState: デフォルト構築後の初期値") {
    media::UiState state;
    // FPS の初期値は 0
    CHECK(state.get_fps() == doctest::Approx(0.0F));
    // ログは空
    CHECK(state.get_logs().empty() == true);
}

TEST_CASE("UiState: HSV 範囲の取得/設定") {
    media::UiState state;

    media::HsvRange new_range{
        cv::Scalar(10, 50, 50),
        cv::Scalar(30, 200, 200)
    };
    state.set_hsv_range(new_range);

    auto range = state.get_hsv_range();
    CHECK(range.lower[0] == doctest::Approx(10.0));
    CHECK(range.upper[2] == doctest::Approx(200.0));
}

TEST_CASE("UiState: FPS の設定/取得") {
    media::UiState state;
    state.set_fps(30.0F);
    CHECK(state.get_fps() == doctest::Approx(30.0F));
}

TEST_CASE("UiState: ログの追加/取得/クリア") {
    media::UiState state;

    state.add_log("メッセージ 1");
    state.add_log("メッセージ 2");

    auto logs = state.get_logs();
    CHECK(logs.size() == 2);
    CHECK(logs[0] == "メッセージ 1");
    CHECK(logs[1] == "メッセージ 2");

    state.clear_logs();
    CHECK(state.get_logs().empty() == true);
}

TEST_CASE("UiState: 変更通知コールバックが呼ばれる") {
    media::UiState state;

    int call_count = 0;
    state.on_change([&call_count]() { ++call_count; });

    // HSV 範囲を変更するとコールバックが呼ばれる
    state.set_hsv_range(media::HsvRange{
        cv::Scalar(10, 50, 50),
        cv::Scalar(30, 200, 200)
    });

    CHECK(call_count == 1);

    // もう一度変更
    state.set_hsv_range(media::HsvRange{
        cv::Scalar(50, 80, 80),
        cv::Scalar(70, 255, 255)
    });

    CHECK(call_count == 2);
}

TEST_CASE("UiState: ログが最大件数を超えたら古いものから削除") {
    media::UiState state;

    // 100件を超えるログを追加
    for (int i = 0; i < 110; ++i) {
        state.add_log("log " + std::to_string(i));
    }

    auto logs = state.get_logs();
    // 最大 100件
    CHECK(logs.size() == 100);
    // 最新のものが残っている
    CHECK(logs.back() == "log 109");
}
