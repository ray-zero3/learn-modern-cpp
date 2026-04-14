// ============================================================
// tests/test_pipeline.cpp
// ------------------------------------------------------------
// SPSC キューとパイプラインの単体テスト。
// ============================================================
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "media/spsc_queue.hpp"

#include <thread>
#include <chrono>

TEST_CASE("SpscQueue: push/pop の基本動作") {
    media::SpscQueue<int, 8> queue;

    // 空の状態
    CHECK(queue.empty() == true);
    CHECK(queue.try_pop().has_value() == false);

    // push
    CHECK(queue.try_push(42) == true);
    CHECK(queue.empty() == false);

    // pop
    auto val = queue.try_pop();
    REQUIRE(val.has_value() == true);
    CHECK(*val == 42);
    CHECK(queue.empty() == true);
}

TEST_CASE("SpscQueue: 満杯の場合は push が false を返す") {
    // N=4 の場合、最大 N-1=3 要素を格納できる
    media::SpscQueue<int, 4> queue;

    CHECK(queue.try_push(1) == true);
    CHECK(queue.try_push(2) == true);
    CHECK(queue.try_push(3) == true);
    // 4番目は満杯になるので false
    CHECK(queue.try_push(4) == false);

    // pop して1つ空ける
    [[maybe_unused]] auto _ = queue.try_pop();
    // 今度は push できる
    CHECK(queue.try_push(4) == true);
}

TEST_CASE("SpscQueue: FIFO 順序が保たれる") {
    media::SpscQueue<int, 8> queue;

    // FIFO テスト用 push（戻り値は確認不要なので maybe_unused を使う）
    [[maybe_unused]] bool r1 = queue.try_push(1);
    [[maybe_unused]] bool r2 = queue.try_push(2);
    [[maybe_unused]] bool r3 = queue.try_push(3);

    CHECK(*queue.try_pop() == 1);
    CHECK(*queue.try_pop() == 2);
    CHECK(*queue.try_pop() == 3);
    CHECK(queue.try_pop().has_value() == false);
}

TEST_CASE("SpscQueue: マルチスレッドで正しく動作する") {
    // SPSC の基本テスト: 1プロデューサー、1コンシューマー
    media::SpscQueue<int, 64> queue;
    constexpr int kCount = 50;

    int received_count = 0;
    int received_sum   = 0;

    // コンシューマースレッド
    std::thread consumer([&]() {
        while (received_count < kCount) {
            auto val = queue.try_pop();
            if (val.has_value()) {
                received_sum += *val;
                ++received_count;
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
    });

    // プロデューサー（メインスレッド）
    for (int i = 0; i < kCount; ++i) {
        while (!queue.try_push(i)) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    consumer.join();

    // 0+1+2+...+49 = 1225
    CHECK(received_sum == 1225);
    CHECK(received_count == kCount);
}

TEST_CASE("SpscQueue: capacity が正しい値を返す") {
    // N=8 の場合、capacity() = 7
    CHECK(media::SpscQueue<int, 8>::capacity() == 7);
    CHECK(media::SpscQueue<int, 4>::capacity() == 3);
}
