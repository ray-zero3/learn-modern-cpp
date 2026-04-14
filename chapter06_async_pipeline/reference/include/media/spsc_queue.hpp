// ============================================================
// media/spsc_queue.hpp
// ------------------------------------------------------------
// Single-Producer Single-Consumer (SPSC) リングバッファキュー。
// ヘッダオンリー実装。std::atomic でスレッドセーフ。
//
// この章で学ぶ主要概念:
//   - std::atomic<T>: 不可分操作（アトミック変数）
//   - memory_order: メモリオーダーリング制約
//   - lock-free: mutex を使わずにスレッドセーフを実現
//
// TS 対比:
//   TS は基本的にシングルスレッドなので、この問題は存在しない。
//   Worker threads / SharedArrayBuffer を使う場合は類似の問題が起きる。
//   C++ では明示的なメモリモデルの知識が必要。
//
// 設計:
//   - 固定容量 N のリングバッファ
//   - producer が head に push, consumer が tail から pop
//   - head/tail を atomic で管理してロックフリーを実現
// ============================================================
#pragma once

#include <array>
#include <atomic>
#include <optional>

namespace media {

// ============================================================
// SpscQueue<T, N> — SPSC リングバッファキュー
// ============================================================
template <typename T, std::size_t N>
class SpscQueue {
    // キュー容量は 2 以上である必要がある
    static_assert(N >= 2, "SpscQueue: N は 2 以上にしてください");

public:
    SpscQueue()
        : head_(0), tail_(0) {}

    // コピー/ムーブ禁止（atomic は移動不可）
    SpscQueue(const SpscQueue&)            = delete;
    SpscQueue& operator=(const SpscQueue&) = delete;
    SpscQueue(SpscQueue&&)                 = delete;
    SpscQueue& operator=(SpscQueue&&)      = delete;

    // -------------------------------------------------------
    // try_push: キューに値を追加する（producer スレッドから呼ぶ）
    // -------------------------------------------------------
    // キューが満杯の場合は false を返す（ノンブロッキング）。
    //
    // memory_order の意味:
    //   acquire/release ペアで「head の書き込みは tail の読み込みより後に見える」を保証。
    //   これがないと、コンパイラや CPU の命令リオーダーで tail を
    //   古い値で読む可能性がある（= データ競合 / UB）。
    [[nodiscard]] bool try_push(const T& val) {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t next = (head + 1) % N;

        // キューが満杯かチェック（tail は consumer が更新する）
        if (next == tail_.load(std::memory_order_acquire)) {
            return false; // 満杯
        }

        buffer_[head] = val;
        // head を更新（release: この書き込みが consumer に見えるようにする）
        head_.store(next, std::memory_order_release);
        return true;
    }

    // ムーブセマンティクス版 push
    [[nodiscard]] bool try_push(T&& val) {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t next = (head + 1) % N;

        if (next == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[head] = std::move(val);
        head_.store(next, std::memory_order_release);
        return true;
    }

    // -------------------------------------------------------
    // try_pop: キューから値を取り出す（consumer スレッドから呼ぶ）
    // -------------------------------------------------------
    // キューが空の場合は nullopt を返す（ノンブロッキング）。
    [[nodiscard]] std::optional<T> try_pop() {
        const std::size_t tail = tail_.load(std::memory_order_relaxed);

        // キューが空かチェック（head は producer が更新する）
        if (tail == head_.load(std::memory_order_acquire)) {
            return std::nullopt; // 空
        }

        T val = std::move(buffer_[tail]);
        const std::size_t next = (tail + 1) % N;
        tail_.store(next, std::memory_order_release);
        return val;
    }

    // -------------------------------------------------------
    // キューが空かどうかを確認（近似値）
    // -------------------------------------------------------
    // スレッドセーフだが、確認直後に状態が変わる可能性がある。
    [[nodiscard]] bool empty() const {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    // キューの容量を返す（最大 N-1 要素を格納できる）
    [[nodiscard]] static constexpr std::size_t capacity() { return N - 1; }

private:
    // head: producer が書き込む位置（producer のみが更新）
    // tail: consumer が読み出す位置（consumer のみが更新）
    //
    // キャッシュライン分離のため padding を挿入すると理想的だが、
    // 教育目的なのでシンプルに実装する。
    alignas(64) std::atomic<std::size_t> head_; // producer 側
    alignas(64) std::atomic<std::size_t> tail_; // consumer 側

    // 固定サイズのリングバッファ（スタック or BSS セクション）
    // std::array<T, N>: コンパイル時に決まるサイズ（ヒープ割り当て不要）
    // TS でいう Tuple<T, N> 的な概念（固定長）
    std::array<T, N> buffer_{};
};

} // namespace media
