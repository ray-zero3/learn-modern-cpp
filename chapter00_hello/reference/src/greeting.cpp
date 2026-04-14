// ============================================================
// src/greeting.cpp
// ------------------------------------------------------------
// 実装側 (= 定義)。ヘッダに書いた関数の中身をここに書く。
//
// 「宣言と定義を分ける」の実利:
//   - ヘッダを #include した他ファイルはシグネチャだけ見る
//   - この .cpp だけコンパイルし直せば OK (= 再コンパイル最小化)
//   - 詳細は docs/deepdive/00-header-vs-source.md
// ============================================================
#include "greet/greeting.hpp"

#include <string>

namespace greet {

std::string greet(std::string_view name) {
    // std::string は string_view から暗黙変換できないので明示する。
    // `+` 演算子は std::string と string_view の連結をサポート (C++26予定なので安全側で)。
    std::string result = "Hello, ";
    result.append(name);
    result.push_back('!');
    return result;
}

TimeOfDay classify_hour(int hour_0_23) {
    // 境界値に注意: 5-11 = 朝, 12-17 = 昼, 18-21 = 夕, それ以外 = 夜
    if (hour_0_23 >= 5 && hour_0_23 <= 11) {
        return TimeOfDay::Morning;
    }
    if (hour_0_23 >= 12 && hour_0_23 <= 17) {
        return TimeOfDay::Afternoon;
    }
    if (hour_0_23 >= 18 && hour_0_23 <= 21) {
        return TimeOfDay::Evening;
    }
    return TimeOfDay::Night;
}

std::string greet_at(std::string_view name, TimeOfDay tod) {
    std::string_view prefix;
    switch (tod) {
    case TimeOfDay::Morning:   prefix = "Good morning, ";   break;
    case TimeOfDay::Afternoon: prefix = "Good afternoon, "; break;
    case TimeOfDay::Evening:   prefix = "Good evening, ";   break;
    case TimeOfDay::Night:     prefix = "Good night, ";     break;
    }
    std::string result;
    result.reserve(prefix.size() + name.size() + 1);
    result.append(prefix);
    result.append(name);
    result.push_back('!');
    return result;
}

} // namespace greet
