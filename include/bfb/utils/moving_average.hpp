#ifndef BFB_UTILS_MOVING_AVERAGE_HPP
#define BFB_UTILS_MOVING_AVERAGE_HPP

#include <cstdint>

namespace bfb
{
    class MovingAverage
    {
      public:
        MovingAverage( const std::uint32_t windowSize ) : _windowSize( windowSize ), _sum( 0 ), _array( new double[ _windowSize ]() ) {}
        ~MovingAverage() { delete[] _array; }

        void emplace( const double & value )
        {
            _sum += value;
            _sum -= _array[ _currentIndex ];

            _array[ _currentIndex ] = value;
            _currentIndex           = ( _currentIndex + 1 ) % _windowSize;

            if (_currentSize < _windowSize)
                _currentSize++;
        }

        double mean() const { return _sum / static_cast<double>( _currentSize ); }

        void reset()
        {
            _sum          = 0;
            _currentIndex = 0;
            _currentSize  = 0;
        }

      private:
        std::uint32_t _windowSize;
        double        _sum;
        double *      _array;

        std::uint32_t _currentIndex { 0 };
        std::uint32_t _currentSize { 0 };
    };
} // namespace bfb

#endif // BFB_UTILS_MOVING_AVERAGE_HPP
