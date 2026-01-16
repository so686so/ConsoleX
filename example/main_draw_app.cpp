#include "ConsoleX.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <ctime>
#include <sstream>
#include <functional>
#include <iomanip> // put_time

using namespace std::chrono_literals;

// =============================================================================
// [DrawApp Class] Application Logic & State
// =============================================================================

class DrawApp
{
public:
    DrawApp()
    {
        std::srand( std::time(nullptr) );
        UpdateBrushChar();
    }

    void Run()
    {
        cx::Screen::Clear();
        cx::Device::EnableMouse( true );

        // 초기 렌더링
        RenderUI();

        while( state_.is_running )
        {
            if( auto input_opt = cx::Device::GetInput( 1000ms ); input_opt ){
                ProcessInput( cx::Device::Inspect( input_opt ) );
            }
            RenderUI();
        }

        Cleanup();
    }

private:
    // --- Types & Constants ---
    enum class AppMode { BRUSH, ERASER, COLOR_INPUT };
    const std::string DENSITY_CHARS = ".:+*oO#@";

    struct AppState {
        AppMode   mode = AppMode::BRUSH;
        bool      is_running = true;

        // Brush
        int       brush_density_idx = 3;
        char      brush_char = '*';
        cx::Color current_color = cx::Color::White;
        bool      is_gradient_on = false;

        // Eraser
        int       eraser_size = 3;
        std::vector<cx::Coord> eraser_blocks;
        std::vector<cx::Coord> eraser_trace; // 시각적 트레이스용

        // Input & UI
        std::string input_buffer = "";
        std::string last_key_msg = "Ready";
        cx::Coord   last_cursor_pos = {0, 0};
    };

    // UI 클릭 영역 구조체
    struct UIHitbox {
        int x_start;
        int x_end;
        int y;
        std::function<void()> action;
    };

    // --- Member Variables ---
    AppState state_;
    std::vector<UIHitbox> hitboxes_; // 현재 프레임의 클릭 가능 영역들
    cx::Coord press_pos_ = {-1, -1}; // 마우스 눌린 위치 저장

    // --- Helper Logic ---

    void Cleanup()
    {
        cx::Device::EnableMouse( false );
        cx::Screen::ResetColor();
        cx::Screen::Clear();
        std::cout << "Notepad App Terminated." << std::endl;
    }

    void UpdateBrushChar()
    {
        state_.brush_char = DENSITY_CHARS[state_.brush_density_idx];
    }

    void UpdateGradient()
    {
        if ( !state_.is_gradient_on ) return;

        cx::Rgb rgb = state_.current_color.GetRgb();
        auto GetDelta = []() { return ( std::rand() % 3 - 1 ) * 3; };

        int r = std::clamp( (int)rgb.r + GetDelta(), 0, 255 );
        int g = std::clamp( (int)rgb.g + GetDelta(), 0, 255 );
        int b = std::clamp( (int)rgb.b + GetDelta(), 0, 255 );

        state_.current_color = cx::Color( (uint8_t)r, (uint8_t)g, (uint8_t)b );
    }

    // 현재 시간 문자열 반환 (HH:MM:SS)
    std::string GetTimeString()
    {
        auto now = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
        std::stringstream ss;
        ss << std::put_time( std::localtime( &now ), "%H:%M:%S" );
        return ss.str();
    }

    // --- Action Methods ---

    void ActionDraw( int x, int y )
    {
        auto size = cx::Screen::GetSize();
        if ( y <= 1 || y >= size.rows ) return;

        UpdateGradient();

        cx::Screen::MoveCursor({ x, y });
        cx::Screen::SetColor( state_.current_color );
        std::cout << state_.brush_char;
        cx::Screen::ResetColor();
    }

    // [핵심 수정] 지우개 동작 - 차집합 연산으로 플리커링 제거
    void ActionErase( int center_x, int center_y )
    {
        auto size = cx::Screen::GetSize();

        // 1. 새로운 영역 계산 (New Blocks)
        std::vector<cx::Coord> new_blocks;

        int h = state_.eraser_size;
        int w = state_.eraser_size * 2; // 가로 보정

        int start_y = std::max(2, center_y - (h / 2));
        int end_y   = std::min(size.rows - 1, start_y + h - 1);
        int start_x = std::max(1, center_x - (w / 2));
        int end_x   = std::min(size.cols, start_x + w - 1);

        for( int y = start_y; y <= end_y; ++y ) {
            for( int x = start_x; x <= end_x; ++x ) {
                new_blocks.push_back({ x, y });
            }
        }

        // 2. [Diff Algorithm]
        // 사라져야 할 블록 (Old - New) -> 검은색(지움)
        cx::Screen::ResetColor();
        for( const auto& old_pos : state_.eraser_blocks )
        {
            // 새 영역에 포함되지 않은 구 영역만 지움
            bool keep = false;
            for( const auto& new_pos : new_blocks ) {
                if ( old_pos == new_pos ) { keep = true; break; }
            }

            if ( !keep ) {
                // UI 영역 보호
                if( old_pos.y > 1 && old_pos.y < size.rows ) {
                    cx::Screen::MoveCursor( old_pos );
                    std::cout << " ";
                }
            }
        }

        // 3. 새로 그려야 할 블록 (New - Old) -> 회색(Highlight)
        // 이미 그려진(Old에 포함된) 부분은 덧칠하지 않음 -> 플리커링 방지
        cx::Screen::SetBackColor( cx::Color(80, 80, 80) );
        cx::Screen::SetColor( cx::Color::Black );

        for( const auto& new_pos : new_blocks )
        {
            bool already_drawn = false;
            for( const auto& old_pos : state_.eraser_blocks ) {
                if ( old_pos == new_pos ) { already_drawn = true; break; }
            }

            if ( !already_drawn ) {
                cx::Screen::MoveCursor( new_pos );
                std::cout << " ";
            }

            // Trace에는 무조건 추가 (나중에 확정 지우기를 위해)
            state_.eraser_trace.push_back( new_pos );
        }

        cx::Screen::ResetColor();

        // 4. 상태 업데이트
        state_.eraser_blocks = new_blocks;
    }

    void ActionEraseRelease()
    {
        // Highlight(회색) 블록 제거 -> 검은색으로 덮기
        if ( !state_.eraser_blocks.empty() ) {
            cx::Screen::ResetColor();
            for( const auto& pos : state_.eraser_blocks ) {
                cx::Screen::MoveCursor( pos );
                std::cout << " ";
            }
            state_.eraser_blocks.clear();
        }

        // Trace 영역 실제 지우기 (Release 시점에 확정)
        // 이미 ActionErase에서 ' '를 출력했지만, 회색 배경이었을 수 있으므로
        // Release 시점에 확실하게 기본 배경색(검정) 공백으로 덮어씀.
        if ( !state_.eraser_trace.empty() ) {
            cx::Screen::ResetColor();
            for( const auto& pos : state_.eraser_trace ) {
                 auto size = cx::Screen::GetSize();
                 if( pos.y > 1 && pos.y < size.rows ) {
                    cx::Screen::MoveCursor( pos );
                    std::cout << " ";
                 }
            }
            state_.eraser_trace.clear();
        }
        std::cout << std::flush;
        state_.last_key_msg = "Eraser Applied";
    }

    // --- Input Handling ---

    void ProcessInput( const cx::Device::Event& event )
    {
        // 1. F4 Color Input Mode
        if ( state_.mode == AppMode::COLOR_INPUT )
        {
            HandleColorInput( event );
            return;
        }

        // 2. General Input
        switch( event.code )
        {
            case cx::DeviceInputCode::q:
                state_.is_running = false; break;
            case cx::DeviceInputCode::F1:
                SetMode( AppMode::BRUSH, "Mode: Brush" ); break;
            case cx::DeviceInputCode::F2:
                SetMode( AppMode::ERASER, "Mode: Eraser" ); break;
            case cx::DeviceInputCode::F3:
                ToggleGradient(); break;
            case cx::DeviceInputCode::F4:
                SetMode( AppMode::COLOR_INPUT, "Input Hex Color..." ); state_.input_buffer = ""; break;
            case cx::DeviceInputCode::RESIZE_EVENT:
                cx::Screen::Clear(); state_.last_key_msg = "Resized"; break;
            default:
                HandleHotKeys( event ); break;
        }

        // 3. Mouse Handling
        if ( event.code == cx::DeviceInputCode::MOUSE_EVENT )
        {
            HandleMouse( event.mouse );
        }
    }

    void SetMode( AppMode mode, const std::string& msg ) {
        // 모드 변경 시 이전 커서 잔상 정리
        if ( state_.mode == AppMode::ERASER && mode != AppMode::ERASER ) {
            ActionEraseRelease();
        }
        state_.mode = mode;
        state_.last_key_msg = msg;
    }

    void ToggleGradient() {
        state_.is_gradient_on = !state_.is_gradient_on;
        state_.last_key_msg = state_.is_gradient_on ? "Gradient ON" : "Gradient OFF";
    }

    void HandleColorInput( const cx::Device::Event& event )
    {
        if ( event.code == cx::DeviceInputCode::ESC ) {
            SetMode( AppMode::BRUSH, "Canceled" );
        }
        else if ( event.code == cx::DeviceInputCode::ENTER ) {
            cx::Color new_color( state_.input_buffer );
            if ( new_color.IsValid() ) {
                state_.current_color = new_color;
                SetMode( AppMode::BRUSH, "Applied #" + state_.input_buffer );
            } else {
                state_.last_key_msg = "Invalid Hex!";
            }
        }
        else if ( event.code == cx::DeviceInputCode::BACKSPACE ) {
            if ( !state_.input_buffer.empty() ) state_.input_buffer.pop_back();
        }
        else {
            std::string key_str = cx::Device::KeyToString( event.code );
            if ( key_str.length() == 1 && isxdigit(key_str[0]) ) {
                if ( state_.input_buffer.length() < 6 )
                    state_.input_buffer += (char)toupper(key_str[0]);
            }
        }
    }

    void HandleHotKeys( const cx::Device::Event& event )
    {
        std::string key_name = cx::Device::KeyToString( event.code );
        state_.last_key_msg = "Key: " + key_name;

        bool is_plus  = (key_name == "+" || key_name == "=" || key_name == "2");
        bool is_minus = (key_name == "-" || key_name == "_" || key_name == "1");

        if ( state_.mode == AppMode::BRUSH ) {
            if ( is_plus ) {
                state_.brush_density_idx = std::min( (int)DENSITY_CHARS.size()-1, state_.brush_density_idx + 1 );
                UpdateBrushChar();
                state_.last_key_msg = "Density Up";
            } else if ( is_minus ) {
                state_.brush_density_idx = std::max( 0, state_.brush_density_idx - 1 );
                UpdateBrushChar();
                state_.last_key_msg = "Density Down";
            }
        }
        else if ( state_.mode == AppMode::ERASER ) {
            if ( is_plus ) {
                state_.eraser_size = std::min( 10, state_.eraser_size + 1 );
                state_.last_key_msg = "Eraser Size Up";
            } else if ( is_minus ) {
                state_.eraser_size = std::max( 1, state_.eraser_size - 1 );
                state_.last_key_msg = "Eraser Size Down";
            }
        }
    }

    void HandleMouse( const cx::MouseState& mouse )
    {
        // UI 클릭 처리 (Left Button)
        if ( mouse.button == cx::MouseButton::LEFT )
        {
            if ( mouse.action == cx::MouseAction::PRESS ) {
                // 메뉴 Hitbox 검사를 위해 누른 위치 저장
                press_pos_ = { mouse.x, mouse.y };
            }
            else if ( mouse.action == cx::MouseAction::RELEASE ) {
                // 릴리즈 시, 눌렀던 곳과 뗀 곳이 같은 Hitbox 안이면 액션 실행
                CheckMenuClick( mouse.x, mouse.y );
                press_pos_ = { -1, -1 }; // Reset
            }
        }

        // 드로잉 처리 (UI 영역 아닐 때만)
        auto size = cx::Screen::GetSize();
        if ( mouse.y > 1 && mouse.y < size.rows )
        {
            state_.last_cursor_pos = { mouse.x, mouse.y };

            if ( mouse.button == cx::MouseButton::LEFT ) {
                if ( mouse.action == cx::MouseAction::PRESS || mouse.action == cx::MouseAction::DRAG ) {
                    if ( state_.mode == AppMode::BRUSH ) ActionDraw( mouse.x, mouse.y );
                    else if ( state_.mode == AppMode::ERASER ) ActionErase( mouse.x, mouse.y );
                }
                else if ( mouse.action == cx::MouseAction::RELEASE ) {
                    if ( state_.mode == AppMode::ERASER ) ActionEraseRelease();
                }
            }
            else if ( mouse.button == cx::MouseButton::MIDDLE && mouse.action == cx::MouseAction::PRESS ) {
                cx::Screen::Clear();
                state_.last_key_msg = "Screen Cleared";
                RenderUI(); // Clear 후 UI 복구
            }
        }
    }

    void CheckMenuClick( int release_x, int release_y )
    {
        if ( press_pos_.y != release_y ) return; // 같은 줄이어야 함

        for ( const auto& box : hitboxes_ ) {
            if ( box.y == release_y ) {
                // 눌렀던 곳과 뗀 곳이 모두 해당 박스 안에 있어야 함
                bool press_in = ( press_pos_.x >= box.x_start && press_pos_.x < box.x_end );
                bool release_in = ( release_x >= box.x_start && release_x < box.x_end );

                if ( press_in && release_in && box.action ) {
                    box.action();
                    return;
                }
            }
        }
    }

    // --- Rendering Methods ---

    void PrintLineBuffer( int y, const cx::Color& bg_color, const std::string& left_text, const std::string& right_text = "" )
    {
        auto size = cx::Screen::GetSize();
        cx::Screen::MoveCursor({ 1, y });
        cx::Screen::SetBackColor( bg_color );
        cx::Screen::SetColor( cx::Color::White );

        std::cout << left_text;

        size_t left_len  = cx::Util::StripAnsiCodes( left_text ).length();
        size_t right_len = cx::Util::StripAnsiCodes( right_text ).length();
        int padding = size.cols - (int)left_len - (int)right_len;

        cx::Screen::SetBackColor( bg_color );
        if ( padding > 0 ) std::cout << std::string( padding, ' ' );
        if ( right_len > 0 ) std::cout << right_text;

        cx::Screen::ResetColor();
    }

    void DrawTopBar()
    {
        // Hitbox 초기화
        hitboxes_.clear();

        std::stringstream ss;

        // 배경색
        auto bg_color = cx::Color{ 40, 40, 40 };
        std::string bg_ansi = bg_color.ToAnsiBackground();

        std::string style_active = "\033[1;32m" + bg_ansi;
        std::string style_normal = "\033[0;37m" + bg_ansi;
        std::string style_info   = "\033[1;36m" + bg_ansi;

        // 현재 그리는 텍스트의 누적 X 좌표 (1-based)
        int current_x = 1;

        // [Helper] 메뉴 아이템 추가 및 Hitbox 등록 함수
        auto AddMenu = [&]( const std::string& label, bool is_active, std::function<void()> onClick ) {
            std::string display_text;
            if ( is_active ) display_text = style_active + label + style_normal + " ";
            else             display_text = label + " ";

            // 스트림에 추가
            ss << display_text;

            // Hitbox 등록 (순수 길이 기준)
            int len = (int)cx::Util::StripAnsiCodes(display_text).length();

            // Hitbox 범위: [current_x, current_x + len)
            // onClick 액션이 있으면 등록
            if ( onClick ) {
                hitboxes_.push_back({ current_x, current_x + len, 1, onClick });
            }

            current_x += len;
        };

        // [Helper] 단순 텍스트 추가
        auto AddText = [&]( const std::string& text ) {
            ss << text;
            current_x += (int)cx::Util::StripAnsiCodes(text).length();
        };

        // --- Build Top Bar ---

        AddMenu( "[Q] Exit", false, [&](){ state_.is_running = false; } );

        AddText( "| " );
        AddMenu( "[F1] Brush", (state_.mode == AppMode::BRUSH), [&](){ SetMode(AppMode::BRUSH, "Mode: Brush"); } );

        AddText( "| " );
        AddMenu( "[F2] Eraser", (state_.mode == AppMode::ERASER), [&](){ SetMode(AppMode::ERASER, "Mode: Eraser"); } );

        AddText( "| " );
        std::string grad_txt = state_.is_gradient_on ? "[F3] Grad:ON " : "[F3] Grad:OFF";
        // Grad는 텍스트 색상을 다르게 처리
        std::string grad_display = state_.is_gradient_on ?
            ("\033[1;35m" + bg_ansi + grad_txt + style_normal + " ") : (grad_txt + " ");

        ss << grad_display;
        int grad_len = (int)cx::Util::StripAnsiCodes(grad_display).length();
        hitboxes_.push_back({ current_x, current_x + grad_len, 1, [&](){ ToggleGradient(); } });
        current_x += grad_len;

        AddText( "| " );
        AddMenu( "[F4] Color", (state_.mode == AppMode::COLOR_INPUT), [&](){ SetMode(AppMode::COLOR_INPUT, "Input Hex..."); state_.input_buffer = ""; } );

        AddText( "| " );
        if ( state_.mode == AppMode::BRUSH ) {
            ss << "Density: " << style_info << "-(1)/+(2)" << style_normal;
        }
        else if ( state_.mode == AppMode::ERASER ) {
            ss << "Size: " << style_info << "-(1)/+(2)" << style_normal;
        }

        // [Feature 2] 시간 출력 (우측 상단)
        std::string time_str = " Time: " + GetTimeString() + " ";

        PrintLineBuffer( 1, bg_color, ss.str(), time_str );
    }

    void DrawBottomBar()
    {
        auto size = cx::Screen::GetSize();
        std::stringstream left_ss;
        std::stringstream right_ss;

        cx::Color bar_bg_color(40, 40, 40);
        std::string bg_ansi = bar_bg_color.ToAnsiBackground();
        std::string style_normal = "\033[0;37m" + bg_ansi;

        if ( state_.mode == AppMode::COLOR_INPUT )
        {
            left_ss << " Input Hex: #";
            cx::Color temp_check( state_.input_buffer );
            std::string color_ansi = temp_check.IsValid() ? temp_check.ToAnsiForeground() : "\033[0;31m";

            left_ss << color_ansi << bg_ansi << state_.input_buffer << style_normal << "_ (Enter/Esc)";
        }
        else
        {
            if ( state_.mode == AppMode::BRUSH )      left_ss << " Info: Char('" << state_.brush_char << "')";
            else if ( state_.mode == AppMode::ERASER ) left_ss << " Info: Size(" << (state_.eraser_size * 2) << "x" << state_.eraser_size << ")";

            std::string cur_bg_ansi = state_.current_color.ToAnsiBackground();
            left_ss << " | Color: " << cur_bg_ansi << "  " << style_normal;
            left_ss << " | Msg: " << state_.last_key_msg;
        }

        if ( state_.last_cursor_pos.IsValid() ) right_ss << "Pos(" << state_.last_cursor_pos.x << ", " << state_.last_cursor_pos.y << ") ";
        else right_ss << "Pos(-, -) ";

        PrintLineBuffer( size.rows, bar_bg_color, left_ss.str(), right_ss.str() );
    }

    void RenderUI()
    {
        DrawTopBar();
        DrawBottomBar();
    }
};

// =============================================================================
// [Main] Entry Point
// =============================================================================

int main()
{
    DrawApp app;
    app.Run();
    return 0;
}