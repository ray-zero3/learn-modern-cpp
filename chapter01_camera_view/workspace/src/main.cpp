// ============================================================
// app/main.cpp
// ------------------------------------------------------------
// 実行ファイルのエントリポイント。ライブラリを「使う側」の例。
// 本物のアプリはこの薄さで書いて、ロジックはライブラリに置く
// ほうがテストしやすい（TS でも同じ思想）。
// ============================================================
#include "greet/greeting.hpp"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    // 引数があれば名前として使う。なければ "World"。
    // TS: const name = process.argv[2] ?? "World"
    const std::string name = (argc > 1) ? std::string{argv[1]} : std::string{"World"};

    std::cout << greet::greet(name) << '\n';

    // 時刻帯別バリアントも見せる。実時刻は後の章で触れる (chrono)。
    std::cout << greet::greet_at(name, greet::TimeOfDay::Morning) << '\n';
    std::cout << greet::greet_at(name, greet::TimeOfDay::Evening) << '\n';
    return 0;
}
