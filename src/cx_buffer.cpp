#include "cx_buffer.hpp"
#include "cx_util.hpp"

#include <iostream>
#include <sstream>

namespace cx {

    void Buffer::Resize(int w, int h) {
        if (width_ == w && height_ == h) return;
        width_ = w;
        height_ = h;

        // Front/Back 버퍼 메모리 할당
        front_buffer_.assign(h, std::vector<Cell>(w));
        back_buffer_.assign(h, std::vector<Cell>(w));

        // 리사이즈 직후에는 화면 전체 갱신을 위해 Front를 초기화
        ClearImpl(front_buffer_, Color::Black);
    }

    void Buffer::Clear(const Color& bg_color) {
        ClearImpl(back_buffer_, bg_color);
    }

    void Buffer::ClearImpl(std::vector<std::vector<Cell>>& buf, const Color& bg) {
        for(auto& row : buf) {
            for(auto& cell : row) {
                cell.ch = " ";
                cell.fg = Color::White;
                cell.bg = bg;
                cell.is_wide_trail = false;
            }
        }
    }

    void Buffer::DrawString(int x, int y, const std::string& text, const Color& fg, const Color& bg) {
        if (y < 0 || y >= height_) return;

        int cursor_x = x;
        size_t i = 0;
        size_t len = text.length();

        while (i < len && cursor_x < width_) {
            // UTF-8 문자 길이 계산
            int char_len = 1;
            unsigned char c = (unsigned char)text[i];
            if      (c < 0x80) char_len = 1;
            else if ((c & 0xE0) == 0xC0) char_len = 2;
            else if ((c & 0xF0) == 0xE0) char_len = 3;
            else if ((c & 0xF8) == 0xF0) char_len = 4;

            std::string ch = text.substr(i, char_len);
            int visual_width = (int)cx::Util::GetStringWidth(ch);

            if (cursor_x >= 0 && cursor_x < width_) {
                auto& cell = back_buffer_[y][cursor_x];
                cell.ch = ch;
                cell.fg = fg;
                cell.bg = bg;
                cell.is_wide_trail = false;

                // 2칸 문자(한글 등) 처리: 뒤쪽 칸은 Trail로 마킹
                if (visual_width == 2 && cursor_x + 1 < width_) {
                    auto& trail = back_buffer_[y][cursor_x + 1];
                    trail.ch = ""; // 렌더링 생략
                    trail.fg = fg;
                    trail.bg = bg;
                    trail.is_wide_trail = true;
                }
            }
            cursor_x += visual_width;
            i += char_len;
        }
    }

    void Buffer::DrawBox(int x, int y, int w, int h, const Color& fg, const Color& bg, bool red_border) {
        Color border_c = red_border ? Color::Red : fg;

        DrawString(x, y, "┏", border_c, bg);
        DrawString(x + w - 1, y, "┓", border_c, bg);
        DrawString(x, y + h - 1, "┗", border_c, bg);
        DrawString(x + w - 1, y + h - 1, "┛", border_c, bg);

        for(int i = x + 1; i < x + w - 1; ++i) {
            DrawString(i, y, "━", border_c, bg);
            DrawString(i, y + h - 1, "━", border_c, bg);
        }
        for(int i = y + 1; i < y + h - 1; ++i) {
            DrawString(x, i, "┃", border_c, bg);
            DrawString(x + w - 1, i, "┃", border_c, bg);
        }
        // 내부 채우기 (배경색 적용을 위해 공백 출력)
        for(int j = y + 1; j < y + h - 1; ++j) {
            for(int i = x + 1; i < x + w - 1; ++i) {
                DrawString(i, j, " ", fg, bg);
            }
        }
    }

    void Buffer::Flush() {
        std::stringstream ss;
        Color last_fg = Color::White;
        Color last_bg = Color::Black;

        int term_cursor_y = -1;
        int term_cursor_x = -1;
        bool color_set = false;

        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                Cell& back = back_buffer_[y][x];
                Cell& front = front_buffer_[y][x];

                // 1. 변경 감지 (Diff)
                if (!(back != front)) continue;

                // 2. Wide char 뒷부분은 스킵 (상태만 동기화)
                if (back.is_wide_trail) {
                    front = back;
                    continue;
                }

                // 3. 커서 이동 최적화
                int target_y = y + 1;
                int target_x = x + 1;

                if (term_cursor_y != target_y || term_cursor_x != target_x) {
                    ss << "\033[" << target_y << ";" << target_x << "H";
                    term_cursor_y = target_y;
                    term_cursor_x = target_x;
                }

                // 4. 색상 변경 최적화
                if (!color_set || back.fg != last_fg) {
                    ss << back.fg.ToAnsiForeground();
                    last_fg = back.fg;
                }
                if (!color_set || back.bg != last_bg) {
                    ss << back.bg.ToAnsiBackground();
                    last_bg = back.bg;
                }
                color_set = true;

                // 5. 출력 및 동기화
                ss << back.ch;
                front = back;

                // 6. 커서 위치 추적 업데이트
                int width_step = (int)Util::GetStringWidth(back.ch);
                term_cursor_x += width_step;
            }
        }

        if (ss.rdbuf()->in_avail() > 0) {
            std::cout << ss.str() << std::flush;
        }
    }

} // namespace cx