#include "ConsoleX.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cmath>

using namespace std::chrono_literals;

int main()
{
    // 1. 초기 설정
    cx::Device::EnableMouse(false); // 마우스 불필요

    cx::Screen::SetBackColor(cx::Color::Black); // 배경색을 검정으로 설정
    cx::Screen::Clear();                        // 해당 배경색으로 화면 전체 지우기
    std::cout << std::flush;                    // 즉시 반영

    cx::Buffer buffer; // 가상 버퍼 생성

    int x = 2, y = 2;
    int dx = 1, dy = 1;
    long long frame_count = 0;
    bool is_running = true;

    while (is_running)
    {
        // --- [입력 처리] ---
        // 0ms 대기로 입력 확인 (Non-blocking)
        if (auto input = cx::Device::GetInput(1ms); input) {
            auto event = cx::Device::Inspect(input);
            if (event.code == cx::DeviceInputCode::q || event.code == cx::DeviceInputCode::ESC) {
                is_running = false;
            }
        }

        // --- [상태 업데이트] ---
        auto size = cx::Screen::GetSize();

        // 1. 버퍼 리사이즈 & 초기화
        buffer.Resize(size.cols, size.rows);
        buffer.Clear(cx::Color::Black);

        // 2. 배경 패턴 그리기 (플리커링 테스트용)
        // 화면 전체에 점을 찍습니다. 깜빡임이 있다면 이 점들이 사라졌다 나타났다 할 것입니다.
        for (int r = 0; r < size.rows; r += 2) {
            for (int c = 0; c < size.cols; c += 4) {
                buffer.DrawString(c, r, ".", cx::Color(200, 200, 200), cx::Color::Black);
            }
        }

        // 3. 움직이는 박스 좌표 계산 (벽 튕기기)
        x += dx;
        y += dy;

        // 경계 체크
        if (x <= 0 || x >= size.cols - 24) dx = -dx;
        if (y <= 0 || y >= size.rows - 12)  dy = -dy;

        // 4. 박스 그리기 (색상 애니메이션 포함)
        // 매 프레임 색상이 변하는 박스
        uint8_t r = (frame_count * 2) % 255;
        uint8_t g = (frame_count * 3) % 255;
        uint8_t b = (frame_count * 5) % 255;
        cx::Color box_color(r, g, b);

        buffer.DrawBox(x, y, 24, 12, box_color, cx::Color{20,20,20});
        buffer.DrawString(x + 8, y + 5, "NO FLICKER", cx::Color::White, cx::Color::Black);

        // 5. 프레임 카운터 및 안내
        std::string info = " Frame: " + std::to_string(frame_count) + " | Press [Q] to Quit ";
        buffer.DrawString(2, 0, info, cx::Color::Yellow, cx::Color::Blue);

        // --- [렌더링] ---
        // 여기서 변경된 픽셀만 계산되어 터미널로 전송됩니다.
        buffer.Flush();

        frame_count++;

        // 약 60 FPS 조절 (너무 빠르면 눈이 아플 수 있음)
        std::this_thread::sleep_for(16ms);
    }

    cx::Screen::Clear();
    cx::Screen::ResetColor();
    std::cout << "Test Finished." << std::endl;

    return 0;
}