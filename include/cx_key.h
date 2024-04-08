#ifndef _DIRECT_KEY_H_
#define _DIRECT_KEY_H_

/**
 * Linux Direct Keystroke functions
 */

// STL :: C
#include <termios.h> // struct termios
#include <unistd.h>  //
#include <stdio.h>   //
// STL :: CPP
#include <memory>    // std::shared_ptr
#include <mutex>     // mutex, unique_lock
#include <iostream>

namespace cx // ConsoleX main namespace
{
    // Enum for key identifier
    constexpr int ARROW_ADD_VALUE = 128;
    constexpr int FUNCIONAL_VALUE = 256;

    enum KeyBoard
    {
        // Alphabet : Only use uppercase
        A = 65, B = 66, C = 67, D = 68, E = 69, F = 70, G = 71, H = 72, I = 73,
        J = 74, K = 75, L = 76, M = 77, N = 78, O = 79, P = 80, Q = 81, R = 82,
        S = 83, T = 84, U = 85, V = 86, W = 87, X = 88, Y = 89, Z = 90,

        // Numbers
        NUM_0 = 48, NUM_1 = 49, NUM_2 = 50, NUM_3 = 51, NUM_4 = 52,
        NUM_5 = 53, NUM_6 = 54, NUM_7 = 55, NUM_8 = 56, NUM_9 = 57,

        // Special keys
        TAB = 9, ENTER = 10, ESC = 27, SPACE = 32, BACKSPACE = 127,

        // Funcional keys
        F1  = 11 + FUNCIONAL_VALUE,
        F2  = 12 + FUNCIONAL_VALUE,
        F3  = 13 + FUNCIONAL_VALUE,
        F4  = 14 + FUNCIONAL_VALUE,
        F5  = 15 + FUNCIONAL_VALUE,
        F6  = 16 + FUNCIONAL_VALUE,
        F7  = 17 + FUNCIONAL_VALUE,
        F8  = 18 + FUNCIONAL_VALUE,
        F9  = 19 + FUNCIONAL_VALUE,
        F10 = 20 + FUNCIONAL_VALUE,
        F11 = 21 + FUNCIONAL_VALUE,
        F12 = 22 + FUNCIONAL_VALUE,

        // Arrows
        ARROW_UP    = 65 + ARROW_ADD_VALUE,
        ARROW_DOWN  = 66 + ARROW_ADD_VALUE,
        ARROW_RIGHT = 67 + ARROW_ADD_VALUE,
        ARROW_LEFT  = 68 + ARROW_ADD_VALUE,

        // Default Flags
        NONE_INPUT       = -2,
        ALREADY_OCCUPIED = -3,
        FORCE_INTERRUPT  = 11,
    };

    // For check & capitalize
    constexpr int CAPITALIZE_OFFSET = 32;
    enum LowerKeyBoard
    {
        a = A + CAPITALIZE_OFFSET,
        z = Z + CAPITALIZE_OFFSET
    };

    // For convinient timeout set
    enum KeyDelay
    {
        FPS_01 = 1000 / 01,
        FPS_10 = 1000 / 10,
        FPS_15 = 1000 / 15,
        FPS_20 = 1000 / 20,
        FPS_25 = 1000 / 25,
        FPS_30 = 1000 / 30,
        FPS_60 = 1000 / 60,
    };

    // Typename contraction
    using Termios = struct termios;
    using Timeval = struct timeval;

    class Key
    {
    // Main key stroke read functions ----------------------------------------------------------------------------------------
    public:
        /**
         * @brief  Reading key values synchronously.
         * @note   This function waits until you stroke a key
         * @return Pressed key value converted to KeyBoard enum value
         */
        static KeyBoard GetKey( void ) noexcept;

        /**
         * @brief  Reading key values synchronously until timeout (millisec)
         * @note   This function waits for a timeout or until you stroke a key
         * @param  timeout_milsec If value >= 0, a one-time timeout waits for the value in millisec when the function operates.
         * @return Pressed key value converted to KeyBoard enum value, or 'KeyBoard::NONE_INPUT' if not pressed until timeout.
         *         If another keystroke waiting function is already in progress, it returns 'KeyBoard::ALREADY_OCCUPIED'.
         */
        static KeyBoard GetKeyTimeout( const int timeout_milsec = -1 ) noexcept;

    // Set flag or config functions ------------------------------------------------------------------------------------------
    public:
        /**
         * @brief  Function to set timeout milliseconds to wait for key input synchronously
         * @param  timeout_milsec If value >= 0, set the value as the default synchronous key input waiting timeout value.
         * @return True if the change was applied, false if the change failed cuz other code was already waiting for key input.
         */
        static bool SetReadKeyAwaitTimeout( const int timeout_milsec ) noexcept;

    // Utilities -------------------------------------------------------------------------------------------------------------
    public:
        /**
         * @brief  A function that attempts to temporarily disable 'immediate key input' and 'cursor hiding'
         * @return true if the attempt was successful, false if the attempt failed because key input was waiting elsewhere
         */
        static bool TryPause( void ) noexcept;

        /**
         * @brief A function that temporarily force disable 'immediate key input' and 'cursor hiding'
         * @note  If you are waiting for a key input somewhere else, wait until the task is completed and then force a pause.
         */
        static void ForcePause( void ) noexcept;

        /**
         * @brief Resume 'immediate key input' and 'cursor hiding' functions stopped with TryPause() or ForcePause()
         */
        static void Resume( void ) noexcept;

        static int KeyNumToInt( const KeyBoard key ) noexcept
        {
            return ( key >= NUM_0 && key <= NUM_9 ) ? (int)( key - 48 ) : -1;
        }

        static void ForceStopGetKey( void )
        {
            GetKeyPtr()->await_force_stop_flg = true;
        }

    // Others ----------------------------------------------------------------------------------------------------------------
    public:
        /**
         * @brief public destructor for RAII of shared_ptr
         */
        ~Key() { Key::Deinit(); }

    // Private methodes ------------------------------------------------------------------------------------------------------
    private:
        /**
         * @brief private constructor for RAII and singleton
         */
        explicit Key()
            : is_key_stroke_direct( false )
            , await_force_stop_flg( false )
            , await_timeout_milsec( KeyDelay::FPS_30 )
        { Key::Init(); }

        /**
         * @brief Key class initialize
         */
        void Init( void ) noexcept;

        /**
         * @brief Key class deinitialize
         */
        void Deinit( void ) noexcept;

        /**
         * @brief  Get singleton Key shared_ptr
         * @return Singleton pattern DirectKey::Key pointer
         */
        static std::shared_ptr<cx::Key> GetKeyPtr( void ) noexcept;

        /**
         * @brief  Function to change the key input time to when the key is pressed
         * @return true if the attempt was successful, false if the attempt failed because key input was waiting elsewhere
         */
        bool SetKeyStrokeDirect( void ) noexcept;

        /**
         * @brief  Function to change the key input time to when 'Enter' key pressed
         * @return true if the attempt was successful, false if the attempt failed because key input was waiting elsewhere
         */
        bool SetKeyStrokeWhenPressEnter( void ) noexcept;

        /**
         * @brief Return current key stroke type is DirectStroke
         */
        bool IsKeyStrokeDirect( void ) noexcept
        { return this->is_key_stroke_direct; }

    // Private members -------------------------------------------------------------------------------------------------------
    private:
        Termios save_attr;
        bool    is_key_stroke_direct;
        bool    await_force_stop_flg;
        int     await_timeout_milsec;
        mutable std::mutex mtx;
    }; // cls: Key

} // nsp::cx
#endif