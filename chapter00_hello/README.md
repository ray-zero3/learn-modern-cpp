# Chapter 0 — C++プロジェクトの 0→1

## TL;DR

この章のゴールは **挨拶ライブラリ + 実行ファイル + doctestのテスト** を、CMake で一発ビルド・テストできる最小プロジェクトを手で作ることです。
C++ 固有の「なぜヘッダが必要なのか」「なぜ CMake なのか」を理解します。

ゴール:

- [ ] `cmake -S . -B build ... && cmake --build build` が成功する
- [ ] `ctest --test-dir build --output-on-failure` が **reference 側は全 PASS** になる
- [ ] `workspace/src/greeting.cpp` の TODO を実装して、**workspace 側も全 PASS** にできる
- [ ] 「ヘッダはテキスト展開」「コンパイル単位ごとの独立コンパイル → リンカ」が人に説明できる

---

## この章の構成

```
chapter00_hello/
├── CMakeLists.txt          # reference と workspace を束ねる（workspace が空なら自動スキップ）
├── reference/              # お手本（完成コード。読む用）
│   ├── CMakeLists.txt
│   ├── include/greet/greeting.hpp  (宣言)
│   ├── src/greeting.cpp            (定義)
│   ├── app/main.cpp                (実行ファイル)
│   └── tests/test_greeting.cpp     (doctest)
└── workspace/              # ★開始時点では空★（README のみ）
    └── README.md
```

**この章は 0→1 がテーマなので、workspace は開始時点で空です。**
`reference/` を「読む」ことで構成と意図を理解し、`workspace/` で同じものを**自分で一から作る**のがゴール。

各章のルール（今後も共通）:

- `reference/` … 完成コード。読み物として、および動作確認済みのお手本として提供
- `workspace/` … **その章の開始状態**（前章の完了物）。この章の作業を通じて `reference/` 相当まで育てる

---

## TypeScript との対比（この章の最重要表）

| | TypeScript | この章の C++ |
|---|---|---|
| プロジェクト設定 | `package.json` + `tsconfig.json` | `CMakeLists.txt` |
| 依存取得 | `npm install` | `FetchContent_Declare` / `FetchContent_MakeAvailable` |
| 依存定義の場所 | `dependencies` フィールド | `cmake/Dependencies.cmake` |
| モジュール境界 | `import` / `export` | `#include` + **リンカ** |
| 型宣言の別置き | `.d.ts`（任意） | `.hpp`（関数・クラスは**ほぼ必須**） |
| 実行ファイル定義 | `"bin"` / `"main"` | `add_executable` |
| ライブラリ定義 | publish 先 package | `add_library` |
| 単体テスト | vitest / jest | doctest (今回) |

**本質的に違うところ**: TS は 1 プロセスの tsc が全ファイルを舐めて型解決するのに対し、C++ は **ファイル単位で独立コンパイル → 生成された `.o` をリンカが繋ぐ** というモデルです。この違いが「ヘッダ」「ODR」「リンクエラー」など C++ の多くの概念を生みます。詳しくは [docs/deepdive/00-header-vs-source.md](../docs/deepdive/00-header-vs-source.md)。

---

## Step 1: ビルドシステムを理解する

まずビルド設定から読みます。コードを書くより先にここを読むのが C++ 学習の急がば回れ。

### ルート [`CMakeLists.txt`](../CMakeLists.txt)

ポイントだけ抜粋:

```cmake
cmake_minimum_required(VERSION 3.25)
project(modern_cpp ... LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # C++17 へのフォールバック禁止
set(CMAKE_CXX_EXTENSIONS OFF)          # GNU 拡張禁止（移植性を確保）

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # clangd / clang-tidy 用
enable_testing()                       # ctest を有効化

include(Warnings)
include(Dependencies)                  # doctest を FetchContent

add_subdirectory(chapter00_hello)      # 章を追加
```

TS 的に読み替えると、ルート `tsconfig.json` で `target: "es2022"`, `strict: true` を宣言しているのと近い。

### Chapter 0 の [`reference/CMakeLists.txt`](reference/CMakeLists.txt)

ここが一番勉強になる。3 つの CMake 「ターゲット」を作っています:

1. **`chapter00_ref_greet`** — `add_library` で作る静的ライブラリ。`greeting.cpp` をコンパイルして `.a` にする。
2. **`chapter00_ref_app`** — `add_executable` で作る実行ファイル。greet ライブラリをリンク。
3. **`chapter00_ref_tests`** — 同じくライブラリと doctest をリンクしたテストバイナリ。

それぞれに `target_link_libraries` / `target_include_directories` / `target_compile_features` を呼んで「このターゲットは何を要求するか」を宣言します。
「ターゲット中心で書く」= モダン CMake の鉄則。グローバル命令（`include_directories(...)` を直接書く等）は避けます。

#### PRIVATE / PUBLIC / INTERFACE の使い分け

これは TS にない概念なので要注意:

| 指定 | このターゲット自身で使う | リンクしてきた相手に伝搬 |
|---|---|---|
| `PRIVATE` | ○ | × |
| `PUBLIC` | ○ | ○ |
| `INTERFACE` | × | ○ |

`target_include_directories(chapter00_ref_greet PUBLIC include)` は「このライブラリ自身のコンパイル時にも、リンクする側のコンパイル時にも、`include/` をヘッダ探索パスに入れる」という意味です。
ヘッダを公開 API として露出させる場合は **PUBLIC**、実装詳細なら **PRIVATE** が基本。

---

## Step 2: ビルドと実行

初回セットアップ:

```bash
cmake -S . -B build \
  -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build
```

### 実行

```bash
# お手本のアプリ
./build/chapter00_hello/reference/chapter00_ref_app
./build/chapter00_hello/reference/chapter00_ref_app Alice    # 引数を渡す

# お手本のテスト（全 PASS するはず）
./build/chapter00_hello/reference/chapter00_ref_tests
```

### CTest から一括実行

```bash
ctest --test-dir build --output-on-failure
```

現時点では reference の 4 テストが登録されて全 PASS するのが正しい状態。workspace を作り始めると、そちらのテストも自動で登録されます（親 `CMakeLists.txt` が `workspace/CMakeLists.txt` の存在を検知して `add_subdirectory` する）。

### 特定のターゲットだけ再ビルド

```bash
cmake --build build --target chapter00_ref_tests
```

---

## Step 3: workspace を一から作る

ここが Chapter 0 の本番。`workspace/` にファイルを自分で作っていきます。
`reference/` を見ながらで OK ですが、**写経ではなく「なぜこう書くか」を考えながら**進めてください。

### 3.1 — ディレクトリと空ファイルの準備

以下のレイアウトに倣って、まずは空ファイルだけ作る。ここで**ディレクトリ設計の意図**を理解します:

```
workspace/
├── CMakeLists.txt
├── include/greet/greeting.hpp   # 公開 API（.hppは "宣言" を置く場所）
├── src/greeting.cpp              # 実装詳細
├── app/main.cpp                  # 実行ファイルのエントリ
└── tests/test_greeting.cpp       # doctest
```

`include/greet/...` と 1 段掘ってあるのは、`#include "greet/greeting.hpp"` と書けるようにして **誰が提供するヘッダか** を明示するためです（TS の scoped package `@greet/core` と同じ発想）。

### 3.2 — `CMakeLists.txt` を書く

最小要件:

1. `add_library` で `chapter00_ws_greet` を作る（`src/greeting.cpp` を含める）
2. `target_include_directories(... PUBLIC include)` でヘッダ探索パスを公開
3. `add_executable` で `chapter00_ws_app` を作り、ライブラリをリンク
4. `add_executable` で `chapter00_ws_tests` を作り、ライブラリ と `doctest::doctest` をリンク
5. `include(doctest)` と `doctest_discover_tests(...)` でテストを CTest に登録

**ヒント:** `reference/CMakeLists.txt` を読み、ターゲット名を `ref` → `ws` に付け替えるだけでも動きます。ただし、読みながら以下を自分で説明できる状態にしてから進むこと:

- `PRIVATE` と `PUBLIC` の違いは？
- `target_link_libraries` と `target_include_directories` の役割の違いは？
- `target_compile_features(... PUBLIC cxx_std_20)` を書く理由は？（ルートで `CMAKE_CXX_STANDARD=20` を指定してもなお書く価値はあるか？）

書いたらルートに戻って configure をやり直します:

```bash
cmake -S . -B build ...   # workspace/CMakeLists.txt を検知させるため再 configure
cmake --build build
```

### 3.3 — テストを先に書く（TDD の RED）

`tests/test_greeting.cpp` を書きます。仕様:

| 関数 | 入力 → 期待される出力 |
|---|---|
| `greet::greet("World")` | `"Hello, World!"` |
| `greet::greet("")` | `"Hello, !"`（空でも落ちない） |
| `greet::classify_hour(h)` | 5-11→Morning, 12-17→Afternoon, 18-21→Evening, それ以外→Night |
| `greet::greet_at("Alice", TimeOfDay::Morning)` | `"Good morning, Alice!"` |

テンプレはファイル先頭:

```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "greet/greeting.hpp"

TEST_CASE("...") { CHECK(...); }
```

この段階でビルドすると、ヘッダがまだ無いので**コンパイルエラー**になるはず。それが正しい RED。

### 3.4 — ヘッダと stub 実装で RED を明確にする

`include/greet/greeting.hpp` に関数**宣言**だけ、`src/greeting.cpp` に**空の実装**（`return {};`）を書く。
ビルドは通るがテストは **全失敗**する状態にします。

```cpp
// greeting.hpp
#pragma once
#include <string>
#include <string_view>
namespace greet {
    enum class TimeOfDay { Morning, Afternoon, Evening, Night };
    std::string greet(std::string_view name);
    TimeOfDay classify_hour(int hour_0_23);
    std::string greet_at(std::string_view name, TimeOfDay tod);
}
```

```bash
cmake --build build && ./build/chapter00_hello/workspace/chapter00_ws_tests
# → FAIL 多数。これが RED。
```

### 3.5 — 実装して GREEN にする

`src/greeting.cpp` を埋めて、テストが全通過するまで回す:

```bash
cmake --build build --target chapter00_ws_tests && \
  ./build/chapter00_hello/workspace/chapter00_ws_tests
```

実装のヒント:

- `greet`: `std::string` は `string_view` 引数を取る `append` / `+=` を持つ
- `classify_hour`: `if` で十分。`switch` は列挙型に使う
- `greet_at`: `switch (tod) { case TimeOfDay::Morning: ... }`

### 3.6 — アプリと動作確認

`app/main.cpp` も書き、:

```bash
./build/chapter00_hello/workspace/chapter00_ws_app Alice
```

で挨拶が表示されれば完成。

### 3.7 — reference と読み比べて "差分" を味わう

完成したら `diff -r reference/ workspace/` などで差を眺め、以下を自問:

- `reference/src/greeting.cpp` では `reserve()` を呼んでいる箇所がある。なぜ？
- `[[nodiscard]]` は何をする属性か？ つけると何が変わるか？
- `const std::string name = (argc > 1) ? ...` の `const` の意味は？ TS の `const` と何が違うか？

---

## Step 4: 深掘り

この章では以下の2本を深掘り記事にしています。コードを動かせた後、腰を据えて読んでください。

- [docs/deepdive/00-header-vs-source.md](../docs/deepdive/00-header-vs-source.md) — なぜヘッダが必要か／コンパイル単位／ODR
- [docs/deepdive/00-cmake-mental-model.md](../docs/deepdive/00-cmake-mental-model.md) — CMake の「2段階実行」メンタルモデル

---

## 達成チェックリスト

- [ ] ルートで `cmake -S . -B build ...` が通る
- [ ] `cmake --build build` が全ターゲットビルド成功する
- [ ] `ctest --test-dir build` で reference 側 4/4 PASS
- [ ] workspace の実装を埋めて、workspace 側も 4/4 PASS
- [ ] `#pragma once` が何を防いでいるか説明できる
- [ ] `.hpp` と `.cpp` を分ける利点を 1 つ挙げられる
- [ ] `target_link_libraries ... PUBLIC` と `PRIVATE` の違いを説明できる
- [ ] `add_library(... STATIC)` と `add_executable(...)` の成果物の違いを説明できる
- [ ] ODR（One Definition Rule）が何を言っているか自分の言葉で話せる

全部埋まったら次の Chapter 1（カメラ映像をウィンドウに表示）に進む合図です。
