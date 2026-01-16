
# ConsoleX

**ConsoleX**는 리눅스 터미널 환경에서 고성능 비동기 입력 처리와 모던한 UI 렌더링을 구현하기 위한 경량 C++ 라이브러리입니다.

이 프로젝트에는 라이브러리의 기능을 시연하기 위한 **CLI 기반의 드로잉/메모장 애플리케이션**(DrawApp)이 포함되어 있습니다.

## ✨ Key Features

### 🛠 Core Library (`cx_*`)

* **비동기 입력 처리 (`cx_device`)**: `select()` 기반의 멀티플렉싱을 통해 키보드와 마우스 입력을 넌블로킹(Non-blocking)으로 처리합니다.
* **고급 파싱 지원**: xterm, VT100, Tera Term 등 다양한 터미널의 이스케이프 시퀀스(F1~F12, Backspace 등)를 호환성 있게 처리합니다.
* **RGB 트루컬러 지원 (`cx_color`)**: 24-bit RGB 색상을 지원하며, ANSI 코드로 자동 변환합니다.
* **UTF-8 완벽 지원 (`cx_util`)**: 한글, 한자, 이모지(Emoji) 등의 Double-Width 문자와 결합 문자(ZWJ)의 너비를 정확하게 계산하여 깨짐 없는 UI를 제공합니다.
* **플리커링 방지**: 라인 버퍼링 및 차집합 연산(Differential Update) 알고리즘을 통해 화면 깜빡임 없는 매끄러운 렌더링을 구현했습니다.

### 🎨 Demo Application: Notepad/DrawApp

* **마우스 인터랙션**: 클릭 가능한 UI 메뉴, 드래그 드로잉을 지원합니다.
* **도구 모드**:
* **Brush**: 밀도 조절 및 랜덤 그라데이션 효과.
* **Eraser**: 가변 크기 조절, 실시간 영역 하이라이트 및 트레이스 제거.


* **실시간 색상 입력**: F4 키를 통해 Hex Code(#RRGGBB)를 실시간으로 입력하고 미리볼 수 있습니다.
* **반응형 UI**: 터미널 리사이즈 이벤트에 대응하며, 상단 메뉴와 하단 로그창을 분리하여 정보를 표시합니다.

---

## 📂 Project Structure

```bash
ConsoleX/
├── include/
│   ├── ConsoleX.hpp   # 모든 ConsoleX 헤더 통합본
│   ├── cx_color.hpp   # RGB 색상 데이터 및 ANSI 변환
│   ├── cx_device.hpp  # 터미널 입력 제어 (Mouse/Key Parser)
│   ├── cx_screen.hpp  # 커서 이동, 화면 클리어, 해상도 조회
│   └── cx_util.hpp    # UTF-8 문자열 너비 계산 유틸리티
├── src/
│   ├── cx_color.cpp
│   ├── cx_device.cpp
│   ├── cx_screen.cpp
│   ├── cx_util.cpp
│   └── main.cpp       # DrawApp 애플리케이션 구현부
└── CMakeLists.txt     # 빌드 설정

```

---

## 🚀 Build & Run

### Prerequisites

* **C++17** 호환 컴파일러 (GCC, Clang)
* CMake 3.10 이상
* Linux/macOS 환경 (또는 WSL)

### Build Steps

```bash
# 1. 프로젝트 클론
git clone https://github.com/so686so/ConsoleX.git
cd ConsoleX

# 2. 빌드 디렉토리 생성 및 컴파일
mkdir build && cd build
cmake ..
make

# 3. 실행
./ConsoleX

```

---

## 🎮 User Manual (DrawApp)

프로그램 실행 시 마우스를 활성화해야 하며, 터미널이 **xterm-256color** 모드인지 확인하는 것이 좋습니다.

### ⌨️ Keyboard Controls

| Key | Function | Description |
| --- | --- | --- |
| **F1** | **Brush Mode** | 브러시 모드로 전환합니다. |
| **F2** | **Eraser Mode** | 지우개 모드로 전환합니다. |
| **F3** | **Gradient** | 그라데이션 효과를 ON/OFF 토글합니다. |
| **F4** | **Color Input** | Hex Code 색상 입력 모드로 진입합니다. |
| **Q** | **Quit** | 프로그램을 종료합니다. |
| **+ / 2** | **Density/Size Up** | 브러시 밀도를 높이거나 지우개 크기를 키웁니다. |
| **- / 1** | **Density/Size Down** | 브러시 밀도를 낮추거나 지우개 크기를 줄입니다. |
| **ESC** | **Cancel** | 색상 입력 모드를 취소합니다. |

### 🖱️ Mouse Controls

| Action | Function |
| --- | --- |
| **Left Click (Menu)** | 상단 메뉴바의 텍스트(`[F1]`, `[Q]` 등)를 클릭하여 해당 기능을 실행합니다. |
| **Left Drag (Screen)** | 화면에 그림을 그리거나 지웁니다. |
| **Middle Click** | 화면 전체를 깨끗하게 지웁니다 (Clear). |

---

## 🔧 Technical Details

### 1. Flicker-Free Rendering

기존의 `clear()` 후 전체를 다시 그리는 방식 대신, **Line Buffering** 기법을 사용합니다. 변경이 필요한 라인만 커서를 이동하여 덮어쓰고, 빈 공간은 공백 문자로 패딩(Padding) 처리하여 화면 깜빡임을 원천 차단했습니다.

### 2. Differential Update (Eraser)

지우개 동작 시, 이전 프레임의 영역을 모두 지우고 새 영역을 그리는 대신, **차집합 연산**을 수행합니다.

* `Old - New`: 지워야 할 영역 (검은색)
* `New - Old`: 새로 그려야 할 영역 (회색 하이라이트)
* `Intersection`: 유지 (연산 없음)
이를 통해 I/O 부하를 줄이고 시각적 노이즈를 제거했습니다.

### 3. Terminal Compatibility

* **Tera Term**: Backspace(`^H`, ASCII 8)와 F1~F4(`CSI` vs `SS3`) 키 시퀀스 호환성 코드가 적용되어 있습니다.
* **UTF-8**: `cx::Util`은 바이트 단위가 아닌 유니코드 코드포인트 단위로 문자를 분석하여, 한글이 2칸을 차지함을 정확히 인식합니다.

---

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](https://www.google.com/search?q=LICENSE) file for details.
