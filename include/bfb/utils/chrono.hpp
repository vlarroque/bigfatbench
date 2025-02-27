#ifndef BFB_UTILS_CHRONO_HPP
#define BFB_UTILS_CHRONO_HPP

#include <chrono>

namespace bfb
{
    template<typename ClockType>
    class Chrono
    {
      public:
        Chrono() : _start( ClockType::now() ) {}
        ~Chrono() = default;

        void start() { _start = ClockType::now(); }

        template<typename TimeUnit>
        double elapsed() const
        {
            return std::chrono::duration<double, TimeUnit>( ClockType::now() - _start ).count();
        }

        double elapsed_ms() const { return elapsed<std::milli>(); }
        double elapsed_us() const { return elapsed<std::micro>(); }
        double elapsed_ns() const { return elapsed<std::nano>(); }
        double elapsed() const { return elapsed<std::ratio<1>>(); }

      private:
        std::chrono::time_point<ClockType> _start;
    };

    typedef Chrono<std::chrono::steady_clock> SteadyChrono;
    typedef Chrono<std::chrono::system_clock> SystemChrono;
} // namespace bfb

#endif // BFB_UTILS_CHRONO_HPP
