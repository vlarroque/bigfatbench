#include "bfb/benchmark.hpp"

#include "bfb/utils/moving_average.hpp"
#include "bfb/utils/print_helpers.hpp"

namespace bfb
{
    Benchmark::Benchmark( std::string name ) : _name( std::move( name ) ) {}

    std::vector<double> Benchmark::runInternal( const Task & task, const Task & init, const Task & end ) const
    {
        if ( _warmups > 0 )
        {
            for ( std::uint32_t i = 0; i < _warmups; i++ )
            {
                init();
                task();
                end();
            }
        }

        std::vector<double> results {};
        results.resize( _iterations );
        for ( std::uint32_t i = 0; i < _iterations; i++ )
        {
            init();
            const double currentTime = _timerFunction( task );
            results[ i ]             = currentTime;
            end();
        }

        return results;
    }

    std::vector<double> Benchmark::runInternalWithProgress( const Task & task, const Task & init, const Task & end ) const
    {
        std::cout << std::setprecision( 3 );

        typedef std::ratio<1> seconds;

        std::cout << COL_WHITE << "> " << BOLD << UNDERLINE << _name << RESET_ALL;
        std::cout << COL_WHITE << " (" << _warmups << " warmups " << _iterations << " iterations)" << RESET_ALL << std::endl;

        MovingAverage itTimes( 10 );

        double totalTime = 0.0;

        if ( _warmups > 0 )
        {
            SteadyChrono warmupChrono;
            for ( std::uint32_t i = 0; i < _warmups; i++ )
            {
                SteadyChrono itChrono;
                printProgressBar( "Warmups", i, _warmups, _progressBarWidth );

                init();
                totalTime += _timerFunction( task );
                end();

                itTimes.emplace( itChrono.elapsed<seconds>() );
                printRemainingTime( i, _warmups, itTimes.mean(), _printIterationStats );
            }
            printProgressBar( "Warmups", _warmups, _warmups, _progressBarWidth, warmupChrono.elapsed<seconds>() );
        }
        std::cout << std::endl << RESET_ALL;

        itTimes = MovingAverage( 10 );

        std::vector<double> results {};
        results.resize( _iterations );

        SteadyChrono benchmarkChrono;
        for ( std::uint32_t i = 0; i < _iterations; i++ )
        {
            SteadyChrono itChrono;

            printProgressBar( "Benchmark", i, _iterations, _progressBarWidth );

            init();
            const double currentTime = _timerFunction( task );
            results[ i ]             = currentTime;
            totalTime += currentTime;
            end();

            itTimes.emplace( itChrono.elapsed<seconds>() );
            printRemainingTime( i, _iterations, itTimes.mean(), _printIterationStats );
        }
        printProgressBar( "Benchmark", _iterations, _iterations, _progressBarWidth, benchmarkChrono.elapsed<seconds>() );

        if ( _printStats )
        {
            std::cout << std::endl << CLEAR_LINE << LINE_UP << RESET_ALL;
            printStats( totalTime, results );
        }

        std::cout << std::endl << CLEAR_LINE << LINE_UP << RESET_ALL << std::endl;

        return results;
    }

    void Benchmark::printStats( const double totalTime, const std::vector<double> & results ) const
    {
        const double mean = totalTime / static_cast<double>( _warmups + _iterations );

        double variance = 0.;
        double fastest  = std::numeric_limits<double>::max();
        double slowest  = std::numeric_limits<double>::lowest();
        for ( const double time : results )
        {
            variance += ( time - mean ) * ( time - mean );
            slowest = std::max( slowest, time );
            fastest = std::min( fastest, time );
        }
        variance /= static_cast<double>( _warmups + _iterations - 1 );

        const double standardDeviation = std::sqrt( variance );

        const std::size_t totalTimeWidth = std::to_string( static_cast<std::size_t>( totalTime ) ).size();

        std::cout << std::endl << std::endl;
        std::cout << std::setprecision( static_cast<std::int32_t>( totalTimeWidth ) + 2 );
        std::cout << COL_WHITE << UNDERLINE << "Execution time" << RESET_ALL;
        std::cout << "\n  Total time          " << totalTime << " " << _timerUnit;
        std::cout << std::setprecision( 3 );
        std::cout << COL_LIGHT_YELLOW << "\n  Mean time           " << mean << " " << _timerUnit;
        std::cout << COL_LIGHT_BLUE << "\n  Slowest             " << slowest << " " << _timerUnit;
        std::cout << " (" << slowest - mean << " " << _timerUnit << " (" << ( slowest - mean ) * 100. / mean << " %))";
        std::cout << COL_LIGHT_CORAL << "\n  Fastest             " << fastest << " " << _timerUnit;
        std::cout << " (" << fastest - mean << " " << _timerUnit << " (" << ( fastest - mean ) * 100. / mean << " %))";
        std::cout << std::endl << COL_WHITE << UNDERLINE << "Stats" << RESET_ALL;
        std::cout << "\n  Variance            " << variance;
        std::cout << "\n  Standard deviation  " << standardDeviation;
    }
} // namespace bfb
