#ifndef _CONSOLE_X_CONSOLE_H_
#define _CONSOLE_X_CONSOLE_H_

// STL::C++
#include <cstddef> // size_t

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

} // nsp::cx

#endif