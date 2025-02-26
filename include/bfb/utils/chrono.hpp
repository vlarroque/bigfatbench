#ifndef BFB_UTILS_CHRONO_HPP
#define BFB_UTILS_CHRONO_HPP

#include <chrono>

namespace bfb
{
    class Chrono
    {
      public:
        Chrono() : _start( std::chrono::high_resolution_clock::now() ) {}
        ~Chrono() = default;

        void start() { _start = std::chrono::high_resolution_clock::now(); }

        template<typename TimeUnit>
        double elapsed() const
        {
            return std::chrono::duration<double, TimeUnit>( std::chrono::high_resolution_clock::now() - _start ).count();
        }

        double elapsed_ms() const { return elapsed<std::milli>(); }
        double elapsed_us() const { return elapsed<std::micro>(); }
        double elapsed_ns() const { return elapsed<std::nano>(); }
        double elapsed() const { return elapsed<std::ratio<1>>(); }

      private:
        std::chrono::time_point<std::chrono::high_resolution_clock> _start;
    };
} // namespace BFB_NAMESPACE

#endif // BFB_UTILS_CHRONO_HPP
