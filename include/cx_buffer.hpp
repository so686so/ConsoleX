#pragma once

#include "cx_color.hpp"
#include <vector>
#include <string>

namespace cx {

    // 화면의 한 칸을 나타내는 구조체
    struct Cell {
        std::string ch = " ";       // 출력할 문자 (UTF-8)
        Color fg = Color::White;    // 글자색
        Color bg = Color::Black;    // 배경색
        bool is_wide_trail = false; // 2칸짜리 문자의 뒷부분인지 여부

        // 변경 감지용 비교 연산자
        bool operator!=(const Cell& other) const {
            return ch != other.ch || fg != other.fg || bg != other.bg;
        }
    };

    // 더블 버퍼링 렌더러 클래스
    class Buffer {
    public:
        Buffer() = default;
        ~Buffer() = default;

        // 버퍼 크기 조절 (리사이즈 시 호출)
        void Resize(int w, int h);

        // Back Buffer 초기화 (매 프레임 시작 시 호출)
        void Clear(const Color& bg_color = Color::Black);

        // 문자열 그리기 (좌표 x, y)
        void DrawString(int x, int y, const std::string& text, const Color& fg, const Color& bg);

        // 박스 그리기 (UI 테두리용)
        void DrawBox(int x, int y, int w, int h, const Color& fg, const Color& bg, bool red_border = false);

        // [핵심] 변경된 부분만 터미널로 출력 (Render)
        void Flush();

    private:
        int width_ = 0;
        int height_ = 0;

        // Front: 현재 화면 상태, Back: 다음 프레임 상태
        std::vector<std::vector<Cell>> front_buffer_;
        std::vector<std::vector<Cell>> back_buffer_;

        // 내부 헬퍼: 버퍼 초기화
        void ClearImpl(std::vector<std::vector<Cell>>& buf, const Color& bg);
    };

} // namespace cx