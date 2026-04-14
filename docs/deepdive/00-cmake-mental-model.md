# 深掘り 00: CMake の2段階実行メンタルモデル

## TL;DR

- CMake は **「Configure → Generate → Build」** の 3 段階で動く。うち最初の2つが「CMakeの仕事」、最後は実際のコンパイラ/リンカの仕事。
- `CMakeLists.txt` は**スクリプト**（宣言的ではない）。上から順に「コマンド」として実行される。
- ターゲット（`add_library` / `add_executable`）を作って、それに `target_xxx` で性質を盛り付けていく、のがモダン CMake の作法。
- ディレクトリ全体に効くグローバル命令（`include_directories(...)`, `link_libraries(...)` の直接呼び出し等）は**避ける**。

## 3段階

```
CMakeLists.txt ──(configure)──► 内部モデル ──(generate)──► build.ninja / Makefile ──(build)──► .o/.a/実行ファイル
           ↑                                                       ↑
     cmake -S . -B build                                       cmake --build build
```

| フェーズ | 誰がやる | 何をする |
|---|---|---|
| Configure | `cmake -S ... -B ...` | `CMakeLists.txt` を上から実行、ターゲット情報を内部モデルに登録 |
| Generate | 同上（続けて） | 内部モデルを元に Ninja / Make / Xcode 用のビルドスクリプトを生成 |
| Build | `cmake --build build` | 生成されたスクリプトを使って実際のコンパイル |

最初の2つをまとめて「configure 時」と呼ぶことが多い。

## 「CMakeLists.txt はスクリプト」という罠

JSON や TOML のような宣言形式ではありません。**上から順に実行される**。なので:

```cmake
add_executable(foo main.cpp)
target_link_libraries(foo PRIVATE bar)  # ← ここで bar が定義済みでないとエラー
add_library(bar ...)
```

この書き順はダメ。`bar` を先に作ってから `foo` に link する。

## ターゲット中心モデル

モダン CMake の核心は「**ターゲット単位で性質を宣言する**」ことです。

```cmake
add_library(chapter00_ref_greet src/greeting.cpp)
target_include_directories(chapter00_ref_greet PUBLIC include)
target_compile_features(chapter00_ref_greet PUBLIC cxx_std_20)
```

ここで重要なのは **PUBLIC / PRIVATE / INTERFACE**:

```
      自分自身のコンパイルに使う ?    リンクしてきた相手にも伝わる ?
PRIVATE    ○                                  ×
PUBLIC     ○                                  ○
INTERFACE  ×                                  ○
```

### 例

`chapter00_ref_greet` が `include/greet/greeting.hpp` を公開する場合:

```cmake
target_include_directories(chapter00_ref_greet PUBLIC include)
# このライブラリを target_link_libraries した相手は、自動で `-I include` が付く
```

もし `greeting.cpp` の中だけで使う内部ヘッダの場所なら:

```cmake
target_include_directories(chapter00_ref_greet PRIVATE src)
# リンクしてきた相手には伝わらない
```

「ヘッダオンリーライブラリ」（.cpp がない）は `add_library(foo INTERFACE)` で作って `target_include_directories(foo INTERFACE include)` する。

## `FetchContent` — 依存取得

TS の `npm install` に近いですが、実装は**ソースを取得してサブプロジェクトとして自分のビルドに組み込む**という大胆な方式です。

```cmake
include(FetchContent)
FetchContent_Declare(doctest GIT_REPOSITORY ... GIT_TAG v2.4.11)
FetchContent_MakeAvailable(doctest)
# ここで doctest::doctest ターゲットが使えるようになる
```

- 取得先は `build/_deps/doctest-src/`
- 相手の `CMakeLists.txt` が `add_subdirectory` 相当で読まれ、`doctest::doctest` などのターゲットが現在のスコープに現れる
- ネット越しクローンが入るので、初回 configure が少し遅い（以降はキャッシュ）

代替として **`find_package`** があり、これはシステムにインストール済みの依存を探します（`brew install opencv` した OpenCV はこちら）。`FetchContent` は「小さめ・ソースで扱いたい」、`find_package` は「ネイティブ依存・既に配布されている」、という使い分けが多いです。

## よくある落とし穴

### 1. キャッシュ破損

configure が変な状態になったら `build/` を消して再 configure。TS でいう `rm -rf node_modules && npm install` に近い。

```bash
rm -rf build && cmake -S . -B build ...
```

### 2. ターゲット名のスコープ

CMake のターゲット名は**グローバル**です。名前空間の概念がないため、他の章と衝突しないよう接頭辞を付けています（`chapter00_ref_greet` / `chapter00_ws_greet`）。

### 3. ディレクトリのグローバル命令は避ける

`include_directories(...)` や `link_libraries(...)` を直接呼ぶと、そのディレクトリ以下の全ターゲットに波及します。一見便利ですが、どこで何が効いているか追えなくなる。必ず `target_include_directories(<target> ...)` のほうを使う。

### 4. `target_link_libraries` は「依存宣言」

「ただリンクする」というより「このターゲットは xxx に依存する」という宣言。依存先の include パスやコンパイルオプションまで伝搬（PUBLIC なら）。なので多くの場合、`target_include_directories` を手書きせずとも依存ターゲット経由で勝手に通ることがあります。

## まとめ

- CMake は Configure → Generate → Build の3段階
- `CMakeLists.txt` は**スクリプト**。書き順に依存する
- **ターゲット + `target_xxx`** で性質を宣言する
- **PUBLIC / PRIVATE / INTERFACE** は「使用側への伝搬」を制御する
- `FetchContent` はソース取得、`find_package` はシステム依存
