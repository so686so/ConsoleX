#ifndef _CONSOLE_X_CONSOLE_H_
#define _CONSOLE_X_CONSOLE_H_

// ConsoleX
#include "cx_rgb.h" // SetColor, ResetColor
// STL::C++
#include <cstddef>  // size_t
#include <iostream> // cout, endl, basic_ostream

namespace cx // ConsoleX main namespace
{
    /**
     * @brief
     *   터미널의 너비(row)를 구하는 함수.
     *
     * @return
     *   터미널의 너비를 성공적으로 구한다면 1 이상의 너비값을 반환.
     *   만약 실패한다면 0을 반환.
     */
    size_t GetConsoleW( void );

    /**
     * @brief
     *   터미널의 높이(column)를 구하는 함수.
     *
     * @return
     *   터미널의 높이를 성공적으로 구한다면 1 이상의 높이값을 반환.
     *   만약 실패한다면 0을 반환.
     */
    size_t GetConsoleH( void );

    /**
     * @brief
     *    cx::Color 세팅이 적용된 표준출력(std::cout)을 반환.
     *
     * @param value
     *    색상 세팅 값.
     *    예를 들어, `"#00FF00"` 같은 RGB HEX 문자열이거나,
     *    enum class로 정의된 `cx::RGB::GREEN` 같은 값들.
     *
     * @param is_font
     *    해당 값이 `cx::FG(=true)` 값이라면, 색상 세팅값이 글자에 적용됨.
     *    만약 `cx::BG(=false)` 값이라면, 색상 세팅값이 배경에 적용됨.
     *
     * @return
     *    std::cout
     *
     * @example
     *    cx::console("#012345", cx::BG) << "Hello world" << std::endl;
     *
     * @details
     *    템플릿 함수는 정의와 선언을 개별 파일에 작성할 수 없다.
     *    따라서 템플릿 함수는 일반적으로 헤더 파일에 선언과 정의를 같이 작성.
     */
    template <typename T>
    inline std::ostream& console( const T& value, const bool is_font = cx::FG )
    {
        cx::_SetColor( value, is_font );
        return std::cout;
    }

    /**
     * @brief
     *    cx::Color 세팅 초기화를 적용한 표준출력(std::cout)을 반환.
     *
     * @return
     *    std::cout
     */
    std::ostream& console( void ) noexcept;

    /**
     * @brief
     *   std::cout 과 같이 사용하는, `<<` 를 활용한 cx::Color 초기화 함수.
     *
     * @return
     *   std::ostream 출력 객체 그 자신. 예제 참조.
     *
     * @example
     *   std::cout << cx::FontColor(cx::RGB::RED) << "Hello world" << cx::ResetColor << std::endl;
     */
    std::ostream& ResetColor( std::ostream& os ) noexcept;

    /**
     * @brief
     *    cx::Color 세팅 초기화를 적용한 std::enld 을 반환.
     *
     * @return
     *    std::endl
     *
     * @example
     *    cx::console("#FF00FF", cx::FG) << "Hello world" << cx::end_line;
     */
    template<typename _CharT, typename _Traits>
    inline std::basic_ostream<_CharT, _Traits>& end_line(std::basic_ostream<_CharT, _Traits>& __os)
    {
        cx::ResetColor();
        return std::endl(__os);
    }

} // nsp::cx

#endif