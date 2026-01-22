#include "cx_device.hpp"

// System Headers
#include <sys/eventfd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <iostream>
#include <cstring>
#include <algorithm>

namespace cx
{
    // 시그널 제어는 전역변수 없이는 힘듬
    // 전역변수가 꼭 필요한 매우매우 드문 케이스
    static int g_signal_event_fd = -1;

    // =========================================================================
    // Public Static Implementation
    // =========================================================================

    DeviceInputCode Device::GetInput( void )
    {
        // C++17 Style: 변수 선언과 조건 검사를 동시에 수행
        if( auto result = GetPtr()->GetInputMs( -1 ); result ){
            return *result;
        }
        return DeviceInputCode::NONE;
    }

    Device::Event Device::Inspect( const std::optional<DeviceInputCode>& opt_key )
    {
        // 값이 없으면 바로 NONE 반환
        if( !opt_key.has_value() ){
            return Event{ DeviceInputCode::NONE };
        }

        Event e {};
        e.code = opt_key.value();

        // 2. Enum 분기 처리는 switch가 가장 깔끔함
        switch( e.code )
        {
            case DeviceInputCode::MOUSE_EVENT:
                e.mouse = GetMouseState();
                break;

            case DeviceInputCode::RESIZE_EVENT:
                // case 구문 내부에서 변수를 만들어 사용한다면
                // 반드시 scope 처리를 해 줘야 함.
                {
                    cx::TermSize ts = cx::Screen::GetSize();
                    e.term_size = { ts.cols, ts.rows };
                }
                break;

            case DeviceInputCode::CURSOR_EVENT:
                e.cursor = GetPtr()->last_cursor_pos_;
                break;

            default:
                // 일반 키 입력(A, B, ENTER 등)은 추가 데이터가 없으므로 아무것도 안 함
                break;
        }

        return e;
    }

    void Device::ForcePause( void )
    {
        Device* instance = GetPtr();

        uint64_t u = EVENT_CODE_INTERRUPT;
        ssize_t  s = write( instance->event_fd_, &u, sizeof(uint64_t) ); (void)(s);

        instance->SetRawMode( false );
    }

    void Device::Resume( void )
    {
        GetPtr()->SetRawModeWithLock( true );
    }

    void Device::Deinit( void )
    {
        GetPtr()->SetRawModeWithLock( false );
    }

    MouseState Device::GetMouseState( void )
    {
        return GetPtr()->last_mouse_state_;
    }

    void Device::EnableMouse( bool enable )
    {
        auto ptr = GetPtr();

        ptr->is_mouse_tracking_ = enable;

        if( enable ){ std::cout << "\033[?1000h\033[?1002h\033[?1006h" << std::flush; }
        else        { std::cout << "\033[?1000l\033[?1002l\033[?1006l" << std::flush; }
    }

    int Device::KeyToInt( const DeviceInputCode key )
    {
        if( key >= DeviceInputCode::NUM_0 && key <= DeviceInputCode::NUM_9 )
            return static_cast<int>( key ) - static_cast<int>( DeviceInputCode::NUM_0 );
        return -1;
    }

    std::string Device::KeyToString( const DeviceInputCode key )
    {
        switch( key ) {
            case DeviceInputCode::INTERRUPT:    return "INTERRUPT";
            case DeviceInputCode::BUSY:         return "BUSY";
            case DeviceInputCode::RESIZE_EVENT: return "RESIZE_EVENT";
            case DeviceInputCode::MOUSE_EVENT:  return "MOUSE_EVENT";
            case DeviceInputCode::CURSOR_EVENT: return "CURSOR_EVENT";

            case DeviceInputCode::ENTER:        return "ENTER";
            case DeviceInputCode::ESC:          return "ESC";
            case DeviceInputCode::SPACE:        return "SPACE";
            case DeviceInputCode::TAB:          return "TAB";
            case DeviceInputCode::BACKSPACE:    return "BACKSPACE";

            case DeviceInputCode::ARROW_UP:     return "UP";
            case DeviceInputCode::ARROW_DOWN:   return "DOWN";
            case DeviceInputCode::ARROW_LEFT:   return "LEFT";
            case DeviceInputCode::ARROW_RIGHT:  return "RIGHT";

            case DeviceInputCode::INSERT:       return "INSERT";
            case DeviceInputCode::DEL:          return "DELETE";
            case DeviceInputCode::HOME:         return "HOME";
            case DeviceInputCode::END:          return "END";
            case DeviceInputCode::PAGE_UP:      return "PAGE_UP";
            case DeviceInputCode::PAGE_DOWN:    return "PAGE_DOWN";

            case DeviceInputCode::F1:  return "F1";  case DeviceInputCode::F2:  return "F2";
            case DeviceInputCode::F3:  return "F3";  case DeviceInputCode::F4:  return "F4";
            case DeviceInputCode::F5:  return "F5";  case DeviceInputCode::F6:  return "F6";
            case DeviceInputCode::F7:  return "F7";  case DeviceInputCode::F8:  return "F8";
            case DeviceInputCode::F9:  return "F9";  case DeviceInputCode::F10: return "F10";
            case DeviceInputCode::F11: return "F11"; case DeviceInputCode::F12: return "F12";
            default: break;
        }

        char c = static_cast<char>( key );
        if( std::isprint( c ) )
            return std::string( 1, c );

        return "UNKNOWN_KEY( " + std::to_string( (int)key ) + " )";
    }

    // =========================================================================
    // Private Implementation
    // =========================================================================

    Device* Device::GetPtr( void )
    {
        static Device instance;
        return &instance;
    }

    Device::Device()
        : event_fd_         ( -1      )
        , is_raw_mode_      ( false   )
        , is_mouse_tracking_( false   )
        , is_input_running_ ( false   )
        , cursor_promise_   ( nullptr )
    {
        input_buf_.reserve( 256 );

        memset( &old_sa_winch_, 0, sizeof(old_sa_winch_) );
        memset( &old_sa_int_,   0, sizeof(old_sa_int_)   );

        Init();
    }

    Device::~Device()
    {
        // rollback signal handle
        if( g_signal_event_fd != -1 ){ sigaction( SIGWINCH, &old_sa_winch_, nullptr ); }
        sigaction( SIGINT, &old_sa_int_, nullptr );

        ResetTerminalMode();

        if( event_fd_ != -1 ){ close( event_fd_ ); }

        g_signal_event_fd = -1;
    }

    void Device::ResetTerminalMode( void )
    {
        if( is_mouse_tracking_ ){
            const char* seq = "\033[?1000l\033[?1002l\033[?1006l";
            write( STDOUT_FILENO, seq, strlen(seq) );
        }

        if( is_raw_mode_ ){
            tcsetattr( STDIN_FILENO, TCSANOW, &orig_termios_ );
            const char* seq = "\033[?25h";
            write( STDOUT_FILENO, seq, strlen(seq) );
        }
    }

    void Device::HandleSignal( int sig )
    {
        if( sig == SIGWINCH && g_signal_event_fd != -1 )
        {
            uint64_t val = EVENT_CODE_RESIZE;
            write( g_signal_event_fd, &val, sizeof(uint64_t) );
        }
        else if( sig == SIGINT || sig == SIGTERM )
        {
            Device* instance = GetPtr();
            if( instance )
                instance->ResetTerminalMode();

            const char* nl = "\n";
            write( STDOUT_FILENO, nl, 1 );

            _exit( 0 );
        }
    }

    void Device::Init( void )
    {
        event_fd_ = eventfd( 0, EFD_NONBLOCK | EFD_CLOEXEC );
        if( event_fd_ == -1 ) perror( "cx::Device eventfd creation failed" );
        g_signal_event_fd = event_fd_;

        struct sigaction sa;
        sa.sa_flags = 0;
        sigemptyset( &sa.sa_mask );

        sa.sa_handler = HandleSignal;
        sigaction( SIGWINCH, &sa, &old_sa_winch_ );
        sigaction( SIGINT,   &sa, &old_sa_int_   );

        tcgetattr( STDIN_FILENO, &orig_termios_ );
        SetRawModeWithLock( true );
    }

    // =========================================================================
    // Core Input Logic
    // =========================================================================

    /**
     * @brief  지정된 시간(ms) 동안 입력을 대기하고 파싱합니다.
     * @param  timeout_ms 대기 시간 (음수: 무한 대기, 0: 즉시 리턴, 양수: 밀리초 대기)
     * @return std::optional<DeviceInputCode> (입력 발생 시 코드 반환, 타임아웃 시 nullopt)
     *
     * @details
     *   1. Atomic Flag를 사용하여 한 번에 하나의 스레드만 입력 처리에 진입하도록 보장합니다.
     *   2. 내부 버퍼(input_buf_)를 우선 검사하여 처리되지 않은 데이터가 있는지 확인합니다.
     *   3. select() 시스템 콜을 사용하여 키보드 입력(STDIN)과 시스템 이벤트(EventFD)를 동시에 감시합니다.
     */
    std::optional<DeviceInputCode> Device::GetInputMs( const int timeout_ms )
    {
        // ---------------------------------------------------------------------
        // 1. Thread Safety (The Gatekeeper)
        // ---------------------------------------------------------------------
        // 다른 스레드가 이미 GetInput을 실행 중인지 확인합니다.
        // CAS(Compare-And-Swap) 연산을 통해 lock-free 방식으로 진입을 제어합니다.
        bool expected = false;
        if( !is_input_running_.compare_exchange_strong( expected, true ) ){
            return DeviceInputCode::BUSY; // 이미 다른 스레드가 점유 중임
        }

        // RAII Pattern
        // 함수가 종료(return)되거나 예외가 발생하면
        // 자동으로 소멸자에서 is_input_running_ 플래그를
        // false로 해제합니다. (Deadlock 방지)
        struct BusyGuard {
            std::atomic<bool>& flag;
            ~BusyGuard(){ flag.store( false ); }
        } guard{ is_input_running_ };

        // ---------------------------------------------------------------------
        // 2. Ensure Raw Mode
        // ---------------------------------------------------------------------
        // 입력 처리를 위해서는 터미널이 Raw Mode(엔터 없이 입력, 에코 끔)여야 합니다.
        if( !is_raw_mode_ ){
            std::lock_guard<std::mutex> lock( mtx_ );
            if( !is_raw_mode_ ) SetRawMode( true );
        }

        using namespace std::chrono;
        auto start_time = steady_clock::now();

        // ---------------------------------------------------------------------
        // 3. Main Loop (Parse -> Select -> Read)
        // ---------------------------------------------------------------------
        while( true )
        {
            // A. 버퍼 처리 (Parsing)
            // 이전에 읽어둔 데이터(input_buf_)가 남아있다면 먼저 처리합니다.
            while( !input_buf_.empty() )
            {
                auto [ key, consumed_len ] = ParseInputBuffer( input_buf_ );

                // consumed_len == 0 은 "데이터가 부족하여 파싱 불가(Incomplete)"를 의미
                // 더 많은 데이터를 읽기 위해 select 단계로 이동
                if( consumed_len == 0 ) break;

                // 파싱 성공: 처리한 만큼 버퍼에서 제거
                input_buf_.erase( 0, consumed_len );

                // [Intercept Logic]
                // 커서 위치 응답(CURSOR_EVENT) 가로채기
                // 일반 사용자에게 전달하지 않고, GetCursorPos()를 호출해 대기 중인 스레드에게 전달
                if( key == DeviceInputCode::CURSOR_EVENT )
                {
                    std::lock_guard<std::mutex> lock( cursor_promise_mtx_ );
                    if( cursor_promise_ != nullptr )
                    {
                        // 값 전달; 약속 이행 완료
                        cursor_promise_->set_value( last_cursor_pos_ );
                        cursor_promise_ = nullptr;

                        // 사용자에겐 알리지 않고 다음 입력 처리
                        continue;
                    }
                    // scope 벗어나며 lock_guard 해제
                }

                // 일반 키 입력이면 즉시 반환
                if( key != DeviceInputCode::NONE ) return key;
            }

            // B. 남은 시간 계산 (Timeout Calculation)
            int current_timeout = -1;
            if( timeout_ms >= 0 )
            {
                auto elapsed    = duration_cast<milliseconds>( steady_clock::now() - start_time );
                current_timeout = timeout_ms - (int)elapsed.count();

                // 시간 초과 시 std::optional의 std::nullopt 반환
                if( current_timeout <= 0 )
                    return std::nullopt;
            }

            // C. I/O Multiplexing (select)
            fd_set readfds;
            FD_ZERO( &readfds );
            FD_SET( STDIN_FILENO, &readfds ); // 키보드 입력 감시
            FD_SET( event_fd_, &readfds );    // 시그널/강제종료 감시
            int max_fd = std::max( STDIN_FILENO, event_fd_ );

            struct timeval  tv;
            struct timeval* ptv = nullptr;
            if( current_timeout >= 0 )
            {
                tv.tv_sec  = ( current_timeout / 1000 );
                tv.tv_usec = ( current_timeout % 1000 ) * 1000;
                ptv = &tv;
            }

            // 커널에게 "데이터가 들어올 때까지" 혹은 "타임아웃까지" 대기 요청
            int activity = select( max_fd + 1, &readfds, nullptr, nullptr, ptv );

            // 시스템 에러
            if( activity < 0 && errno != EINTR )
                return std::nullopt;

            // 타임아웃 발생 [ESC 지연 해결 패치]
            if( activity == 0 )
            {
                // 버퍼에 ESC(27) 하나만 남아있고 타임아웃이 발생했다면,
                // 이는 시퀀스(화살표 키 등)를 기다리는 중이 아니라 단독 ESC 키 입력입니다.
                if( input_buf_.length() == 1 && input_buf_[0] == 27 )
                {
                    input_buf_.clear();
                    return DeviceInputCode::ESC;
                }

                return std::nullopt;
            }

            // D. Read Data
            bool data_read = false;

            // D-1. 시스템 이벤트 (EventFD)
            if( FD_ISSET( event_fd_, &readfds ) )
            {
                uint64_t u = 0;
                read( event_fd_, &u, sizeof(uint64_t) );

                if( u == EVENT_CODE_INTERRUPT ) return DeviceInputCode::INTERRUPT;
                if( u == EVENT_CODE_RESIZE    ) return DeviceInputCode::RESIZE_EVENT;

                data_read = true;
            }

            // D-2. 키보드/마우스 입력 (STDIN)
            if( FD_ISSET( STDIN_FILENO, &readfds ) )
            {
                char temp_buf[256];
                ssize_t len = read( STDIN_FILENO, temp_buf, sizeof(temp_buf) );

                if( len > 0 )
                {
                    input_buf_.append( temp_buf, len ); // 버퍼 뒤에 이어 붙이기
                    data_read = true;
                }
            }

            // 데이터를 읽었다면 루프의 처음(A. 버퍼 처리)으로 돌아가 파싱 시도
            if( data_read )
                continue;
        }
    }

    void Device::SetRawModeWithLock( const bool enable )
    {
        std::lock_guard<std::mutex> lock( mtx_ );
        SetRawMode( enable );
    }

    /**
     * @brief 터미널의 Raw Mode(원시 모드)를 활성화하거나 비활성화합니다.
     * @param enable true: Raw Mode 진입, false: 원래 모드(Cooked) 복구
     *
     * @details
     *   Raw Mode           : 사용자가 키를 누르는 즉시 프로그램으로 전달됨 (Enter 불필요).
     *   Cooked Mode (기본) : Enter를 누르기 전까지 커널이 입력을 버퍼링하고 편집(Backspace 등) 처리함.
     */
    void Device::SetRawMode( const bool enable )
    {
        if( is_raw_mode_ == enable )
            return;

        if( enable )
        {
            // 기존 설정 복사
            struct termios raw = orig_termios_;

            // [Flag 설명]
            // ECHO   : 입력한 키를 화면에 출력하지 않음 (비밀번호 칠 때처럼)
            // ICANON : Canonical Mode(줄 단위 버퍼링) 끄기 -> 바이트 단위 입력
            raw.c_lflag &= ~( ECHO | ICANON );

            // VMIN  = 1 : 최소 1바이트가 들어올 때까지 read()가 블로킹됨
            // VTIME = 0 : 타임아웃 없음
            raw.c_cc[VMIN]  = 1;
            raw.c_cc[VTIME] = 0;

            // 설정을 즉시(TCSANOW) 적용
            tcsetattr( STDIN_FILENO, TCSANOW, &raw );

            // 커서 숨기기 (\033[?25l) - UI 깔끔함을 위해
            const char* seq = "\033[?25l";
            write( STDOUT_FILENO, seq, strlen(seq) );

            is_raw_mode_ = true;
        }
        else
        {
            // 원래 설정으로 복구 (프로그램 종료 시 필수)
            tcsetattr( STDIN_FILENO, TCSANOW, &orig_termios_ );

            // 커서 다시 보이기 (\033[?25h)
            const char* seq = "\033[?25h";
            write( STDOUT_FILENO, seq, strlen(seq) );

            is_raw_mode_ = false;
        }
    }

    void Device::RequestCursorPos( void )
    {
        std::cout << "\033[6n" << std::flush;
    }

    /**
     * @brief 커서 위치 요청 시퀀스를 보내고 응답을 기다립니다.
     *
     * @details
     *   커서 위치를 얻으려면 터미널에 '\033[6n'을 쓰고, STDIN으로 들어오는 '\033[row;colR'을 읽어야 합니다.
     *
     *   1. Observer Mode:
     *      이미 다른 스레드(Main Loop)가 GetInput()으로 STDIN을 점유 중이라면,
     *      Promise/Future를 통해 그 스레드에게 "응답 오면 나한테 줘"라고 부탁하고 대기합니다.
     *
     *   2. Direct Mode:
     *      아무도 STDIN을 안 쓰고 있다면, 직접 GetInput()을 호출하여 응답을 읽어옵니다.
     */
    std::optional<Coord> Device::GetCursorPosMs( const int timeout_ms )
    {
        // 1. 터미널에 "너 지금 커서 어디야?"라고 질문 전송 (Device Status Report)
        RequestCursorPos(); // sends "\033[6n"

        // 2. 실행 모드 결정: 누가 입력 버퍼(STDIN)를 쥐고 있는가?
        if( is_input_running_.load() )
        {
            // [Case A: Observer Mode] 이미 입력 루프가 돌고 있음
            std::promise<Coord> p;
            std::future<Coord>  f = p.get_future();

            {
                std::lock_guard<std::mutex> lock( cursor_promise_mtx_ );

                // 이미 다른 요청이 진행 중임 (Busy)
                if( cursor_promise_ != nullptr )
                    return std::nullopt;

                // "결과 나오면 여기다 넣어줘" 라고 등록
                cursor_promise_ = &p;
            }

            // 결과가 도착할 때까지 대기
            std::future_status status = f.wait_for( std::chrono::milliseconds( timeout_ms ) );

            // 대기 끝, 포인터 해제 (중요: Dangling Pointer 방지)
            {
                std::lock_guard<std::mutex> lock( cursor_promise_mtx_ );
                cursor_promise_ = nullptr;
            }

            if( status == std::future_status::ready ){
                return f.get();
            }
            else { return std::nullopt; } // Timeout
        }
        else
        {
            // [Case B: Direct Mode] 내가 직접 읽어야 함
            using namespace std::chrono;
            auto start_time = steady_clock::now();

            while( true )
            {
                // 남은 시간 계산
                auto elapsed   = duration_cast<milliseconds>( steady_clock::now() - start_time );
                int  remaining = timeout_ms - (int)elapsed.count();

                if( remaining <= 0 ) break;

                // 재귀적으로 GetInputMs를 호출하지만, 이번엔 내가 점유자(Owner)가 됨
                auto input_opt = GetInputMs( remaining );

                if( !input_opt.has_value() ) continue;

                // 내가 원하던 커서 이벤트가 맞나?
                // (이때 키보드나 마우스 입력이 오면 무시/유실될 수 있음 -> Direct Mode의 한계)
                if( input_opt.value() == DeviceInputCode::CURSOR_EVENT ) {
                    return last_cursor_pos_;
                }
            }
            return std::nullopt;
        }
    }

    /**
     * @brief  입력 버퍼의 앞부분을 분석하여 의미 있는 키 코드로 변환합니다.
     * @param  buf 입력된 로우 데이터 버퍼
     * @return {식별된 키 코드, 사용된 바이트 길이}
     * 길이가 0이면 "데이터가 더 필요함(Incomplete)"을 의미합니다.
     */
    std::pair<DeviceInputCode, size_t> Device::ParseInputBuffer( const std::string& buf )
    {
        if( buf.empty() )
            return { DeviceInputCode::NONE, 0 };

        size_t len = buf.length();

        // ESC(27)로 시작하는 시퀀스 처리 (ESC Key, Arrow Keys, F-Keys, Mouse)
        if( buf[0] == static_cast<int>( cx::DeviceInputCode::ESC ) )
        {
            // 데이터가 너무 짧으면 더 기다려야 함 (최소 2바이트 필요)
            if( len < 2 )
                return { DeviceInputCode::NONE, 0 };

            // '[' (CSI - Control Sequence Introducer)
            if( buf[1] == '[' )
            {
                if( len < 3 )
                    return { DeviceInputCode::NONE, 0 };

                // Mouse Event (SGR Mode: \033[<...)
                if( buf[2] == '<' )
                    return ParseMouseSequence( buf );

                // Focus Event (I/O) - 무시하고 넘어감 (3바이트 소비)
                if( buf[2] == 'I' || buf[2] == 'O' )
                    return { DeviceInputCode::NONE, 3 };

                // 숫자 시작 (F-Key, Insert/Del, Cursor Pos) : \033[15~ or \033[24;1R
                if( buf[2] >= '0' && buf[2] <= '9' )
                {
                    // 종료 문자(~, R 등)를 찾음
                    size_t t_pos = std::string::npos;
                    for( size_t i = 2; i < len; ++i )
                    {
                        // ASCII 0x40(@) ~ 0x7E(~) 사이의 문자가 터미네이터임
                        if( buf[i] >= 0x40 && buf[i] <= 0x7E ) { t_pos = i; break; }
                    }

                    if( t_pos == std::string::npos ) // 아직 덜 옴
                        return { DeviceInputCode::NONE, 0 };

                    size_t seq_len    = t_pos + 1;
                    char   terminator = buf[t_pos];

                    // 'R': Cursor Position Report (\033[row;colR)
                    if( terminator == 'R' )
                    {
                        int r = 0, c = 0;
                        size_t semi_pos = buf.find( ';', 2 );

                        if( semi_pos != std::string::npos && semi_pos < t_pos )
                        {
                            try {
                                // 문자열 파싱 ( Row;Col )
                                r = std::stoi( buf.substr( 2, semi_pos - 2 ) );
                                c = std::stoi( buf.substr( semi_pos + 1, t_pos - (semi_pos + 1) ) );
                            }
                            catch(...) { return { DeviceInputCode::NONE, seq_len }; }

                            last_cursor_pos_.x = c;
                            last_cursor_pos_.y = r;

                            return { DeviceInputCode::CURSOR_EVENT, seq_len };
                        }
                    }

                    // '~': Extended Keys (Home, End, F1~F12, etc.)
                    if( terminator == '~' )
                    {
                        DeviceInputCode k = DeviceInputCode::NONE;

                        // VT100/xterm 호환 키 매핑 로직
                        // 예: \033[15~ -> F5
                        if( buf[2] == '1' ) {
                            // [NEW] Tera Term 등에서 F1~F4를 [11~ 스타일로 보내는 경우 처리
                            std::string sub = buf.substr(2, 2);
                            if      ( sub == "11" ) k = DeviceInputCode::F1; // \033[11~
                            else if ( sub == "12" ) k = DeviceInputCode::F2; // \033[12~
                            else if ( sub == "13" ) k = DeviceInputCode::F3; // \033[13~
                            else if ( sub == "14" ) k = DeviceInputCode::F4; // \033[14~

                            // [Existing] F5 이상
                            else if ( sub == "15" ) k = DeviceInputCode::F5;
                            else if ( sub == "17" ) k = DeviceInputCode::F6;
                            else if ( sub == "18" ) k = DeviceInputCode::F7;
                            else if ( sub == "19" ) k = DeviceInputCode::F8;
                            else if ( sub == "1~" ) k = DeviceInputCode::HOME; // \033[1~ case
                        }
                        else if( buf[2] == '2' ) {
                            std::string sub = buf.substr(2, 2);
                            if      ( sub == "20" ) k = DeviceInputCode::F9;
                            else if ( sub == "21" ) k = DeviceInputCode::F10;
                            else if ( sub == "23" ) k = DeviceInputCode::F11;
                            else if ( sub == "24" ) k = DeviceInputCode::F12;
                            else if ( sub == "2~" ) k = DeviceInputCode::INSERT;
                        }
                        else if( buf[2] == '3' ) k = DeviceInputCode::DEL;
                        else if( buf[2] == '4' ) k = DeviceInputCode::END;
                        else if( buf[2] == '5' ) k = DeviceInputCode::PAGE_UP;
                        else if( buf[2] == '6' ) k = DeviceInputCode::PAGE_DOWN;

                        return { k, seq_len };
                    }
                    // 알 수 없는 시퀀스는 소비하고 무시함
                    return { DeviceInputCode::NONE, seq_len };
                }

                // 문자 커맨드 (화살표 키 등) : \033[A
                switch( buf[2] )
                {
                    case 'A': return { DeviceInputCode::ARROW_UP,    3 };
                    case 'B': return { DeviceInputCode::ARROW_DOWN,  3 };
                    case 'C': return { DeviceInputCode::ARROW_RIGHT, 3 };
                    case 'D': return { DeviceInputCode::ARROW_LEFT,  3 };
                    case 'H': return { DeviceInputCode::HOME,        3 };
                    case 'F': return { DeviceInputCode::END,         3 };
                }
            }
            // SS3 (Single Shift 3) - \033O... (F1~F4)
            else if( buf[1] == 'O' )
            {
                if( len < 3 )
                    return { DeviceInputCode::NONE, 0 };

                switch( buf[2] )
                {
                    case 'P': return { DeviceInputCode::F1,   3 };
                    case 'Q': return { DeviceInputCode::F2,   3 };
                    case 'R': return { DeviceInputCode::F3,   3 };
                    case 'S': return { DeviceInputCode::F4,   3 };
                    case 'H': return { DeviceInputCode::HOME, 3 };
                    case 'F': return { DeviceInputCode::END,  3 };
                }
            }
            // 그냥 ESC 키 하나만 눌린 경우 (매우 빠르게 판단해야 함)
            if( len == 1 )
                return { DeviceInputCode::ESC, 1 };
        }

        // 일반 ASCII 문자 (A, a, 1, Enter...)
        unsigned char c = static_cast<unsigned char>( buf[0] );

        // Backspace 처리: ASCII 8(^H) 또는 127(^?)
        // Tera Term은 8을 보내고, 다른 터미널은 127을 보내기도 함
        if( c == 8 || c == 127 ) {
            return { DeviceInputCode::BACKSPACE, 1 };
        }

        // Enter 처리: ASCII 10(\n) 또는 13(\r)
        // (보통 DeviceInputCode::ENTER가 정의되어 있다면 매핑)
        if( c == 10 || c == 13 ) {
             return { DeviceInputCode::ENTER, 1 };
        }

        // Tab 처리: ASCII 9(\t)
        if( c == 9 ) {
             return { DeviceInputCode::TAB, 1 }; // TAB이 정의되어 있다면
        }

        // 그 외 일반 ASCII 문자 (A, a, 1, ...)
        return { static_cast<DeviceInputCode>( c ), 1 };
    }

    /**
     * @brief SGR 1006 마우스 시퀀스를 파싱하여 사용자 친화적인 이벤트로 변환합니다.
     *
     * @details
     *   Format : \033[<BUTTON;X;Y;TYPE
     *   BUTTON : 비트마스크 (왼/오/휠/드래그 정보 포함)
     *   TYPE   : 'M' (누름/이동), 'm' (뗌)
     */
    std::pair<DeviceInputCode, size_t> Device::ParseMouseSequence( const std::string& buf )
    {
        // 1. 'M' 또는 'm' 종료 문자 찾기
        size_t m_pos = std::string::npos;
        for( size_t i = 3; i < buf.length(); ++i ){
            if( buf[i] == 'M' || buf[i] == 'm' ) { m_pos = i; break; }
        }

        if( m_pos == std::string::npos ) // 데이터 불충분
            return { DeviceInputCode::NONE, 0 };

        size_t seq_len = m_pos + 1;

        // 2. 파라미터(숫자) 파싱: BUTTON ; X ; Y
        int param[3] = {0, 0, 0};
        int p_idx    = 0;
        for( size_t i = 3; i < m_pos; ++i )
        {
            if     ( buf[i] == ';' )                  { if( ++p_idx > 2 ) break; }
            else if( buf[i] >= '0' && buf[i] <= '9' ) { param[p_idx] = param[p_idx] * 10 + ( buf[i] - '0' ); }
        }

        int raw_btn         = param[0];
        last_mouse_state_.x = param[1];
        last_mouse_state_.y = param[2];

        // 'm': Release, 'M': Press/Drag
        char last_char = buf[m_pos];

        // ---------------------------------------------------------------------
        // 표준 프로토콜 값 -> cx::Device 논리 구조 매핑
        // ---------------------------------------------------------------------

        // Case A: 휠 이벤트 (64번 비트 설정됨)
        // 휠은 "누르고 있는" 상태가 없으므로 항상 Action(WHEEL_UP/DOWN)으로 처리
        if( raw_btn >= 64 )
        {
            last_mouse_state_.button = MouseButton::UNKNOWN;

            if      ( raw_btn == 64 ) last_mouse_state_.action = MouseAction::WHEEL_UP;
            else if ( raw_btn == 65 ) last_mouse_state_.action = MouseAction::WHEEL_DOWN;
            else                      last_mouse_state_.action = MouseAction::UNKNOWN;
        }
        // Case B: 버튼 뗌 (Release) - SGR 모드는 어떤 버튼을 뗐는지 알려줌
        else if( last_char == 'm' )
        {
            last_mouse_state_.action = MouseAction::RELEASE;

            // 하위 2비트로 버튼 식별 (0:Left, 1:Middle, 2:Right)
            switch( raw_btn & 3 )
            {
                case 0:  last_mouse_state_.button = MouseButton::LEFT;    break;
                case 1:  last_mouse_state_.button = MouseButton::MIDDLE;  break;
                case 2:  last_mouse_state_.button = MouseButton::RIGHT;   break;
                default: last_mouse_state_.button = MouseButton::UNKNOWN; break;
            }
        }
        // Case C: 누름(Press) 혹은 드래그(Drag)
        else
        {
            // 32번 비트가 1이면 드래그 중인 상태임
            if( raw_btn & 32 )
            {
                last_mouse_state_.action = MouseAction::DRAG;
                raw_btn -= 32; // 버튼 ID 추출을 위해 드래그 비트 제거
            }
            else
            {
                last_mouse_state_.action = MouseAction::PRESS;
            }

            switch( raw_btn & 3 )
            {
                case 0:  last_mouse_state_.button = MouseButton::LEFT;    break;
                case 1:  last_mouse_state_.button = MouseButton::MIDDLE;  break;
                case 2:  last_mouse_state_.button = MouseButton::RIGHT;   break;
                default: last_mouse_state_.button = MouseButton::UNKNOWN; break;
            }
        }

        return { DeviceInputCode::MOUSE_EVENT, seq_len };
    }
}