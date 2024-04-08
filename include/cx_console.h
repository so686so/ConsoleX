#ifndef _CONSOLE_X_CONSOLE_H_
#define _CONSOLE_X_CONSOLE_H_

// ConsoleX
#include "cx_rgb.h"

// STL::C++
#include <cstddef> // size_t
#include <ostream> // basic_ostream

namespace cx // ConsoleX main namespace
{
    /**
     * @brief
     *   Get console width(row) size
     *
     * @return
     *   If we successfully get the width of the console,
     *   we return that width. If get fails, 0 is returned.
     */
    size_t GetConsoleW( void );

    /**
     * @brief
     *   Get console height(column) size
     *
     * @return
     *   If we successfully get the height of the console,
     *   we return that height. If get fails, 0 is returned.
     */
    size_t GetConsoleH( void );

    /**
     * @brief
     *    Get standard output(std::cout) with cx::Color setting
     * 
     * @param value
     *    color setting value, for example, `"#00FF00"` or `cx::RGB::GREEN`
     * 
     * @param is_font
     *    if that value `cx::FG`, color setting adjust Font Color
     *    else, value `cx::BG`, color setting adjust Background Color
     * 
     * @return
     *    std::cout
     * 
     * @example
     *    cx::console("#012345", cx::BG) << "Hello world" << std::endl;
     */
    template <typename T>
    inline std::ostream& console( const T& value, const bool is_font = cx::FG )
    {
        cx::SetColor( value, is_font );
        return std::cout;
    }

    /**
     * @brief
     *    Get standard output(std::cout) with reset cx::Color setting
     * 
     * @return
     *    std::cout
     */
    inline std::ostream& console( void )
    {
        cx::ResetColor();
        return std::cout;
    }

    /**
     * @brief
     *   Reset cx::Color setting - use with std::cout
     * 
     * @return
     *   std::ostream itself
     * 
     * @example
     *   std::cout << cx::FontColor(cx::RGB::RED) << "Hello world" << cx::ResetColor << std::endl; 
     */
    std::ostream& ResetColor( std::ostream& os ) noexcept;

    /**
     * @brief
     *    Get std::endl with reset cx::Color setting
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