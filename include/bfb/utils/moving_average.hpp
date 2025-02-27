#ifndef BFB_UTILS_MOVING_AVERAGE_HPP
#define BFB_UTILS_MOVING_AVERAGE_HPP

#include <cstdint>
#include <queue>

namespace bfb
{
    class MovingAverage
    {
      public:
        MovingAverage( const std::uint32_t windowSize ) : _windowSize( windowSize ) {}
        ~MovingAverage() = default;

        void emplace( const double & value )
        {
            _sum += value;
            _values.emplace( value );

            if ( _values.size() > _windowSize )
            {
                _sum -= _values.front();
                _values.pop();
            }
        }

        double mean() const { return _sum / static_cast<double>( _values.size() ); }

        void reset()
        {
            _sum = 0.0;

            std::queue<double> empty;
            std::swap( _values, empty );
        }

      private:
        std::uint32_t _windowSize;
        double        _sum { 0.0 };

        std::queue<double> _values;
    };
} // namespace bfb

#endif // BFB_UTILS_MOVING_AVERAGE_HPP
