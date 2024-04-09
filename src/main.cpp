// DEFINE TEST ==================================================================
#define CODE_TEST_COORD
#define CODE_TEST_COLOR
#define CODE_TEST_GET_KEY
#define CODE_TEST_CONSOLE
// DEFINE TEST ==================================================================

// SET INCLUDE FROM TEST DEFINE =================================================
#if defined(CODE_TEST_COORD)
  #include "cx_coord.h"
#endif
// ------------------------------------------------------------------------------
#if defined(CODE_TEST_COLOR)
  #include "cx_rgb.h"
#endif
// ------------------------------------------------------------------------------
#if defined(CODE_TEST_GET_KEY)
  #include "cx_key.h"
#endif
// ------------------------------------------------------------------------------
#if defined(CODE_TEST_CONSOLE)
  #include "cx_console.h"
#endif
// SET INCLUDE FROM TEST DEFINE =================================================

// ConsoleX
// 실제 사용 시 아래 헤더 파일만 #include 하면 됨
#include "console_x.h"
// STL::C
#include <stdio.h>
// STL::C++
#include <iostream>

/**
 * HOW TO COMPILE
 * ==============
 * 1. 프로젝트 폴더로 이동 (Makefile 있는 경로)
 * 2. make clean && make
 * 3. ./build/runfile
 */

int main()
{
    system("clear");

#if defined(CODE_TEST_COORD)
    std::cout << "[CODE_TEST_COORD] ======================================================================" << std::endl;

    cx::Coord coord_sample { 6, 4 };
    std::cout << coord_sample << "1. COORD MOVED: " << coord_sample.str() << std::endl;

    cx::Coord coord_add { 3, 1 };
    std::cout << coord_sample + coord_add << "2. COORD ADD + MOVED: " << (coord_sample + coord_add).str() << std::endl;

    cx::Coord coord_sub { 4, 2 };
    std::cout << coord_sample - coord_sub << "3. COORD SUB + MOVED: " << (coord_sample - coord_sub).str() << std::endl;

    cx::SetCoord( { 15, 3 } );
    std::cout << "4. SetCoord(), and GetCoord() " <<  cx::GetCoord().str() << std::endl;

    std::cout << cx::Coord( 2, 7 ) << "5. set with std::cout" << std::endl;
    std::cout << std::endl;
#endif // CODE_TEST_COORD

#if defined(CODE_TEST_COLOR)
    std::cout << "[CODE_TEST_COLOR] ======================================================================" << std::endl;

    cx::SetFontColor("#FF0000");
    std::cout << "0. SetFontColor() -> ResetColor()" << std::endl;
    cx::ResetColor();

    cx::SetFontColor(cx::RGB::YELLOW);
    std::cout << "0. SetFontColor() -> ResetColor()" << std::endl;
    cx::ResetColor();

    cx::Color color_sample_fg = cx::FontColor( cx::RGB::CYAN );
    cx::Color color_sample_bg = cx::BackColor( cx::RGB::WHITE );

    std::cout << color_sample_fg << "1. Hello Wolrd with FG" << std::endl;
    cx::ResetColor();

    std::cout << color_sample_bg << "2. Hello Wolrd with BG" << std::endl;
    cx::ResetColor();

    std::cout << color_sample_fg << color_sample_bg << "3. Hello World with FG & BG" << std::endl;
    cx::ResetColor();

    cx::Color rgb_check_good {"#FF00F0"};
    cx::Color rgb_check_fail {"?FX00F!"};

    const auto rgb_set_good = rgb_check_good.rgb();
    const auto rgb_set_fail = rgb_check_fail.rgb();

    std::cout << "rgb_set_good : " << rgb_set_good.r << ", " << rgb_set_good.g << ", " << rgb_set_good.b << std::endl;
    std::cout << "rgb_set_fail : " << rgb_set_fail.r << ", " << rgb_set_fail.g << ", " << rgb_set_fail.b << std::endl;

    #if defined(CODE_TEST_COLOR) && defined(CODE_TEST_COORD)
        std::cout << cx::Coord( 40, 15 )
            << "4. Test Coord + " << cx::FontColor( cx::RGB::MAROON )
            << "color code lasts" << cx::ResetColor << std::endl;
    #endif // CODE_TEST_COLOR + CODE_TEST_COORD

    #if defined(CODE_TEST_COLOR) && defined(CODE_TEST_CONSOLE)
        auto color_sample_fg_2 = cx::FontColor( "#F2F030" );
        auto color_sample_bg_2 = cx::BackColor( "#45F13A" );

        // use cx::ResetColor inside std::cout ~ std::endl
        std::cout << color_sample_fg_2 << "5. Hello Wolrd with FG_II" << cx::ResetColor << std::endl;
        std::cout << color_sample_bg_2 << "6. Hello Wolrd with BG_II" << cx::ResetColor << std::endl;

        // 'cx::end_line' == 'cx::ResetColor << std::endl'
        std::cout << color_sample_fg_2 << "7. Hello Wolrd with FG_II" << cx::end_line;
        std::cout << color_sample_bg_2 << "8. Hello Wolrd with BG_II" << cx::end_line;

        // you can use 'cx::console(RGB) << ...' instead of 'std::cout << cx::Color << ...'
        cx::console("#A2A03A", cx::FG) << "9. Hellow World" << cx::end_line;
        cx::console("#3BFBFB", cx::BG) << "0. Hellow World" << cx::end_line;
    #endif // CODE_TEST_COLOR + CODE_TEST_CONSOLE

    std::cout << std::endl;

#endif // CODE_TEST_COLOR

#if defined(CODE_TEST_CONSOLE)
    std::cout << "[CODE_TEST_CONSOLE] ======================================================================" << std::endl;
    std::cout << "W: " << cx::GetConsoleW() << " H: " << cx::GetConsoleH() << std::endl;
    std::cout << std::endl;
#endif

#if defined(CODE_TEST_GET_KEY)
    cx::Key::SetReadKeyAwaitTimeout( cx::KeyDelay::FPS_60 );

    std::cout << "[CODE_TEST_GET_KEY] ( PRESS Any Key / EXIT = ESC ) =======================================" << std::endl;
    bool isQuit = false;
    while( !isQuit ) {

        // If you use Key::GetKeyTimeout(), Until a key is entered,
        // '.' is output repeatedly in the console.
        cx::KeyBoard input = cx::Key::GetKeyTimeout();

        // If you use Key::GetKey(), Until a key is entered,
        // '.' is not output and waits in GetKey()
        // cx::KeyBoard input = cx::Key::GetKey();

        switch( input ) {
        case cx::KeyBoard::ARROW_UP:    printf("[UP]");    break;
        case cx::KeyBoard::ARROW_DOWN:  printf("[DOWN]");  break;
        case cx::KeyBoard::ARROW_RIGHT: printf("[RIGHT]"); break;
        case cx::KeyBoard::ARROW_LEFT:  printf("[LEFT]");  break;

        case cx::KeyBoard::SPACE:       printf("[SPACE]"); break;
        case cx::KeyBoard::ENTER:       printf("[ENTER]"); break;
        case cx::KeyBoard::TAB:         printf("[TAB]");   break;

        case cx::KeyBoard::F10:         printf("[F10]");   break;
        case cx::KeyBoard::F11:         printf("[F11]");   break;
        case cx::KeyBoard::F12:         printf("[F12]");   break;

        // For Multi-Thread thread-safe, not use it
        case cx::KeyBoard::ALREADY_OCCUPIED:
            printf("{ KEY INPUT ALREADY OCCUPIED }\n");
            break;

        // Quit
        case cx::KeyBoard::ESC:
            isQuit = true;
            break;

        // If the Key::GetKeyTimeout() function is over time, it enters the case.
        case cx::KeyBoard::NONE_INPUT:
            printf(".");
            break;

        default:
            printf("[%c]", static_cast<int>( input ));
            break;
        }
        fflush( stdout ); // buffer flush
    } // while( !isQuit )
    printf("\n");
#endif

    return 0;
}