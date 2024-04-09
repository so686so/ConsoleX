// DEFINE TEST ==================================================================
#define CODE_TEST_COORD
#define CODE_TEST_COLOR
//#define CODE_TEST_GET_KEY
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
// #include "console_x.h"
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
    // cx::Coord 이용한 커서 이동
    cx::Coord coord_sample { 6, 4 };
    std::cout << coord_sample << "01. COORD MOVED: " << coord_sample.str() << std::endl;

    // cx::Coord 간의 덧셈 및 이동
    cx::Coord coord_add { 3, 1 };
    std::cout << coord_sample + coord_add << "02. COORD ADD + MOVED: " << (coord_sample + coord_add).str() << std::endl;

    // cx::Coord 간의 뺄셈 및 이동
    cx::Coord coord_sub { 4, 2 };
    std::cout << coord_sample - coord_sub << "03. COORD SUB + MOVED: " << (coord_sample - coord_sub).str() << std::endl;

    // std::cout 외부에서 개별적으로 좌표값 변경 및
    // std::cout 내부에서도 좌표값을 읽을 수 있음
    cx::SetCoord( { 15, 3 } );
    std::cout << "04. SetCoord(), and GetCoord() " <<  cx::GetCoord().str() << std::endl;

    // cx::Coord 변수 지정 없이 생성자로 객체 생성해서 즉시 적용 가능
    std::cout << cx::Coord( 2, 7 ) << "05. set with std::cout" << std::endl;
    std::cout << std::endl;
#endif // CODE_TEST_COORD

#if defined(CODE_TEST_COLOR)
    std::cout << "[CODE_TEST_COLOR] ======================================================================" << std::endl;

    // Hex Code 문자열 값을 이용한 폰트 색깔 할당 및 초기화
    cx::SetFontColor("#FF0000");
    std::cout << "00. SetFontColor() -> ResetColor()" << std::endl;
    cx::ResetColor();

    // cx::RGB enum 값을 이용한 폰트 색깔 할당 및 초기화
    cx::SetFontColor(cx::RGB::YELLOW);
    std::cout << "01. SetFontColor() -> ResetColor()" << std::endl;
    cx::ResetColor();

    // 미리 색상을 변수에 저장해놓고 활용할 수 있음
    cx::Color color_sample_fg = cx::FontColor( cx::RGB::CYAN );
    cx::Color color_sample_bg = cx::BackColor( cx::RGB::WHITE );

    std::cout << color_sample_fg << "02. Hello Wolrd with FG" << std::endl;
    cx::ResetColor();

    std::cout << color_sample_bg << "03. Hello Wolrd with BG" << std::endl;
    cx::ResetColor();

    // 폰트와 배경을 동시에 적용할 수 있음
    std::cout << color_sample_fg << color_sample_bg << "04. Hello World with FG & BG" << std::endl;
    cx::ResetColor();

    // 변수에 저장하지 않고 생성 즉시 std::cout 안에서 적용도 가능
    std::cout << cx::FontColor( cx::RGB::PURPLE ) << "05. Color apply direct" << std::endl;
    cx::ResetColor();

    cx::Color rgb_check_good {"#009F9F"}; // 올바른 헥스 문자열 값
    cx::Color rgb_check_fail {"?FX00F!"}; // 이상한 헥스 문자열 값

    // cx::rgb_set 값 가져오기. 유효하지 않은 색상 값이라면 { -1, -1, -1 } 값.
    const auto rgb_set_good = rgb_check_good.rgb();
    const auto rgb_set_fail = rgb_check_fail.rgb();

    std::cout << rgb_check_good << "06. rgb_set_good : " << rgb_set_good.r << ", " << rgb_set_good.g << ", " << rgb_set_good.b << std::endl;
    cx::ResetColor();
    std::cout << rgb_check_fail << "07. rgb_set_fail : " << rgb_set_fail.r << ", " << rgb_set_fail.g << ", " << rgb_set_fail.b << std::endl;
    cx::ResetColor();

    #if defined(CODE_TEST_COLOR) && defined(CODE_TEST_COORD)
        // 좌표 변환과 색상 설정 동시에 적용 가능
        std::cout << cx::Coord( 40, 20 ) << "08. Test Coord + "
                  << cx::FontColor( cx::RGB::MAROON ) << "color code lasts" << cx::ResetColor << std::endl;
    #endif // CODE_TEST_COLOR + CODE_TEST_COORD

    #if defined(CODE_TEST_COLOR) && defined(CODE_TEST_CONSOLE)
        auto color_sample_fg_2 = cx::FontColor( "#62A030" );
        auto color_sample_bg_2 = cx::BackColor( "#45613A" );

        // std::cout ~ std::endl 문장 사이에 cx::ResetColor 삽입 가능
        std::cout << color_sample_fg_2 << "09. Hello Wolrd with FG_II" << cx::ResetColor << std::endl;
        std::cout << color_sample_bg_2 << "10. Hello Wolrd with BG_II" << cx::ResetColor << std::endl;

        // 'cx::ResetColor << std::endl' 코드를 'cx::end_line' 으로 대체 가능
        std::cout << color_sample_fg_2 << "11. Hello Wolrd with FG_II" << cx::end_line;
        std::cout << color_sample_bg_2 << "12. Hello Wolrd with BG_II" << cx::end_line;

        // 'std::cout << 색상 할당 << ...' 대신에 'cx::console(RGB) << ...' 대체 가능
        cx::console("#A2A03A", cx::FG) << "13. Hellow World" << cx::end_line;
        cx::console("#3BFBFB", cx::BG) << "14. Hellow World" << cx::end_line;
        // 만약 cx::FG / cx::BG 값 없이 쓴다면 기본적으로 폰트 색깔 할당임
        cx::console("#45F045")         << "15. Hellow World" << cx::end_line;

    #endif // CODE_TEST_COLOR + CODE_TEST_CONSOLE

    std::string grad_str = "16. Gradation String Check";
    for( size_t i = 0; i < grad_str.size(); ++i ) {
        const int grad = 255 - ( i * 10 );
        std::cout << cx::FontColor( cx::rgb_set { 90, grad, grad } ) << grad_str[i];
    }
    std::cout << std::endl;
    cx::ResetColor();

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