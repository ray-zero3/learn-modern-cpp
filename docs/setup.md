# 環境構築（macOS）

## 前提（既に揃っている想定）

- Homebrew LLVM: `/opt/homebrew/opt/llvm/bin/clang++`
- CMake 3.25 以上
- OpenCV 4.x（Homebrew）— Chapter 1 以降で使用

確認:

```bash
/opt/homebrew/opt/llvm/bin/clang++ --version
cmake --version
brew list --versions opencv
```

## 初回ビルド

```bash
cd <repo root>
cmake -S . -B build \
  -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

初回の configure 時に doctest が `build/_deps/doctest-src` に取得される（～数秒）。

## clangd を使いたい場合

リポジトリルートに `build/compile_commands.json` が自動生成されます（`CMAKE_EXPORT_COMPILE_COMMANDS=ON`）。
エディタから認識させるには、ルートにシンボリックリンクを張ります:

```bash
ln -sf build/compile_commands.json compile_commands.json
```

これで `#include` 解決や型補完が効くようになります。

## ビルドタイプ

| `CMAKE_BUILD_TYPE` | 用途 |
|---|---|
| `Debug` | デバッグシンボル有、最適化なし。開発時はこれ。 |
| `Release` | 最適化有、`-DNDEBUG`。計測・配布時。 |
| `RelWithDebInfo` | 最適化有＋シンボル有。プロファイリング向け。 |

切り替えは build ディレクトリごと変えるのが安全:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release ...
cmake --build build-release
```

## オプションフラグ

| フラグ | 既定 | 説明 |
|---|---|---|
| `MODERN_CPP_BUILD_REFERENCE` | ON | 各章の `reference/` をビルド |
| `MODERN_CPP_BUILD_WORKSPACE` | ON | 各章の `workspace/` をビルド |

例: workspace だけ試したい:

```bash
cmake -S . -B build -DMODERN_CPP_BUILD_REFERENCE=OFF ...
```

## トラブル

### `cmake_minimum_required ... Compatibility with CMake < 3.5 has been removed`

CMake 4.x と、古い `cmake_minimum_required` を宣言した依存ライブラリの組み合わせで起きる。
本プロジェクトでは `cmake/Dependencies.cmake` 内で `CMAKE_POLICY_VERSION_MINIMUM 3.5` を設定して回避済み。

### macOS のカメラ許可（Chapter 1 以降）

初回カメラ起動時にダイアログが出る。Terminal.app や VS Code のカメラアクセスを許可すること。
