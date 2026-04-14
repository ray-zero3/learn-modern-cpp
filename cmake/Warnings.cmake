# ============================================================
# Warnings.cmake
# ------------------------------------------------------------
# 「警告はエラー級の情報」という立場で、章の全ターゲットに
# 強めの警告を付けるヘルパー関数を提供する。
#
# 呼び出し側:
#   modern_cpp_enable_warnings(<target>)
# ============================================================

function(modern_cpp_enable_warnings target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
            -Wsign-conversion
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            # -Werror は学習中は外しておく（警告を見ながら学びたい）
        )
    elseif(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    endif()
endfunction()
