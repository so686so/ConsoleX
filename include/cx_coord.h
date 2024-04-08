#ifndef _CONSOLE_X_COORD_H_
#define _CONSOLE_X_COORD_H_

// STL::C++
#include <iostream> // for 'std::cout <<' overloading

namespace cx // ConsoleX main namespace
{
    // For function declaration : GetCoord(), SetCoord()
    class Coord;

    /**
     * @brief
     *    Get the current absolute coordinates of the console
     *
     * @return
     *    if get sucess, return 'cx::Coord' instance.
     *    coordination return value start ( 1, 1 ) at left-top.
     *    if get failed, return 'cx::ErrorCoord' instance.
     */
    cx::Coord GetCoord( void ) noexcept;

    /**
     * @brief
     *    Set console coord to absolute coordination
     *
     * @param coord
     *    Based on absolute coordinates in the console window.
     *    Based on the left-top point (1, 1).
     *    If the coordinate value exceeds the console size, the value is set at the edge.
     */
    bool SetCoord( const cx::Coord& coord ) noexcept;

    /**
     * @brief
     *    ConsoleX coordination class
     *
     * @note
     *    All console coordinate calculations inside ConsoleX
     *    must use the corresponding class.
     */
    class Coord {
    public:
        // Constructors
        Coord( void ) = delete; // coord must have coordination pos
        Coord( const int x, const int y );
        Coord( const Coord& coord );

        // Destructor
        ~Coord() = default;

        /**
         * @brief
         *    Get std::string about current cx::Coord coordiation pos
         *
         * @return
         *   If we successfully set cx::Coord,
         *   return example, like "( 1, 2 )"
         */
        std::string str( void ) const noexcept;

        // check current cx::Coord instance is valid
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
     * Error coordinate returned when an error
     * occurs during coordinate calculation
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