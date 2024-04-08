// ConsoleX
#include "cx_console.h"
#include "cx_coord.h"
#include "cx_rgb.h"

// STL::C++
#include <iostream>

// DEFINE TEST
// #define TEST_CONSOLE
// #define TEST_COORD
// #define TEST_GET_COORD
#define TEST_SET_COORD
#define TEST_COLOR
// DEFINE TEST

int main()
{

#ifdef TEST_CONSOLE
    std::cout << "W: " << cx::GetConsoleW() << " H: " << cx::GetConsoleH() << std::endl;
#endif

#ifdef TEST_COORD
    cx::Coord Coord_1 {};
    std::cout << "Coord_1 : " << Coord_1 << std::endl;

    cx::Coord Coord_2 { 1, 4 };
    std::cout << "Coord_2 : " << Coord_2 << std::endl;

    cx::Coord err_Coord = cx::ErrorCoord {};
    if( !err_Coord )
        std::cout << "err_Coord if == false" << std::endl;

    cx::Coord Coord_3 { 1, 5 };
    if( Coord_2 != Coord_3 )
        std::cout << Coord_2 << " != " << Coord_3 << std::endl;

    std::cout << Coord_2 + Coord_3 << std::endl;
    std::cout << cx::Coord { 11, 12 } << std::endl;
#endif

#ifdef TEST_GET_COORD
    // system("clear");
    std::cout << "12345" << std::flush;
    auto coord_get = cx::GetCoord();
    std::cout << coord_get << std::endl;
    coord_get = cx::GetCoord();
    std::cout << coord_get << std::endl;
#endif

#ifdef TEST_SET_COORD
    auto coord_get = cx::GetCoord();
    cx::Coord coord_set { 30, 15 };
    system("clear");
    // cx::SetCoord( coord_set ) ;
    // std::cout << coord_set << coord_get.str() << " -> " << coord_set.str() << std::endl;
    std::cout << cx::Coord(50, 10) << coord_get.str() << " -> " << cx::GetCoord().str() << std::endl;
#endif

#ifdef TEST_COLOR
    std::cout << cx::Color("#12F032") << "Hello!" << cx::ResetColor << std::endl;
#endif

    return 0;
}