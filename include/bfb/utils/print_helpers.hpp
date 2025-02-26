#ifndef PRINT_HELPERS_HPP
#define PRINT_HELPERS_HPP

#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>

namespace bfb
{
    constexpr const char * BOLD      = "\033[1m";
    constexpr const char * ITALIC    = "\033[3m";
    constexpr const char * UNDERLINE = "\033[4m";

    constexpr const char * RESET_ALL  = "\033[0m";
    constexpr const char * CLEAR_LINE = "\033[2K";
    constexpr const char * LINE_UP    = "\033[A";

    constexpr const char * COL_WHITE        = "\033[38;2;255;255;255m";
    constexpr const char * COL_CORAL        = "\033[38;2;255;127;80m";
    constexpr const char * COL_LIGHT_YELLOW = "\033[38;2;255;255;224m";
    constexpr const char * COL_LIGHT_BLUE   = "\033[38;2;173;216;230m";
    constexpr const char * COL_LIGHT_CORAL  = "\033[38;2;240;128;128m";
    constexpr const char * COL_GREEN        = "\033[38;2;0;128;0m";

    inline void setColorFromProgress( const std::uint32_t index, const std::uint32_t count )
    {
        const float progress = static_cast<float>( index ) / static_cast<float>( count );
        std::cout << "\033[38;2;255;" << static_cast<std::uint32_t>( std::round( progress * 255.f ) ) << ";0m";
    }

    inline void printRemainingTime( std::uint32_t index, std::uint32_t count, const double meanItTime = -1.0 )
    {
        std::cout << std::endl << CLEAR_LINE << COL_CORAL << " ~ Estimated remaining time : " << std::setfill( ' ' ) << std::setw( 5 ) << meanItTime * ( count - index + 1 ) << "s";
        if ( meanItTime > 0.0 )
            std::cout << " [" << meanItTime << "s/it | " << 1.f / meanItTime << "it/s]";
        std::cout << LINE_UP << std::flush << RESET_ALL;
    }

    inline std::string getProgressBar( const std::uint32_t index, const std::uint32_t total, const std::uint32_t progressBarWidth )
    {
        std::string progressBar;
        progressBar.reserve( progressBarWidth );

        for ( std::uint32_t i = 0; i < progressBarWidth; ++i )
        {
            const float         progress      = static_cast<float>( index ) / static_cast<float>( total );
            const std::uint32_t progressIndex = static_cast<std::uint32_t>( std::ceil( progress * progressBarWidth ) );
            if ( i < progressIndex )
                progressBar += "█";
            else
                progressBar += " ";
        }

        return progressBar;
    }

    inline void printProgressBar( const std::string & name, std::uint32_t index, std::uint32_t count, const std::uint32_t progressBarWidth, const double totalTime = 0.0 )
    {
        const int32_t width = static_cast<int32_t>( std::to_string( count ).size() );
        const bool    done  = index == count;
        std::cout << "\r" << CLEAR_LINE << ( done ? ITALIC : "" ) << name << ": " << std::setfill( ' ' ) << std::setw( width ) << index << "/" << count << RESET_ALL << " [";
        if ( done )
            std::cout << COL_GREEN;
        else
            setColorFromProgress( index, count );
        std::cout << getProgressBar( index, count, progressBarWidth ) << RESET_ALL << "] " << ( done ? ITALIC : "" ) << static_cast<float>( index ) / count * 100.f << " %" << RESET_ALL << ( done ? " » Done" : "" );
        if ( done )
            std::cout << " (" << totalTime << "s).";
        std::cout << std::flush;
    }

} // namespace bfb
#endif // PRINT_HELPERS_HPP
