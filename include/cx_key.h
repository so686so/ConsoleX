#ifndef _CONSOLE_X_KEY_H_
#define _CONSOLE_X_KEY_H_

/**
 * Linux 즉시 키입력 코드
 */

// STL :: C
#include <termios.h> // struct termios
#include <unistd.h>  //
#include <stdio.h>   //
// STL :: CPP
#include <memory>    // std::shared_ptr
#include <mutex>     // mutex, unique_lock
#include <iostream>

namespace cx // ConsoleX main namespace
{
    // Enum for key identifier
    constexpr int ARROW_ADD_VALUE = 128;
    constexpr int FUNCIONAL_VALUE = 256;

    enum KeyBoard
    {
        // Alphabet : Only use uppercase
        A = 65, B = 66, C = 67, D = 68, E = 69, F = 70, G = 71, H = 72, I = 73,
        J = 74, K = 75, L = 76, M = 77, N = 78, O = 79, P = 80, Q = 81, R = 82,
        S = 83, T = 84, U = 85, V = 86, W = 87, X = 88, Y = 89, Z = 90,

        // Numbers
        NUM_0 = 48, NUM_1 = 49, NUM_2 = 50, NUM_3 = 51, NUM_4 = 52,
        NUM_5 = 53, NUM_6 = 54, NUM_7 = 55, NUM_8 = 56, NUM_9 = 57,

        // Special keys
        TAB = 9, ENTER = 10, ESC = 27, SPACE = 32, BACKSPACE = 127,

        // Funcional keys
        F1  = 11 + FUNCIONAL_VALUE,
        F2  = 12 + FUNCIONAL_VALUE,
        F3  = 13 + FUNCIONAL_VALUE,
        F4  = 14 + FUNCIONAL_VALUE,
        F5  = 15 + FUNCIONAL_VALUE,
        F6  = 16 + FUNCIONAL_VALUE,
        F7  = 17 + FUNCIONAL_VALUE,
        F8  = 18 + FUNCIONAL_VALUE,
        F9  = 19 + FUNCIONAL_VALUE,
        F10 = 20 + FUNCIONAL_VALUE,
        F11 = 21 + FUNCIONAL_VALUE,
        F12 = 22 + FUNCIONAL_VALUE,

        // Arrows
        ARROW_UP    = 65 + ARROW_ADD_VALUE,
        ARROW_DOWN  = 66 + ARROW_ADD_VALUE,
        ARROW_RIGHT = 67 + ARROW_ADD_VALUE,
        ARROW_LEFT  = 68 + ARROW_ADD_VALUE,

        // Default Flags
        NONE_INPUT       = -2,
        ALREADY_OCCUPIED = -3,
        FORCE_INTERRUPT  = 11,
    };

    // For check & capitalize
    constexpr int CAPITALIZE_OFFSET = 32;
    enum LowerKeyBoard
    {
        a = A + CAPITALIZE_OFFSET,
        z = Z + CAPITALIZE_OFFSET
    };

    // For convinient timeout set
    enum KeyDelay
    {
        FPS_01 = 1000 / 01,
        FPS_10 = 1000 / 10,
        FPS_15 = 1000 / 15,
        FPS_20 = 1000 / 20,
        FPS_25 = 1000 / 25,
        FPS_30 = 1000 / 30,
        FPS_60 = 1000 / 60,
    };

    // Typename contraction
    using Termios = struct termios;
    using Timeval = struct timeval;

    class Key
    {
    // 주요 키 입력 read 함수들 ----------------------------------------------------------------------------------------
    public:
        /**
         * @brief  동기적으로 키 값을 읽는 함수.
         * @note   해당 함수는 키 입력이 발생할 때까지 대기.
         * @return 누른 키 값이 cx::KeyBoard enum 값으로 변환되어 반환.
         *         대기 중 cx::Key::ForceStopGetKey() 실행 시, cx::KeyBoard::FORCE_INTERRUPT 반환.
         */
        static KeyBoard GetKey( void ) noexcept;

        /**
         * @brief  동기적으로 키 값을 읽다가 설정한 시간(milsec) 초과시 timeout 하는 함수.
         * @note   해당 함수는 타임아웃 까지의 기간 동안 키 입력을 대기.
         *         키 입력 이벤트 발생시 timeout 시간 이내여도 즉시 반환.
         * @param  timeout_milsec
         *         해당 값이 0 이상이라면, 해당 함수가 동작하는 단 한 번만 해당 milliseconds 만큼
         *         timeout wait를 실행하며 키입력을 대기함.
         * @return 키입력이 발생했다면 누른 키 값이 cx::KeyBoard enum 값으로 변환되어 반환.
         *         만약 timeout 시간까지 키입력이 없었다면 'cx::KeyBoard::NONE_INPUT' 반환.
         *         만약 대기 시간 동안 다른 키입력 함수를 실행 시, 'cx::KeyBoard::ALREADY_OCCUPIED' 반환.
         */
        static KeyBoard GetKeyTimeout( const int timeout_milsec = -1 ) noexcept;

    // Flag 혹은 config 세팅 함수들 ------------------------------------------------------------------------------------------
    public:
        /**
         * @brief  GetKeyTimeout() 함수 실행 시 기다리는 기본 timeout milliseconds 설정하는 함수
         * @param  timeout_milsec
         *         해당 값이 0 이상이라면, GetKeyTimeout() timeout milliseconds를 해당 값으로 변경 시도.
         * @return 변경이 성공했다면 true 반환, 다른 코드에서 이미 GetKeyTimeout() 작동 중 등의 이유로
         *         변경 실패 시 false 반환.
         */
        static bool SetReadKeyAwaitTimeout( const int timeout_milsec ) noexcept;

    // Utilities -------------------------------------------------------------------------------------------------------------
    public:
        /**
         * @brief  임시적으로 '즉시 키 입력'과 '커서 숨김'을 비활성화 시도하는 함수.
         * @return 비활성화 시도가 성공했다면 true, 다른 코드에서 키입력 대기 등의 이유로 비활성화 실패 시 false 반환.
         */
        static bool TryPause( void ) noexcept;

        /**
         * @brief 임시적으로 '즉시 키 입력'과 '커서 숨김' 비활성화를 강제로 시행하는 함수.
         * @note  만약 다른 곳에서 키 입력 대기등을 시행하고 있을 때,
         *        해당 함수는 비활성화 될 때까지 대기하며 비활성화를 시도함.
         */
        static void ForcePause( void ) noexcept;

        /**
         * @brief TryPause() or ForcePause() 함수로 비활성화 된 '즉시 키 입력' 과 '커서 숨김'을 다시 활성화.
         */
        static void Resume( void ) noexcept;

        /**
         * @brief  cx::KeyBoard enum value 값을 Int값으로 변환 시도
         * @param  key cx::KeyBoard enum 값 중 하나
         * @return 만약 해당 값이 문자로 '0' ~ '9' 사이의 값일 시, 실제 0 ~ 9 숫자 반환.
         *         아니라면 -1 반환.
         */
        static int KeyNumToInt( const KeyBoard key ) noexcept
        {
            return ( key >= NUM_0 && key <= NUM_9 ) ? (int)( key - 48 ) : -1;
        }

        /**
         * @brief cx::Key::GetKey() 함수를 강제로 중지하는 함수
         * @note  해당 함수로 cx::Key::GetKey() 중지 시, 대기 코드 부분에서는 cx::KeyBoard::FORCE_INTERRUPT 반환.
         */
        static void ForceStopGetKey( void )
        {
            GetKeyPtr()->await_force_stop_flg = true;
        }

    // Others ----------------------------------------------------------------------------------------------------------------
    public:
        /**
         * @brief shared_ptr 의 RAII 위한 public 소멸자.
         */
        ~Key() { Key::Deinit(); }

    // Private methodes ------------------------------------------------------------------------------------------------------
    private:
        /**
         * @brief 싱글톤 패턴과 RAII 위한 private 생성자.
         */
        explicit Key()
            : is_key_stroke_direct( false )
            , await_force_stop_flg( false )
            , await_timeout_milsec( KeyDelay::FPS_30 )
        { Key::Init(); }

        /**
         * @brief Key class 초기화 함수
         */
        void Init( void ) noexcept;

        /**
         * @brief Key class 종료 및 해제 함수
         */
        void Deinit( void ) noexcept;

        /**
         * @brief  cx::Key 의 singleton shared_ptr 를 반환하는 함수
         * @return Singleton pattern DirectKey::Key pointer
         */
        static std::shared_ptr<cx::Key> GetKeyPtr( void ) noexcept;

        /**
         * @brief  키 입력 순간을 엔터 입력 후가 아닌 키를 누를 때로 바꾸는 함수.
         * @return 해당 변환이 성공한다면 true, 다른 곳에서 키 입력 대기 등으로 변환 실패 시 false.
         */
        bool SetKeyStrokeDirect( void ) noexcept;

        /**
         * @brief  키 입력 순간을 엔터 입력 후로 바꾸는 함수.
         * @return 해당 변환이 성공한다면 true, 다른 곳에서 키 입력 대기 등으로 변환 실패 시 false.
         */
        bool SetKeyStrokeWhenPressEnter( void ) noexcept;

        /**
         * @brief Return current key stroke type is DirectStroke
         */
        bool IsKeyStrokeDirect( void ) noexcept
        { return this->is_key_stroke_direct; }

    // Private members -------------------------------------------------------------------------------------------------------
    private:
        Termios save_attr;
        bool    is_key_stroke_direct;
        bool    await_force_stop_flg;
        int     await_timeout_milsec;
        mutable std::mutex mtx;
    }; // cls: Key

} // nsp::cx
#endif