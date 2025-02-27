#include "bfb/benchmark.hpp"
#include "bfb/utils/chrono.hpp"

namespace bfb
{
    inline std::vector<double> Benchmark::run( const Task & task ) const
    {
        return run( [] {}, task );
    }

    inline std::vector<double> Benchmark::run( const Task & init, const Task & task, const Task & end ) const
    {
        if ( _printProgress )
            return runInternalWithProgress( task, init, end );

        return runInternal( task, init, end );
    }

    inline Benchmark & Benchmark::iterations( const std::uint32_t iterations )
    {
        _iterations = iterations;
        return *this;
    }

    inline Benchmark & Benchmark::warmups( const std::uint32_t warmups )
    {
        _warmups = warmups;
        return *this;
    }

    inline Benchmark & Benchmark::timerFunction( const std::function<double( const Task & )> & timerFunction, std::string timerUnit )
    {
        _timerFunction = timerFunction;
        _timerUnit     = std::move( timerUnit );
        return *this;
    }

    inline Benchmark & Benchmark::printProgress( const bool printProgress )
    {
        _printProgress = printProgress;
        return *this;
    }

    inline Benchmark & Benchmark::printIterationsStats( const bool iterationsStats )
    {
        _printIterationStats = iterationsStats;
        return *this;
    }

    inline Benchmark & Benchmark::printStats( const bool printStats )
    {
        _printStats = printStats;
        return *this;
    }

    inline Benchmark & Benchmark::progressBarWidth( const std::uint32_t progressBarWidth )
    {
        _progressBarWidth = progressBarWidth;
        return *this;
    }

    inline Benchmark & Benchmark::name( std::string name )
    {
        _name = std::move( name );
        return *this;
    }

    inline double Benchmark::timer_s( const Task & task ) { return timer<std::ratio<1>>( task ); }

    inline double Benchmark::timer_ms( const Task & task ) { return timer<std::milli>( task ); }

    inline double Benchmark::timer_us( const Task & task ) { return timer<std::micro>( task ); }

    inline double Benchmark::timer_ns( const Task & task ) { return timer<std::nano>( task ); }

    template<typename TimeUnit>
    double Benchmark::timer( const Task & task )
    {
        const SteadyChrono chrono {};
        task();
        return chrono.elapsed<TimeUnit>();
    }
} // namespace bfb