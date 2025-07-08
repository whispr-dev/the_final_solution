#include <immintrin.h>
#include <cstddef>
#include <cstdint>

// Detect whitespace characters using SIMD
inline __m256i is_whitespace(__m256i block) {
    __m256i spaces = _mm256_cmpeq_epi8(block, _mm256_set1_epi8(' '));
    __m256i tabs   = _mm256_cmpeq_epi8(block, _mm256_set1_epi8('\t'));
    __m256i nl     = _mm256_cmpeq_epi8(block, _mm256_set1_epi8('\n'));
    __m256i cr     = _mm256_cmpeq_epi8(block, _mm256_set1_epi8('\r'));
    return _mm256_or_si256(_mm256_or_si256(spaces, tabs), _mm256_or_si256(nl, cr));
}

size_t minify_json_avx2(const char* src, size_t len, char* dst) {
    size_t src_pos = 0, dst_pos = 0;
    bool in_string = false;
    bool prev_escape = false;

    while (src_pos + 32 <= len) {
        __m256i block = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + src_pos));
        __m256i ws_mask = is_whitespace(block);

        for (int i = 0; i < 32; ++i) {
            char c = src[src_pos + i];
            if (c == '"' && !prev_escape) in_string = !in_string;
            if (c == '\\' && in_string) prev_escape = !prev_escape;
            else prev_escape = false;

            bool is_ws = (c == ' ' || c == '\t' || c == '\r' || c == '\n');
            if (!is_ws || in_string) {
                dst[dst_pos++] = c;
            }
        }
        src_pos += 32;
    }

    // Handle trailing bytes
    for (; src_pos < len; ++src_pos) {
        char c = src[src_pos];
        if (c == '"' && !prev_escape) in_string = !in_string;
        if (c == '\\' && in_string) prev_escape = !prev_escape;
        else prev_escape = false;

        bool is_ws = (c == ' ' || c == '\t' || c == '\r' || c == '\n');
        if (!is_ws || in_string) {
            dst[dst_pos++] = c;
        }
    }
    return dst_pos;
}
