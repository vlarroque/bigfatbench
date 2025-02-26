#ifndef BFB_BENCHMARK_HPP
#define BFB_BENCHMARK_HPP

#include <cstdint>
#include <functional>
#include <string>

namespace bfb
{
    class Benchmark
    {
      public:
        using Task = std::function<void()>;

        Benchmark( std::string name = "Unnamed Benchmark" );
        ~Benchmark() = default;

        std::vector<double> run( const Task & task ) const;
        std::vector<double> run( const Task & init, const Task & task, const Task & end = [] {} ) const;

        Benchmark & iterations( const std::uint32_t iterations );
        Benchmark & warmups( const std::uint32_t warmups );
        Benchmark & timerFunction( const std::function<double( const Task & )> & timerFunction, std::string timerUnit = "" );
        Benchmark & printProgress( const bool printProgress = true );
        Benchmark & printIterationsStats( const bool iterationsStats = true );
        Benchmark & printStats( const bool printStats = true );
        Benchmark & progressBarWidth( const std::uint32_t progressBarWidth );
        Benchmark & name( std::string name );

        static double timer_s( const Task & task );
        static double timer_ms( const Task & task );
        static double timer_us( const Task & task );
        static double timer_ns( const Task & task );
        template<typename TimeUnit>
        static double timer( const Task & task );

      private:
        std::string _name { "Unnamed Benchmark" };

        std::uint32_t _iterations { 10 };
        std::uint32_t _warmups { 0 };
        std::uint32_t _progressBarWidth { 50 };

        std::function<double( const Task & )> _timerFunction { timer_ms };
        std::string                           _timerUnit { "ms" };

        bool _printProgress { true };
        bool _printIterationStats { false };
        bool _printStats { false };

        std::vector<double> runInternal( const Task & task, const Task & init, const Task & end ) const;
        std::vector<double> runInternalWithProgress( const Task & task, const Task & init, const Task & end ) const;

        void printStats( const double totalTime, const std::vector<double> & results ) const;
    };
} // namespace bfb

#include "bfb/benchmark.inl"

#endif // BFB_BENCHMARK_HPP
