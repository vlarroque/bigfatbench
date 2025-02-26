#ifndef BFB_BENCHMARK_HPP
#define BFB_BENCHMARK_HPP

#define BFB_NAMESPACE bfb

#include <iomanip>
#include <string>

namespace BFB_NAMESPACE
{
    class Chrono
    {
      public:
        Chrono() : _start( std::chrono::high_resolution_clock::now() ) {};
        ~Chrono() = default;

        void start() { _start = std::chrono::high_resolution_clock::now(); }

        template<typename TimeUnit>
        double elapsed() const
        {
            return std::chrono::duration<double, TimeUnit>( std::chrono::high_resolution_clock::now() - _start ).count();
        }

      private:
        std::chrono::time_point<std::chrono::high_resolution_clock> _start;
    };

    class Benchmark
    {
      public:
        using Task = std::function<void()>;

        static double timer_ms( const Task & task ) { return timerInternal<std::milli>( task ); }
        static double timer_us( const Task & task ) { return timerInternal<std::micro>( task ); }
        static double timer_ns( const Task & task ) { return timerInternal<std::nano>( task ); }

        Benchmark( std::string name = "Unnamed Benchmark" ) : _name( std::move( name ) ) {}
        ~Benchmark() = default;

        std::vector<double> run( const Task & task, const Task & init = []() {}, const Task & end = []() {} )
        {
            if ( _printProgress )
                return runPrintInternal( task, init, end );
            else
                return runInternal( task, init, end );
        }

        Benchmark & iterations( const std::uint32_t iterations )
        {
            _iterations = iterations;
            return *this;
        }

        Benchmark & warmups( const std::uint32_t warmups )
        {
            _warmups = warmups;
            return *this;
        }

        Benchmark & warmupPercentage( const float warmupPercentage )
        {
            _warmupPercentage = warmupPercentage;
            return *this;
        }

        Benchmark & timerFunction( const std::function<double( const Task & )> & timerFunction, std::string timerUnit = "" )
        {
            _timerFunction = timerFunction;
            _timerUnit     = std::move( timerUnit );
            return *this;
        }

        Benchmark & printProgress( const bool printProgress = true )
        {
            _printProgress = printProgress;
            return *this;
        }

        Benchmark & printIterationsStats( const bool iterationsStats = true )
        {
            _printIterationStats = iterationsStats;
            return *this;
        }

        Benchmark & printStats( const bool printStats = true )
        {
            _printStats = printStats;
            return *this;
        }

        Benchmark & name( std::string name )
        {
            _name = std::move( name );
            return *this;
        }

      private:
        std::string _name {};

        std::uint32_t _iterations { 10 };
        std::uint32_t _warmups { 0 };
        float         _warmupPercentage { 0.1f };
        std::uint32_t _progressBarWidth { 50 };

        std::function<double( const Task & )> _timerFunction { timer_ms };
        std::string                           _timerUnit { "ms" };

        bool _printProgress { true };
        bool _printIterationStats { false };
        bool _printStats { false };

        static constexpr const char * BOLD      = "\033[1m";
        static constexpr const char * ITALIC    = "\033[3m";
        static constexpr const char * UNDERLINE = "\033[4m";

        static constexpr const char * RESET_ALL  = "\033[0m";
        static constexpr const char * CLEAR_LINE = "\033[2K";
        static constexpr const char * LINE_UP    = "\033[A";

        static constexpr const char * COL_WHITE        = "\033[38;2;255;255;255m";
        static constexpr const char * COL_CORAL        = "\033[38;2;255;127;80m";
        static constexpr const char * COL_LIGHT_YELLOW = "\033[38;2;255;255;224m";
        static constexpr const char * COL_LIGHT_BLUE   = "\033[38;2;173;216;230m";
        static constexpr const char * COL_LIGHT_CORAL  = "\033[38;2;240;128;128m";

        template<typename Unit>
        static double timerInternal( const Task & task )
        {
            Chrono chrono;
            chrono.start();
            task();
            return chrono.elapsed<Unit>();
        }

        std::vector<double> runPrintInternal( const Task & task, const Task & init, const Task & end ) const
        {
            std::cout << std::setprecision( 3 );

            typedef std::ratio<1> seconds;
            const std::uint32_t   warmups = _warmups > 0 ? _warmups : static_cast<std::uint32_t>( _warmupPercentage * static_cast<float>( _iterations ) );

            std::cout << COL_WHITE << "> " << BOLD << UNDERLINE << _name << RESET_ALL;
            std::cout << COL_WHITE << " (" << warmups << " warmups " << _iterations << " iterations)" << RESET_ALL << std::endl;

            RollingArray<double> itTimes( 10 );

            double totalTime = 0.0;

            Chrono warmupChrono;
            if ( warmups > 0 )
            {
                for ( std::uint32_t i = 0; i < warmups; i++ )
                {
                    Chrono itChrono;
                    printProgress( "Warmups", i, warmups );

                    init();
                    totalTime += _timerFunction( task );
                    end();

                    itTimes.emplace( itChrono.elapsed<seconds>() );
                    printRemainingTime( i, warmups, itTimes.mean() );
                }
                printProgress( "Warmups", warmups, warmups, warmupChrono.elapsed<seconds>() );
            }

            std::cout << std::endl << RESET_ALL;
            itTimes.reset();

            std::vector<double> results {};
            results.resize( _iterations );

            Chrono benchmarkChrono;
            for ( std::uint32_t i = 0; i < _iterations; i++ )
            {
                Chrono itChrono;

                printProgress( "Benchmark", i, _iterations );

                init();
                const double currentTime = _timerFunction( task );
                results[ i ]             = currentTime;
                totalTime += currentTime;
                end();

                itTimes.emplace( itChrono.elapsed<seconds>() );
                printRemainingTime( i, _iterations, itTimes.mean() );
            }
            printProgress( "Benchmark", _iterations, _iterations, benchmarkChrono.elapsed<seconds>() );

            std::cout << std::endl << CLEAR_LINE << LINE_UP << RESET_ALL;

            if ( _printStats )
            {
                const double mean = totalTime / static_cast<double>( warmups + _iterations );

                double variance = 0.;
                double fastest  = std::numeric_limits<double>::max();
                double slowest  = std::numeric_limits<double>::lowest();
                for ( const double time : results )
                {
                    variance += ( time - mean ) * ( time - mean );
                    slowest = std::max( slowest, time );
                    fastest = std::min( fastest, time );
                }
                variance /= ( warmups + _iterations ) - 1;

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

            std::cout << std::endl << CLEAR_LINE << LINE_UP << RESET_ALL << std::endl;

            return results;
        }

        std::vector<double> runInternal( const Task & task, const Task & init, const Task & end ) const
        {
            const std::uint32_t warmups = _warmups > 0 ? _warmups : static_cast<std::uint32_t>( _warmupPercentage * static_cast<float>( _iterations ) );
            if ( warmups > 0 )
            {
                for ( std::uint32_t i = 0; i < warmups; i++ )
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

        void printRemainingTime( std::uint32_t index, std::uint32_t count, const double meanItTime ) const
        {
            std::cout << std::endl << CLEAR_LINE << COL_CORAL << " ~ Estimated remaining time : " << std::setfill( ' ' ) << std::setw( 5 ) << meanItTime * ( count - index + 1 ) << "s";
            if ( _printIterationStats )
                std::cout << " [" << meanItTime << "s/it | " << 1.f / meanItTime << "it/s]";
            std::cout << "\033[A" << std::flush << RESET_ALL;
        }

        // clang-format off
        void printProgress(const std::string &name, std::uint32_t index, std::uint32_t count, const double totalTime = 0.0) const {
            const int32_t width = static_cast<int32_t>(std::to_string(count).size());
            bool done = (index == count);
            std::cout << "\r" << CLEAR_LINE
                      << (done ? ITALIC : "")
                      << name << ": " << std::setfill(' ') << std::setw(width) << index << "/" << count
                      << RESET_ALL << " [";
            done ? setColor(0, 128, 0) : setColorFromProgress(index, count);
            std::cout << getProgressBar(index, count)
                      << RESET_ALL << "] " << (done ? ITALIC : "")
                      << (static_cast<float>(index) / count * 100.f) << " %"
                      << RESET_ALL << (done ? " » Done" : "");
            if (done) std::cout << " (" << totalTime << "s).";
            std::cout << std::flush;
        }
        // clang-format on

        static void setColor( const std::uint32_t r, const std::uint32_t g, const std::uint32_t b ) { std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m"; }

        static void setColorFromProgress( const std::uint32_t index, const std::uint32_t count )
        {
            const float progress = static_cast<float>( index ) / static_cast<float>( count );
            setColor( 255u, static_cast<std::uint32_t>( std::round( progress * 255.f ) ), 0u );
        }

        std::string getProgressBar( const std::uint32_t index, const std::uint32_t total ) const
        {
            std::string progressBar;
            progressBar.reserve( _progressBarWidth );

            for ( std::uint32_t i = 0; i < _progressBarWidth; ++i )
            {
                const float progress = static_cast<float>( index ) / static_cast<float>( total );

                if ( i < static_cast<std::uint32_t>( std::ceil( progress * static_cast<float>( _progressBarWidth ) ) ) )
                    progressBar += "█";
                else
                    progressBar += " ";
            }

            return progressBar;
        }

        template<typename T>
        class RollingArray
        {
          public:
            RollingArray( const std::size_t size ) : data( size ) {}
            ~RollingArray() = default;

            void emplace( const T value )
            {
                currentSize          = std::min( currentSize + 1, data.size() );
                data[ currentIndex ] = value;
                currentIndex         = ( currentIndex + 1 ) % data.size();
            }

            void reset()
            {
                currentSize  = 0;
                currentIndex = 0;
            }

            T mean() const
            {
                T total = 0;
                for ( std::size_t i = 0; i < currentSize; i++ )
                    total += data[ i ];
                return total / static_cast<T>( currentSize );
            }

          private:
            std::size_t    currentIndex { 0 };
            std::size_t    currentSize { 0 };
            std::vector<T> data;
        };
    };
} // namespace BFB_NAMESPACE

#endif // BFB_BENCHMARK_HPP
