#ifndef _CONSOLE_X_UTIL_HPP_
#define _CONSOLE_X_UTIL_HPP_

/** ------------------------------------------------------------------------------------
 *  ConsoleX String Utility Module
 *  ------------------------------------------------------------------------------------
 *  UTF-8 문자열의 너비 계산, 문자열 분할, ANSI 코드 제거 등의
 *  helper 기능을 제공하는 정적 클래스입니다.
 *  ------------------------------------------------------------------------------------ */

#include <string>
#include <vector>

namespace cx
{
    /**
     * @brief 문자열 처리 유틸리티 클래스
     */
    class Util
    {
    public:
        /**
         * @brief  문자열의 콘솔 출력 너비를 계산합니다. (한글=2, 영문=1, ANSI코드=0)
         * @param  str UTF-8 인코딩된 문자열
         * @return 콘솔상에서 차지하는 칸 수
         */
        static size_t GetStringWidth( const std::string& str );

        /**
         * @brief  문자열을 지정된 너비(max_width)에 맞춰 여러 줄로 분할합니다.
         * @param  str 원본 문자열
         * @param  max_width 한 줄당 최대 허용 너비
         * @return 분할된 문자열 리스트
         *
         * @details
         *   단어 중간에서 잘리지 않도록 멀티바이트 문자를 고려하여 자릅니다.
         */
        static std::vector<std::string> SplitStringByWidth( const std::string& str, size_t max_width );

        /**
         * @brief 문자열에서 ANSI Escape Code(색상 등)를 제거한 순수 문자열을 반환합니다.
         */
        static std::string StripAnsiCodes( const std::string& str );

        /**
         * @brief 해당 UTF-8 문자가 2칸(Double Width)을 차지하는지 확인합니다.
         * @param codepoint 유니코드 코드포인트
         */
        static bool IsDoubleWidth( uint32_t codepoint );

    private:
        // Static helper class
        Util()  = delete;
        ~Util() = delete;
    };

} // namespace cx

#endif // _CONSOLE_X_UTIL_HPP_