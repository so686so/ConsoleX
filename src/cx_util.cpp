#include "cx_util.hpp"

namespace cx
{
    // =========================================================================
    // Internal Helper Functions (UTF-8 Parsing)
    // =========================================================================

    /**
     * @brief UTF-8 문자열에서 다음 문자의 바이트 길이와 코드포인트를 추출합니다.
     * @return {바이트길이, 코드포인트}
     */
    static std::pair<int, uint32_t> GetUtf8CharInfo( const char* str )
    {
        unsigned char c = static_cast<unsigned char>( str[0] );

        // 1 Byte (ASCII)
        if( c < 0x80 ) return { 1, static_cast<uint32_t>( c ) };

        // 2 Bytes (0x80 ~ 0x7FF)
        if( ( c & 0xE0 ) == 0xC0 ) {
            uint32_t val = ( c & 0x1F ) << 6;
            val |= ( static_cast<unsigned char>( str[1] ) & 0x3F );
            return { 2, val };
        }

        // 3 Bytes (0x800 ~ 0xFFFF) - 한글 포함
        if( ( c & 0xF0 ) == 0xE0 ) {
            uint32_t val = ( c & 0x0F ) << 12;
            val |= ( static_cast<unsigned char>( str[1] ) & 0x3F ) << 6;
            val |= ( static_cast<unsigned char>( str[2] ) & 0x3F );
            return { 3, val };
        }

        // 4 Bytes (0x10000 ~ 0x10FFFF) - 이모지 포함
        if( ( c & 0xF8 ) == 0xF0 ) {
            uint32_t val = ( c & 0x07 ) << 18;
            val |= ( static_cast<unsigned char>( str[1] ) & 0x3F ) << 12;
            val |= ( static_cast<unsigned char>( str[2] ) & 0x3F ) << 6;
            val |= ( static_cast<unsigned char>( str[3] ) & 0x3F );
            return { 4, val };
        }

        return { 1, 0 }; // Invalid UTF-8, treat as length 1
    }

    // =========================================================================
    // Public API Implementation
    // =========================================================================

    /**
     * @brief 너비를 차지하지 않는 특수 문자(Zero Width)인지 확인합니다.
     *
     * @details
     *   ZWJ(Zero Width Joiner), Variation Selectors, Combining Marks 등을 처리합니다.
     */
    static bool IsZeroWidth( uint32_t cp )
    {
        if ( cp == 0 ) return true; // Null

        // 1. ZWJ & ZWNJ (0x200C: Non-Joiner, 0x200D: Joiner)
        // 이모지 결합에 핵심적으로 사용됨 (예: 가족, 직업 이모지 등)
        if ( cp == 0x200C || cp == 0x200D ) return true;

        // 2. Variation Selectors (0xFE00 ~ 0xFE0F)
        // 이모지 스타일 선택자 (텍스트형 vs 이모지형)
        if ( cp >= 0xFE00 && cp <= 0xFE0F ) return true;

        // 3. Combining Diacritical Marks (0x0300 ~ 0x036F)
        // 문자 위에 찍히는 점, 악센트 등
        if ( cp >= 0x0300 && cp <= 0x036F ) return true;

        // 4. Skin Tone Modifiers (0x1F3FB ~ 0x1F3FF)
        // 단독으로 쓰일 땐 너비가 있지만, 보통 앞의 이모지와 합쳐지므로
        // "너비 계산" 관점에서는 0으로 치는 것이 커서 동기화에 유리함.
        if ( cp >= 0x1F3FB && cp <= 0x1F3FF ) return true;

        // 5. Tags for Emoji (0xE0020 ~ 0xE007F) - 국기 시퀀스 등에 사용
        if ( cp >= 0xE0020 && cp <= 0xE007F ) return true;

        return false;
    }

    bool Util::IsDoubleWidth( uint32_t cp )
    {
        // 한글 범위 (Hangul Jamo, Syllables)
        if( ( cp >= 0x1100 && cp <= 0x11FF ) ||
            ( cp >= 0x3130 && cp <= 0x318F ) ||
            ( cp >= 0xAC00 && cp <= 0xD7A3 ) )
            return true;

        // CJK Unified Ideographs (한자)
        if( cp >= 0x4E00 && cp <= 0x9FFF ) return true;
        if( cp >= 0x3400 && cp <= 0x4DBF ) return true; // CJK Ext A
        if( cp >= 0xF900 && cp <= 0xFAFF ) return true; // CJK Compatibility

        // Fullwidth Forms (전각 문자)
        if( cp >= 0xFF01 && cp <= 0xFF60 ) return true;
        if( cp >= 0xFFE0 && cp <= 0xFFE6 ) return true;

        // Emoji & Symbols (Supplemental)
        if( cp >= 0x1F300 && cp <= 0x1F6FF ) return true;
        if( cp >= 0x1F900 && cp <= 0x1F9FF ) return true; // Supplemental Symbols
        if( cp >= 0x1F004 && cp <= 0x1F251 ) return true; // Enclosed CJK chars...

        return false;
    }

    size_t Util::GetStringWidth( const std::string& str )
    {
        size_t width = 0;
        size_t i = 0;
        size_t len = str.length();

        while( i < len ) {
            // Check ANSI Escape Code
            if( str[i] == '\033' ) {
                // '[' 확인
                if( i + 1 < len && str[i+1] == '[' ) {
                    size_t j = i + 2;
                    while( j < len ) {
                        char c = str[j];
                        // ANSI 종료 문자 (m, K, H 등)
                        if( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) ) {
                            j++;
                            break;
                        }
                        j++;
                    }
                    i = j; // Skip ANSI sequence
                    continue;
                }
            }

            // Get Char Info
            auto [byte_len, codepoint] = GetUtf8CharInfo( &str[i] );

            // Zero Width 문자 확인 -> 너비 추가 안 함
            if ( IsZeroWidth( codepoint ) ) {
                // 너비 추가 없음
            }
            else if( IsDoubleWidth( codepoint ) ) {
                width += 2;
            }
            else {
                width += 1;
            }

            i += byte_len;
        }

        return width;
    }

    std::string Util::StripAnsiCodes( const std::string& str )
    {
        std::string res;
        res.reserve( str.length() );

        size_t i = 0;
        size_t len = str.length();

        while( i < len ) {
            if( str[i] == '\033' ) {
                if( i + 1 < len && str[i+1] == '[' ) {
                    size_t j = i + 2;
                    while( j < len ) {
                        char c = str[j];
                        if( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) ) {
                            j++;
                            break;
                        }
                        j++;
                    }
                    i = j;
                    continue;
                }
            }
            res += str[i];
            i++;
        }
        return res;
    }

    std::vector<std::string> Util::SplitStringByWidth( const std::string& str, size_t max_width )
    {
        std::vector<std::string> lines;
        std::string current_line;
        size_t current_width = 0;

        size_t i = 0;
        size_t len = str.length();

        while( i < len ) {
            // 1. Handle ANSI Code (Copy but don't count width)
            if( str[i] == '\033' ) {
                if( i + 1 < len && str[i+1] == '[' ) {
                    size_t start = i;
                    size_t j = i + 2;
                    while( j < len ) {
                        char c = str[j];
                        if( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) ) {
                            j++;
                            break;
                        }
                        j++;
                    }
                    // Add ANSI code to current line directly
                    current_line.append( str, start, j - start );
                    i = j;
                    continue;
                }
            }

            // 2. Handle Character
            auto [byte_len, codepoint] = GetUtf8CharInfo( &str[i] );
            size_t char_width = IsDoubleWidth( codepoint ) ? 2 : 1;

            // 3. Wrap Check
            if( current_width + char_width > max_width ) {
                lines.push_back( current_line );
                current_line.clear();
                current_width = 0;
            }

            // 4. Append
            current_line.append( str, i, byte_len );
            current_width += char_width;
            i += byte_len;
        }

        if( !current_line.empty() ) {
            lines.push_back( current_line );
        }

        return lines;
    }

} // namespace cx