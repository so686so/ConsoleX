#ifndef _CONSOLE_X_DEVICE_HPP_
#define _CONSOLE_X_DEVICE_HPP_

/** ------------------------------------------------------------------------------------
 *  ConsoleX Device Input Module
 *  ------------------------------------------------------------------------------------
 *  키보드/마우스 입력 처리, 시그널 핸들링, 스레드 안전한 입력 대기열 관리를 담당합니다.
 *  화면 출력(Output)과 관련된 기능은 cx::Screen 모듈로 분리되었습니다.
 *  ------------------------------------------------------------------------------------ */

// Coord, TermSize 정의 사용
#include "cx_screen.hpp"

// System & STL Includes
#include <termios.h> // termios struct
#include <unistd.h>  // read, write
#include <signal.h>  // sigaction
#include <memory>    // std::unique_ptr
#include <mutex>     // std::mutex
#include <chrono>    // std::chrono
#include <optional>  // std::optional
#include <string>    // std::string
#include <atomic>    // std::atomic
#include <vector>    // std::vector
#include <future>    // std::promise, std::future

namespace cx
{
    // =========================================================================
    // Input Enums & Structures
    // =========================================================================

    /**
     * @brief 통합 입력 코드 (키보드, 마우스, 시스템 이벤트)
     */
    enum class DeviceInputCode : int
    {
        // --- Meta Signals ---
        NONE         = -1,   // 입력 없음 / 타임아웃
        INTERRUPT    = -2,   // ForcePause / Signal에 의한 중단
        BUSY         = -3,   // 다른 스레드가 이미 입력을 점유 중임

        // --- Events ---
        MOUSE_EVENT  = 2000, // 마우스 동작
        RESIZE_EVENT = 3000, // 터미널 크기 변경 (SIGWINCH)
        CURSOR_EVENT = 4000, // 커서 위치 응답 (내부 처리용)

        // --- Standard Keys ---
        TAB = 9, ENTER = 10, ESC = 27, SPACE = 32, BACKSPACE = 127,

        // --- Numbers ---
        NUM_0 = 48, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9 = 57,

        // --- Alphabets ---
        A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        a = 97, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,

        // --- Special Keys ---
        ARROW_UP = 1001, ARROW_DOWN, ARROW_RIGHT, ARROW_LEFT,
        INSERT = 1005, DEL, HOME, END, PAGE_UP, PAGE_DOWN,

        // --- Function Keys ---
        F1 = 1011, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
    };
    using Key = DeviceInputCode;

    enum class MouseButton { LEFT, MIDDLE, RIGHT, UNKNOWN };
    enum class MouseAction { PRESS, DRAG, RELEASE, WHEEL_UP, WHEEL_DOWN, UNKNOWN };

    struct MouseState {
        int x = 0; int y = 0;
        MouseButton button = MouseButton::UNKNOWN;
        MouseAction action = MouseAction::UNKNOWN;
    };

    // =========================================================================
    // Device Class (Singleton)
    // =========================================================================

    /**
     * @brief 콘솔 입력 장치 제어 클래스
     * @details Singleton 패턴. 입력 버퍼링, Raw Mode 전환, 시그널 처리 담당.
     */
    class Device
    {
    public:
        /**
         * @brief 입력 결과와 상세 데이터를 포함하는 통합 이벤트 구조체
         *
         * @details
         *   이 구조체는 유니온(Union)과 유사한 방식으로 사용됩니다.
         *   모든 필드가 항상 유효한 것이 아니며,
         *   반드시 `code` 값을 먼저 확인한 후
         *   그에 맞는 필드에만 접근해야 합니다.
         *
         * @details
         *   데이터 흐름: Device Input Pipeline -> Parse -> Event Struct
         */
        struct Event
        {
            // 이벤트의 종류 (가장 먼저 확인해야 함!)
            DeviceInputCode code = DeviceInputCode::NONE;

            // --- Context-Dependent Data (Mutually Exclusive) ---
            MouseState mouse     = {}; // 유효 조건: code == MOUSE_EVENT (그 외엔 쓰레기값 혹은 0)
            TermSize   term_size = {}; // 유효 조건: code == RESIZE_EVENT
            Coord      cursor    = {}; // 유효 조건: code == CURSOR_EVENT (동기 요청의 응답)

            // --- Helper Predicates (Safe Check) ---
            bool IsTimeout( void ) const { return code == DeviceInputCode::NONE;         } // 타임아웃이나 잘못된 입력인지 확인
            bool IsMouse  ( void ) const { return code == DeviceInputCode::MOUSE_EVENT;  } // 마우스 이벤트인지 확인   ( mouse 필드 접근 가능 )
            bool IsResize ( void ) const { return code == DeviceInputCode::RESIZE_EVENT; } // 리사이즈 이벤트인지 확인 ( term_size 필드 접근 가능 )
            bool IsCursor ( void ) const { return code == DeviceInputCode::CURSOR_EVENT; } // 커서 위치 응답인지 확인  ( cursor 필드 접근 가능 )
        };

    // --- Public Static API ---------------------------------------------------
    public:
        /**
         * @brief [Blocking] 입력을 무한 대기합니다.
         */
        static DeviceInputCode GetInput( void );

        /**
         * @brief  [Timeout] 지정된 시간 동안 입력을 대기합니다.
         * @return 값이 있으면 DeviceInputCode, 타임아웃 시 std::nullopt
         */
        template <typename Rep, typename Period>
        static std::optional<DeviceInputCode> GetInput( const std::chrono::duration<Rep, Period>& duration )
        {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>( duration ).count();
            return GetPtr()->GetInputMs( (int)ms );
        }

        /**
         * @brief GetInput 결과 코드를 상세 이벤트 객체(Event)로 변환합니다.
         */
        static Event Inspect( const std::optional<DeviceInputCode>& opt_key );

        /**
         * @brief   [Thread-Safe] 현재 커서 위치를 동기적으로 요청하여 반환합니다.
         * @details 내부적으로 입력 루프와 연동하여 커서 응답을 기다립니다.
         * @return  cx::Coord (성공 시), nullopt (타임아웃)
         */
        template <typename Rep, typename Period>
        static std::optional<Coord> GetCursorPos( const std::chrono::duration<Rep, Period>& timeout )
        {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>( timeout ).count();
            return GetPtr()->GetCursorPosMs( (int)ms );
        }

        /**
         * @brief   마우스 추적 모드 활성화/비활성화
         * @details 해당 값이 true로 활성화 되면, cx::Device의 GetInput에서
         * 마우스의 콘솔 조작 스트림을 읽어갈 수 있습니다.
         */
        static void EnableMouse( bool enable );

        static MouseState GetMouseState( void );

        // --- Flow Control ---
        static void ForcePause( void ); // Raw Mode 일시 해제
        static void Resume( void );     // Raw Mode 재진입
        static void Deinit( void );     // 리소스 정리

        // --- Utilities ---
        static int KeyToInt( const DeviceInputCode key );
        static std::string KeyToString( const DeviceInputCode key );

    // --- Internal Implementation ---------------------------------------------
    private:
        static Device* GetPtr( void );

        // hidden constructor for singleton pattern
        Device();

        // destroctor
        ~Device();

        // delete copy & move
        Device( const Device& ) = delete;
        Device& operator=( const Device& ) = delete;

        void Init( void );

        // 내부 입력 처리 (점유 플래그 관리)
        std::optional<DeviceInputCode> GetInputMs( const int timeout_ms );

        // Raw Mode 제어
        void SetRawModeWithLock( const bool enable );
        void SetRawMode( const bool enable );

        // 비상용 : 시그널 핸들러 및 소멸자용 터미널 복구 함수
        void ResetTerminalMode( void );

        // 파서
        std::pair<DeviceInputCode, size_t> ParseInputBuffer  ( const std::string& buf );
        std::pair<DeviceInputCode, size_t> ParseMouseSequence( const std::string& buf );

        // 시그널 핸들러
        static void HandleSignal( int sig );

        // 커서 위치 요청 내부 구현
        std::optional<Coord> GetCursorPosMs( const int timeout_ms );
        void RequestCursorPos( void );

    private:
        static constexpr uint64_t EVENT_CODE_INTERRUPT = 1;
        static constexpr uint64_t EVENT_CODE_RESIZE    = 2;

        int               event_fd_;
        struct termios    orig_termios_;
        std::atomic<bool> is_raw_mode_;
        std::mutex        mtx_;

        struct sigaction old_sa_winch_;
        struct sigaction old_sa_int_;

        MouseState  last_mouse_state_;
        Coord       last_cursor_pos_;
        bool        is_mouse_tracking_;
        std::string input_buf_;

        // Thread Safety
        std::atomic<bool>    is_input_running_;
        std::mutex           cursor_promise_mtx_;
        std::promise<Coord>* cursor_promise_;
    };

} // namespace cx

#endif // _CONSOLE_X_DEVICE_HPP_