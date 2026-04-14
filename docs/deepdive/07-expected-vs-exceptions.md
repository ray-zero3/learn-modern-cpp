# 07: std::expected vs 例外 — エラー設計の選択

## TL;DR

| 方式 | 性能 | 型安全 | 呼び出し元の強制力 | TS対応 |
|------|------|--------|----------------|--------|
| 例外 (`throw`) | ハッピーパス高速、失敗パス遅い | △（型は `std::exception` 派生） | なし（無視可能） | `throw` / `catch` |
| `std::expected<T,E>` (C++23) | 一定 | ◎（Error 型も静的） | ◎（値を使うには確認が必要） | TS の `Result<T,E>` |
| `std::optional<T>` | 一定 | △（理由不明） | ◎ | `T \| undefined` |

## TypeScript との対比

TypeScript では例外が主流で、型安全なエラーは `Result` 型パターンを手動で実装する：

```typescript
// TS: try-catch（例外ベース）
try {
    const config = loadConfig("config.toml");
} catch (e) {
    // e の型は unknown — 型安全でない
    console.error(e);
}

// TS: Result 型パターン（neverthrow などのライブラリ）
type Result<T, E> = { ok: true; value: T } | { ok: false; error: E };

const result = loadConfig("config.toml");
if (!result.ok) {
    console.error(result.error.message);
} else {
    use(result.value);
}
```

C++23 の `std::expected<T, E>` はこの Result 型を標準化したもの：

```cpp
// C++23: std::expected
#include <expected>

std::expected<Config, ConfigError> load_config(std::string_view path);

auto result = load_config("config.toml");
if (!result) {                          // operator bool() で確認
    std::cerr << result.error().message;
} else {
    use(result.value());
}

// value_or でデフォルト値
Config cfg = load_config("config.toml").value_or(make_default_config());
```

## 例外の仕組みとコスト

### ゼロコスト例外 (Zero-cost EH)

C++ の例外は「ハッピーパスにコストをかけない」設計：

```
通常実行:
  1. 例外テーブル（read-only セクション）を事前生成
  2. 実行時は throw が起きない限りオーバーヘッドなし
  
例外発生時:
  1. スタックを巻き戻し (stack unwinding)
  2. catch ブロックを例外テーブルで検索 → O(スタック深さ)
  3. デストラクタを順に呼ぶ
  → 通常 数μs〜数十μs の遅延
```

### 例外が不向きなケース

```cpp
// 毎フレーム呼ばれる処理で例外を使うと、失敗時に目立った遅延が出る
// カメラキャプチャ失敗はよくある「期待された失敗」なので例外は不向き
std::optional<cv::Mat> read_frame() noexcept;  // Chapter 1 の設計

// 設定ファイル読み込みは「起動時1回」なので例外でも許容できるが
// std::expected の方が型安全で呼び出し元に処理を強制できる
std::expected<Config, ConfigError> load_config(std::string_view path);  // Chapter 7
```

## `std::expected` のインターフェース

```cpp
#include <expected>  // C++23

// 成功値を返す
return Config{...};                          // 暗黙変換

// 失敗値を返す
return std::unexpected(ConfigError{"原因"});  // std::unexpected で包む

// 呼び出し側
auto result = load_config(path);

result.has_value();          // bool
result.value();              // T& (失敗時は throw std::bad_expected_access)
result.error();              // E& (成功時は UB — 確認してから使う)
result.value_or(default_v); // 失敗時はデフォルト値
result.and_then(f);         // 成功時に f(value) を実行、モナディックチェーン
result.or_else(f);          // 失敗時に f(error) を実行
```

### モナディックチェーン（C++23）

```cpp
// TS の Promise チェーンに相当
load_config(path)
    .and_then([](Config cfg) -> std::expected<App, ConfigError> {
        return App{cfg};
    })
    .or_else([](ConfigError err) -> std::expected<App, ConfigError> {
        std::cerr << "設定エラー: " << err.message << "\n";
        return App{make_default_config()};
    });
```

## Chapter 7 での実装

```cpp
// include/media/config.hpp
struct ConfigError {
    std::string message;
};

[[nodiscard]] std::expected<Config, ConfigError>
load_config(std::string_view path);

[[nodiscard]] Config make_default_config();
```

`[[nodiscard]]` を付けることで、返値を無視するとコンパイル警告が出る。TypeScript の `// @ts-expect-error` コメントに頼らずとも、静的に「使い忘れ」を検出できる。

## いつ例外を使い、いつ `std::expected` を使うか

```
┌─────────────────────────────────────────────┐
│ 例外が適切:                                   │
│   - コンストラクタのエラー（戻り値がない）      │
│   - 本当に予期しない失敗（バグ、OOM）           │
│   - スタック深い位置からトップレベルへの伝搬   │
├─────────────────────────────────────────────┤
│ std::expected が適切:                         │
│   - 「失敗はあり得る」と仕様に書いてある場合   │
│   - リアルタイム処理（レイテンシが重要）        │
│   - 呼び出し元にエラー処理を強制したい場合     │
│   - API 境界（他モジュールとのインターフェース）│
└─────────────────────────────────────────────┘
```

## まとめ

- `std::expected<T, E>` = TS の `Result<T, E>` を C++23 が標準化したもの。
- 例外はハッピーパスが速いが、エラー型の静的保証がなく、無視も可能。
- `[[nodiscard]]` + `std::expected` の組み合わせが、安全なエラー設計の基本形。
- C++23 が使えない環境では `tl::expected`（ヘッダオンリー）が互換実装として使える。
