#ifndef BFB_BENCHMARK_HPP
#define BFB_BENCHMARK_HPP

#define BFB_NAMESPACE bfb

#include <iomanip>
#include <string>

namespace std
{
    typedef std::ratio<1> seconds;
}

namespace BFB_NAMESPACE
{
    class Chrono
    {
      public:
        Chrono()  = default;
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

        static double timer_ms( const Task & task ) { return _timerInternal<std::milli>( task ); }
        static double timer_us( const Task & task ) { return _timerInternal<std::micro>( task ); }
        static double timer_ns( const Task & task ) { return _timerInternal<std::nano>( task ); }

        Benchmark( std::string name = "Unnamed Benchmark" ) : _name( std::move( name ) ) {}
        ~Benchmark() = default;

        void run( const Task & task, const Task & init = []() {}, const Task & end = []() {} )
        {
            if ( _printProgress )
                _runInternalPrint( task, init, end );
            else
                _runInternal( task, init, end );
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

        Benchmark & printProgress( const bool printProgress = true, const bool iterationsStats = false )
        {
            _printProgress       = printProgress;
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

        template<typename Unit>
        static double _timerInternal( const Task & task )
        {
            Chrono chrono;
            chrono.start();
            task();
            return chrono.elapsed<Unit>();
        }

        // clang-format off
        void printProgress(const std::string &name, std::uint32_t index, std::uint32_t count, const double totalTime = 0.0) const {
            const auto width = std::to_string(count).size();
            bool done = (index == count);
            std::cout << "\r" << _CLEAR_LINE
                      << (done ? _ITALIC : "")
                      << name << ": " << std::setfill(' ') << std::setw(width) << index << "/" << count
                      << _RESET << " [";
            done ? _setColor(0, 128, 0) : _setColorFromProgress(index, count);
            std::cout << _getProgressBar(index, count)
                      << _RESET << "] " << (done ? _ITALIC : "")
                      << std::setprecision(3) << (static_cast<float>(index) / count * 100.f) << " %"
                      << _RESET << (done ? " » Done" : "");
            if (done) std::cout << " (" << totalTime << "s).";
            std::cout << std::flush;
        }


        std::vector<double> _runInternalPrint( const Task & task, const Task & init, const Task & end ) const
        {
            const std::uint32_t warmups = _warmups > 0 ? _warmups : static_cast<std::uint32_t>( _warmupPercentage * static_cast<float>( _iterations ) );

            _setColor( 255, 255, 255 );
            std::cout << "> " << _BOLD << _UNDERLINE << _name << _RESET;
            _setColor( 255, 255, 255 );
            std::cout << " (" << warmups << " warmups " << _iterations << " iterations)" << std::endl;
            _resetColor();

            RollingArray<double> itTimes( warmups + _iterations );

            if ( warmups > 0 )
            {
                Chrono warmupChrono;
                warmupChrono.start();
                for ( std::uint32_t i = 0; i < warmups; i++ )
                {
                    Chrono chrono;
                    chrono.start();

                    printProgress( "Warmups", i, warmups );

                    init();
                    task();
                    end();

                    itTimes.emplace(chrono.elapsed<std::seconds>());
                    std::cout << std::endl << " ~ Estimated remaining time : " << itTimes.mean() * (warmups - i + 1) << "s\033[A" << std::flush;
                }
                printProgress( "Warmups", warmups, warmups, warmupChrono.elapsed<std::seconds>() );
            }
            std::cout << std::endl;
            itTimes.reset();

            std::vector<double> results {};
            results.resize( _iterations );

            Chrono benchmarkChrono;
            benchmarkChrono.start();
            for ( std::uint32_t i = 0; i < _iterations; i++ )
            {
                Chrono chrono;
                chrono.start();

                printProgress( "Benchmark", i, _iterations );

                init();
                const double currentTime = _timerFunction( task );
                results[ i ]             = currentTime;
                end();

                itTimes.emplace(chrono.elapsed<std::seconds>());
                    std::cout << std::endl << " ~ Estimated remaining time : " << itTimes.mean() * (_iterations - i + 1) << "s" <<_LINE_UP << std::flush;

            }
            printProgress( "Benchmark", _iterations, _iterations, benchmarkChrono.elapsed<std::seconds>() );

            std::cout << std::endl << _CLEAR_LINE << _LINE_UP ;

            return results;
        }

        std::vector<double> _runInternal( const Task & task, const Task & init, const Task & end ) const
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

        static constexpr const char * _BOLD       = "\033[1m";
        static constexpr const char * _ITALIC     = "\033[3m";
        static constexpr const char * _UNDERLINE  = "\033[4m";
        static constexpr const char * _RESET      = "\033[0m";
        static constexpr const char * _CLEAR_LINE = "\033[2K";
        static constexpr const char * _LINE_UP    = "\033[A";

        static void _setColor( const std::uint32_t r, const std::uint32_t g, const std::uint32_t b ) { std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m"; }

        static void _setColorFromProgress( const std::uint32_t index, const std::uint32_t count )
        {
            const float progress = static_cast<float>( index ) / static_cast<float>( count );
            _setColor( 255u, static_cast<std::uint32_t>( std::round( progress * 255.f ) ), 0u );
        }

        static void _resetColor() { std::cout << "\033[0m"; }

        std::string _getProgressBar( const std::uint32_t index, const std::uint32_t total ) const
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
            RollingArray( std::size_t size ) : data( size ) {}
            ~RollingArray() = default;

            void emplace(const T value)
            {
                currentSize = std::min(currentSize + 1, data.size());
                data[currentIndex] = value;
                currentIndex = (currentIndex + 1) % data.size();
            }

            void reset()
            {
                currentSize = 0;
                currentIndex = 0;
            }

            T mean() const
            {
                T total = 0;
                for (std::size_t i = 0; i < currentSize; i++)
                    total += data[i];
                return total / static_cast<T>(currentSize);
            }

        private:
            std::size_t currentIndex { 0 };
            std::size_t currentSize { 0 };
            std::vector<T> data;
        };
    };
} // namespace BFB_NAMESPACE

#endif // BFB_BENCHMARK_HPP
