#ifndef _CONSOLE_X_COORD_H_
#define _CONSOLE_X_COORD_H_

// STL::C++
#include <iostream> // for 'std::cout <<' overloading

namespace cx // ConsoleX main namespace
{
    // GetCoord(), SetCoord() 함수 선언을 위한 클래스 전방 선언
    class Coord;

    /**
     * @brief
     *    콘솔의 현재 커서 위치 절대 좌표를 구하는 함수.
     *
     * @return
     *    성공 시, `cx::Coord` 객체로 절대 좌표값을 반환.
     *    좌표값은 ( x, y ) 형태로, 좌상단을 기준으로 ( 1, 1 ) 부터 시작.
     *    실패 시, `cx::ErrorCoord` 객체 반환
     */
    cx::Coord GetCoord( void ) noexcept;

    /**
     * @brief
     *    콘솔의 현재 커서 위치를 주어진 절대좌표 값으로 이동.
     *
     * @param coord
     *    이동 좌표값은 절대좌표이며, 좌상단을 기준으로 ( 1, 1 ) 부터 시작.
     *    좌표갑은 ( x, y ) 형태로, x는 좌우값, y 는 상하값.
     *    만약 좌표값이 콘솔의 크기를 초과할 때는, 최대 콘솔의 모서리까지 이동.
     */
    bool SetCoord( const cx::Coord& coord ) noexcept;

    /**
     * @brief
     *    ConsoleX 좌표 클래스
     *
     * @note
     *    ConsoleX 라이브러리 내 모든 좌표 계산은
     *    반드시 해당 클래스를 사용하여 진행됨.
     */
    class Coord {
    public:
        // 기본 생성자 삭제 - cx::Coord 객체는 반드시 유효한 실제 좌표를 가지고 있어야 함
        Coord( void ) = delete;

        // 일반적인 생성 방법
        Coord( const int x, const int y );

        // 기존 객체 복사 생성자
        Coord( const Coord& coord );

        // 소멸자: default
        // 상속을 위한 virtual 속성
        virtual ~Coord() = default;

        /**
         * @brief
         *    해당 cx::Coord 의 좌표값을 std::string으로 반환하는 함수
         *
         * @return
         *   만약 정상적으로 할당된 cx::Coord 객체라면
         *   "( 1, 2 )" 같은 형식으로 문자열 반환
         */
        std::string str( void ) const noexcept;

        /**
         * @brief
         *   현재 cx::Coord 객체가 유효한 객체인지 판단하는 연산자
         *
         * @example
         *    if( coord ) { ...; } 식으로 사용하면 됨
         */
        operator bool() const { return this->v; }

        // compare operator
        bool operator==( const cx::Coord& other ) const;
        bool operator!=( const cx::Coord& other ) const;

        // calculate operator
        cx::Coord operator+( const cx::Coord& other ) const;
        cx::Coord operator-( const cx::Coord& other ) const;
        cx::Coord operator*( const int scalar ) const;
        cx::Coord operator/( const int scalar ) const;

        // std::cout << overloading
        friend std::ostream& operator<< ( std::ostream& os, const cx::Coord& coord ) {
            cx::SetCoord( coord );
            return os;
        }

        // copy assignment
        cx::Coord& operator=( const cx::Coord& other );

    protected:
        // constructor for cx::ErrorCoord
        explicit Coord( const bool valid );

    public:
        int x = 0; // xpos(row) starting from the top-left
        int y = 0; // ypos(col) starting from the top-left

    private:
        // flag that target Coord is valid
        const bool v;
    };

    /**
     * 좌표 연산 과정에서 오류 발생 시
     * 반환되는 오류 좌표 객체
     */
    class ErrorCoord final : public Coord {
    public:
        ErrorCoord( void ) : Coord( false )
        {}

    public:
        // operators are deleted
        bool operator==( const cx::ErrorCoord& other ) const = delete;
        bool operator!=( const cx::ErrorCoord& other ) const = delete;

        // calculate are deleted
        cx::ErrorCoord operator+( const cx::ErrorCoord& other ) const = delete;
        cx::ErrorCoord operator-( const cx::ErrorCoord& other ) const = delete;
        cx::ErrorCoord operator*( const int scalar ) const = delete;
        cx::ErrorCoord operator/( const int scalar ) const = delete;

        // copy assignment is deleted
        cx::ErrorCoord& operator=( const cx::ErrorCoord& other ) = delete;
    };

} // nsp::cx

#endif