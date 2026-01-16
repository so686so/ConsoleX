#include "ConsoleX.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <ctime>
#include <sstream>
#include <functional>
#include <iomanip>

using namespace std::chrono_literals;

// =============================================================================
// [DrawApp Class] Application Logic & State
// =============================================================================

// [New] 캔버스에 저장될 픽셀 정보
struct CanvasPixel {
    std::string ch = " ";
    cx::Color fg = cx::Color::White;
    cx::Color bg = cx::Color::Black;
};

class DrawApp
{
public:
    DrawApp()
    {
        std::srand( std::time(nullptr) );
        UpdateBrushChar();
        // 초기 캔버스 사이즈 설정 (임시)
        ResizeCanvas(100, 50);
    }

    void Run()
    {
        cx::Device::EnableMouse( true );

        // 초기 검은 화면 설정
        cx::Screen::SetBackColor(cx::Color::Black);
        cx::Screen::Clear();

        // 최초 1회 렌더링
        Render();

        while( state_.is_running )
        {
            // 입력이 있거나 렌더링 요청이 있을 때만 루프
            if( auto input_opt = cx::Device::GetInput( 10ms ); input_opt ){
                ProcessInput( cx::Device::Inspect( input_opt ) );
            }

            Render();
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
        // [Refactor] 복잡한 Trace 벡터 제거됨 (Buffer가 알아서 처리)

        // Input & UI
        std::string input_buffer = "";
        std::string last_key_msg = "Ready";
    };

    struct UIHitbox {
        int x, w;
        std::function<void()> action;
    };

    // --- Member Variables ---
    AppState state_;

    // [New] Rendering Core
    cx::Buffer screen_buffer_;
    std::vector<std::vector<CanvasPixel>> canvas_; // 그림 데이터 저장소

    std::vector<UIHitbox> hitboxes_;
    cx::Coord press_pos_ = {-1, -1};
    cx::Coord mouse_cursor_ = {0, 0}; // 0-based 좌표

    bool is_mouse_down_ = false;

    // --- Helper Logic ---

    void Cleanup()
    {
        cx::Device::EnableMouse( false );
        cx::Screen::ResetColor();
        cx::Screen::Clear();
        std::cout << "DrawApp Terminated." << std::endl;
    }

    void ResizeCanvas(int w, int h)
    {
        // 기존 그림 유지하면서 리사이즈
        if ( (int)canvas_.size() == h && (int)canvas_[0].size() == w ) return;

        std::vector<std::vector<CanvasPixel>> new_canvas(h, std::vector<CanvasPixel>(w));

        // Copy old data
        int copy_h = std::min(h, (int)canvas_.size());
        if (copy_h > 0) {
            int copy_w = std::min(w, (int)canvas_[0].size());
            for(int y=0; y<copy_h; ++y) {
                for(int x=0; x<copy_w; ++x) {
                    new_canvas[y][x] = canvas_[y][x];
                }
            }
        }
        canvas_ = new_canvas;
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

    std::string GetTimeString()
    {
        auto now = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
        std::stringstream ss;
        ss << std::put_time( std::localtime( &now ), "%H:%M:%S" );
        return ss.str();
    }

    // --- Action Methods ---

    // [New] 화면에 직접 찍지 않고 Canvas 데이터만 변경
    void ActionDraw( int x, int y )
    {
        if ( y < 0 || y >= (int)canvas_.size() || x < 0 || x >= (int)canvas_[0].size() ) return;

        // UI 영역 보호 (상단 1줄, 하단 1줄)
        auto size = cx::Screen::GetSize();
        if (y < 1 || y >= size.rows - 1) return;

        UpdateGradient();

        canvas_[y][x].ch = state_.brush_char;
        canvas_[y][x].fg = state_.current_color;
        canvas_[y][x].bg = cx::Color::Black; // 배경은 기본 검정
    }

    void ActionErase( int center_x, int center_y )
    {
        int h = state_.eraser_size;
        int w = state_.eraser_size * 2;
        auto size = cx::Screen::GetSize();

        int start_y = center_y - (h / 2);
        int end_y   = start_y + h - 1;
        int start_x = center_x - (w / 2);
        int end_x   = start_x + w - 1;

        // Canvas 범위 내에서 데이터를 " " (공백)으로 초기화
        for( int y = start_y; y <= end_y; ++y ) {
            for( int x = start_x; x <= end_x; ++x ) {
                // 경계 및 UI 보호
                if ( y > 0 && y < size.rows - 1 &&
                     y >= 0 && y < (int)canvas_.size() &&
                     x >= 0 && x < (int)canvas_[0].size() )
                {
                    canvas_[y][x].ch = " ";
                    canvas_[y][x].fg = cx::Color::White;
                    canvas_[y][x].bg = cx::Color::Black;
                }
            }
        }
    }

    void ClearCanvas() {
        for(auto& row : canvas_) {
            for(auto& px : row) {
                px.ch = " ";
                px.bg = cx::Color::Black;
            }
        }
    }

    // --- Input Handling ---

    void ProcessInput( const cx::Device::Event& event )
    {
        // 0. 마우스 좌표 동기화 (1-based -> 0-based)
        if (event.code == cx::DeviceInputCode::MOUSE_EVENT) {
            mouse_cursor_ = { event.mouse.x - 1, event.mouse.y - 1 };
        }

        // 1. F4 Color Input Mode
        if ( state_.mode == AppMode::COLOR_INPUT ) {
            HandleColorInput( event );
            return;
        }

        // 2. General Input
        switch( event.code ) {
            case cx::DeviceInputCode::q: state_.is_running = false; break;
            case cx::DeviceInputCode::F1: SetMode( AppMode::BRUSH, "Mode: Brush" ); break;
            case cx::DeviceInputCode::F2: SetMode( AppMode::ERASER, "Mode: Eraser" ); break;
            case cx::DeviceInputCode::F3: ToggleGradient(); break;
            case cx::DeviceInputCode::F4: SetMode( AppMode::COLOR_INPUT, "Input Hex..." ); state_.input_buffer = ""; break;
            case cx::DeviceInputCode::RESIZE_EVENT:
                // Buffer & Canvas 리사이즈는 Render 루프에서 처리
                state_.last_key_msg = "Resized";
                break;
            default: HandleHotKeys( event ); break;
        }

        // 3. Mouse Handling
        if ( event.code == cx::DeviceInputCode::MOUSE_EVENT ) {
            HandleMouse( event.mouse );
        }
    }

    void SetMode( AppMode mode, const std::string& msg ) {
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
                state_.last_key_msg = "Size Up";
            } else if ( is_minus ) {
                state_.eraser_size = std::max( 1, state_.eraser_size - 1 );
                state_.last_key_msg = "Size Down";
            }
        }
    }

    void HandleMouse( const cx::MouseState& mouse )
    {
        int mx = mouse_cursor_.x;
        int my = mouse_cursor_.y;

        // [Fix 2] 마우스 버튼 상태 추적 로직 추가
        if ( mouse.button == cx::MouseButton::LEFT ) {
            if ( mouse.action == cx::MouseAction::PRESS ) {
                is_mouse_down_ = true;
            } else if ( mouse.action == cx::MouseAction::RELEASE ) {
                is_mouse_down_ = false;
            }
        }

        // UI Click (Top Bar)
        if ( mouse.button == cx::MouseButton::LEFT && mouse.action == cx::MouseAction::PRESS ) {
            if ( my == 0 ) {
                CheckMenuClick( mx, my );
                return;
            }
        }

        // Drawing Area
        auto size = cx::Screen::GetSize();
        if ( my > 0 && my < size.rows - 1 ) {
            if ( mouse.button == cx::MouseButton::LEFT ) {
                if ( mouse.action == cx::MouseAction::PRESS || mouse.action == cx::MouseAction::DRAG ) {
                    if ( state_.mode == AppMode::BRUSH ) ActionDraw( mx, my );
                    else if ( state_.mode == AppMode::ERASER ) ActionErase( mx, my );
                }
            }
            else if ( mouse.button == cx::MouseButton::MIDDLE && mouse.action == cx::MouseAction::PRESS ) {
                ClearCanvas();
                state_.last_key_msg = "Canvas Cleared";
            }
        }
    }

    void CheckMenuClick( int release_x, int release_y )
    {
        for ( const auto& box : hitboxes_ ) {
            if ( release_x >= box.x && release_x < box.x + box.w ) {
                if ( box.action ) box.action();
                return;
            }
        }
    }

    // --- Rendering Methods ---

    void DrawTopBar()
    {
        hitboxes_.clear();
        cx::Color bg = cx::Color{ 40, 40, 40 };
        cx::Color fg = cx::Color::White;

        auto size = cx::Screen::GetSize();
        // 배경 채우기
        for(int x=0; x<size.cols; ++x) screen_buffer_.DrawString(x, 0, " ", fg, bg);

        int current_x = 1;

        auto AddMenu = [&]( std::string label, bool active, std::function<void()> act ) {
            std::string txt = " " + label + " ";
            cx::Color item_fg = active ? cx::Color::Green : cx::Color::White;

            // Draw text
            screen_buffer_.DrawString(current_x, 0, txt, item_fg, bg);

            int len = (int)cx::Util::GetStringWidth(txt);
            if ( act ) hitboxes_.push_back({ current_x, len, act });
            current_x += len;

            screen_buffer_.DrawString(current_x++, 0, "|", fg, bg);
        };

        AddMenu("[Q] Exit", false, [&](){ state_.is_running = false; });
        AddMenu("[F1] Brush", state_.mode == AppMode::BRUSH, [&](){ SetMode(AppMode::BRUSH, "Mode: Brush"); });
        AddMenu("[F2] Eraser", state_.mode == AppMode::ERASER, [&](){ SetMode(AppMode::ERASER, "Mode: Eraser"); });

        std::string grad_txt = state_.is_gradient_on ? "[F3] Grad:ON " : "[F3] Grad:OFF";
        AddMenu(grad_txt, state_.is_gradient_on, [&](){ ToggleGradient(); });

        AddMenu("[F4] Color", state_.mode == AppMode::COLOR_INPUT, [&](){ SetMode(AppMode::COLOR_INPUT, "Input..."); state_.input_buffer=""; });

        // Info Text
        std::stringstream ss;
        if ( state_.mode == AppMode::BRUSH ) ss << " Dens :" << (state_.brush_density_idx+1);
        else if ( state_.mode == AppMode::ERASER ) ss << " Size :" << state_.eraser_size;

        std::string info_str = ss.str();
        screen_buffer_.DrawString(current_x, 0, info_str, cx::Color::Cyan, bg);
        current_x += (int)cx::Util::GetStringWidth(info_str);

        // Time
        std::string time_str = " Time : " + GetTimeString();
        int time_pos = size.cols - (int)cx::Util::GetStringWidth(time_str) - 1;
        if (time_pos > current_x) {
            screen_buffer_.DrawString(time_pos, 0, time_str, fg, bg);
        }
    }

void DrawBottomBar()
    {
        auto size = cx::Screen::GetSize();
        int y = size.rows - 1;
        cx::Color bg(40, 40, 40);
        cx::Color fg = cx::Color::White;

        // 배경 채우기
        for(int x=0; x<size.cols; ++x) screen_buffer_.DrawString(x, y, " ", fg, bg);

        std::stringstream ss;

        // [수정] 컬러 입력 모드 처리
        if ( state_.mode == AppMode::COLOR_INPUT ) {
            // 1. 현재 입력값으로 색상 파싱 시도
            cx::Color preview_color( state_.input_buffer );
            bool is_valid = preview_color.IsValid();

            // 2. '#' 색상 결정 (유효: 흰색, 무효: 빨강)
            cx::Color hash_fg = is_valid ? cx::Color::White : cx::Color::Red;

            int current_x = 1;

            // " Input: "
            screen_buffer_.DrawString(current_x, y, " Input: ", fg, bg);
            current_x += 8;

            // "#" (상태에 따라 색상 변경)
            screen_buffer_.DrawString(current_x, y, "#", hash_fg, bg);
            current_x += 1;

            // 입력된 텍스트
            screen_buffer_.DrawString(current_x, y, state_.input_buffer, cx::Color::Yellow, bg);
            current_x += (int)state_.input_buffer.length();

            // 커서 깜빡임 효과 (_)
            screen_buffer_.DrawString(current_x, y, "_", fg, bg);
            current_x += 2;

            // 3. [복원] 유효한 색상이면 미리보기 블록 출력
            if ( is_valid ) {
                screen_buffer_.DrawString(current_x, y, "[Preview:  ]", fg, bg);
                // "  " 부분의 배경을 해당 색으로 칠함
                screen_buffer_.DrawString(current_x + 9, y, "  ", fg, preview_color);
            }

        } else {
            // 일반 모드 (기존과 동일)
            ss << " " << state_.last_key_msg;
            screen_buffer_.DrawString(1, y, ss.str(), fg, bg);

            // 현재 선택된 색상 표시
            std::string col_blk = "  ";
            // 텍스트 길이 + 약간의 여백 뒤에 색상 박스 출력
            int col_x = 1 + (int)ss.str().length() + 1;
            screen_buffer_.DrawString(col_x, y, col_blk, fg, state_.current_color);
        }

        // Mouse Pos
        std::stringstream pos_ss;
        pos_ss << "Pos(" << mouse_cursor_.x << "," << mouse_cursor_.y << ")";
        std::string pos_str = pos_ss.str();
        int pos_x = size.cols - (int)pos_str.length() - 1;
        screen_buffer_.DrawString(pos_x, y, pos_str, fg, bg);
    }

    void Render()
    {
        auto size = cx::Screen::GetSize();

        // 1. 버퍼 & 캔버스 리사이즈 동기화
        screen_buffer_.Resize(size.cols, size.rows);
        ResizeCanvas(size.cols, size.rows);

        // 2. 버퍼 초기화
        screen_buffer_.Clear(cx::Color::Black);

        // 3. 캔버스 내용 그리기
        for(int y=0; y<(int)canvas_.size() && y < size.rows; ++y) {
            for(int x=0; x<(int)canvas_[0].size() && x < size.cols; ++x) {
                const auto& px = canvas_[y][x];
                if (px.ch != " ") {
                    screen_buffer_.DrawString(x, y, px.ch, px.fg, px.bg);
                }
            }
        }

        // 4. 지우개 오버레이 (Highlight)
        // [Fix 3] is_mouse_down_ 조건 추가: 클릭 중일 때만 박스를 그림
        if ( state_.mode == AppMode::ERASER && is_mouse_down_ ) {
            // UI 영역 보호 (상단/하단 바 제외)
            if ( mouse_cursor_.y > 0 && mouse_cursor_.y < size.rows - 1 ) {
                int h = state_.eraser_size;
                int w = state_.eraser_size * 2;
                int start_y = mouse_cursor_.y - (h / 2);
                int start_x = mouse_cursor_.x - (w / 2);

                // 회색 박스 그리기
                screen_buffer_.DrawBox(start_x, start_y, w, h, cx::Color::Black, cx::Color(128, 128, 128));
            }
        }

        // 5. UI 그리기
        DrawTopBar();
        DrawBottomBar();

        // 6. 최종 출력
        screen_buffer_.Flush();
    }
};

int main()
{
    DrawApp app;
    app.Run();
    return 0;
}