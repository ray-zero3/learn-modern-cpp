# 05: スタック vs ヒープ、メモリレイアウトとキャッシュ局所性

## TL;DR

| 場所 | 確保コスト | アクセス速度 | 寿命管理 |
|------|----------|------------|---------|
| スタック | 0（SP調整のみ） | 最速（L1ほぼ確実） | 自動（スコープ退出で即解放） |
| ヒープ | `malloc` + OS呼び出し | 遅い（キャッシュミスしやすい） | 手動 or スマートポインタ |

## TypeScript との対比

TypeScript（Node.js/V8）では、プリミティブ型はスタック、オブジェクトはヒープに置かれる。ただしこの区別はエンジンが隠蔽しており、開発者が意識することはほぼない。

C++ では、**どちらに置くかを開発者が選択する**：

```cpp
// スタック: `}` で自動破棄
cv::Mat frame_small(64, 64, CV_8UC3);          // ヘッダ(~100B)はスタック
                                                 // 画素データはヒープ (cv::Mat の内部)

// ヒープ: unique_ptr で寿命を管理
auto frame_heap = std::make_unique<cv::Mat>(64, 64, CV_8UC3);

// std::array: 要素全体がスタックに乗る
std::array<float, 16> tiny_buf;                 // 64B — スタックが最適

// std::vector: ヘッダはスタック、要素はヒープ
std::vector<float> dyn_buf(count);              // count が動的
```

## スタックのしくみ

```
高アドレス
┌──────────────┐
│  前フレーム  │
├──────────────┤ ← 関数呼び出し時の SP (Stack Pointer)
│ ローカル変数 │
│  tiny_buf[0] │  ← SP - 64
│  tiny_buf[1] │
│     ...      │
├──────────────┤ ← 関数が返ると SP を戻すだけで "解放" 完了
低アドレス
```

- SP（スタックポインタ）を動かすだけなので **コストはほぼ 0**。
- `malloc` は空き領域の探索、メタデータ更新など数十〜数百 ns かかることもある。

## キャッシュ局所性（Cache Locality）

現代 CPU のキャッシュは 64B（キャッシュライン）単位で動く。

### `std::vector<Particle>` vs `std::vector<std::unique_ptr<Particle>>`

```
// NG: ポインタ配列 (Pointer-of-Structures, PoS)
// メモリ上の配置:
// vec[0] → 0xA000  vec[1] → 0xF200  vec[2] → 0x3100
//         ↑                 ↑                 ↑
//         別々のキャッシュラインに散在 → キャッシュミスの嵐
std::vector<std::unique_ptr<Particle>> particles;

// OK: 値配列 (Array-of-Structures, AoS)
// メモリ上の配置:
// [p0_x][p0_y][p0_z][p1_x][p1_y][p1_z]... — 連続領域
// → 1 回のキャッシュラインフェッチで複数要素をカバー
std::vector<Particle> particles;
```

### `std::array` がループに強い理由

```cpp
// このループは CPU の prefetcher が先読みできる（連続アドレス）
std::array<float, 17> scores;
for (float s : scores) { /* ... */ }

// unique_ptr 配列は prefetcher が効きにくい
std::array<std::unique_ptr<float>, 17> scores;
for (auto& p : scores) { float s = *p; /* ... */ }
```

## Chapter 5 での応用

### `OpticalFlowDetector` の `prev_gray_`

```cpp
// include/media/optical_flow.hpp
class OpticalFlowDetector {
    std::optional<cv::Mat> prev_gray_;  // 前フレームのグレー画像
    // ...
};
```

- `cv::Mat` のヘッダ（約 100B）はクラス内部（スタックまたはヒープ上のオブジェクト領域）に格納される。
- 画素データ（例: 1280×720 = 921,600B）は **`cv::Mat` が内部でヒープ確保**。
- `std::optional<cv::Mat>` を使うことで、初期状態（フレーム未保持）を `nullopt` で表現できる。

### `cv::Mat` のコピーコスト

```cpp
// 浅いコピー（参照カウントを増やすだけ — O(1)）:
cv::Mat a = frame;

// 深いコピー（画素データも複製 — O(N×M)）:
cv::Mat b = frame.clone();

// OpticalFlowDetector ではフレームを "保存" するので clone が必要
prev_gray_ = current_gray.clone();
```

## 実測してみよう

```cpp
#include <benchmark/benchmark.h>  // Google Benchmark (任意)

// キャッシュフレンドリー: 連続アクセス
static void BM_Sequential(benchmark::State& s) {
    std::vector<int> v(1'000'000, 0);
    for (auto _ : s) {
        for (int& x : v) x += 1;
    }
}

// キャッシュアンフレンドリー: ストライドアクセス
static void BM_Strided(benchmark::State& s) {
    std::vector<int> v(1'000'000, 0);
    for (auto _ : s) {
        for (std::size_t i = 0; i < v.size(); i += 16) v[i] += 1;
    }
}
```

典型的な結果: Sequential は Strided の 4〜8 倍速い。

## まとめ

- **小さく寿命が短いデータ** → スタック (`std::array`, ローカル変数)
- **大きいデータや動的サイズ** → ヒープ (`std::vector`, `std::unique_ptr`)
- **性能が重要なループ** → 連続メモリ (`std::vector<T>` の値配列) を選ぶ
- **TS との最大の差分**: C++ は「どこに置くか」を意識して設計しなければならないが、そのぶん最適化の余地が大きい。
