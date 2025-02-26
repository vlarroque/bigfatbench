#include <iostream>
#include <random>
#include <thread>

#include <bfb/benchmark.hpp>

int main()
{
    bfb::Benchmark benchmark( "Example Benchmark" );
    benchmark.timerFunction( bfb::Benchmark::timer_ms, "ms" );
    benchmark.warmups( 32 );
    benchmark.iterations( 127 );
    benchmark.printIterationsStats();
    benchmark.printStats();

    float total = 0.0f;
    benchmark.run(
        [ &total ]()
        {
            for ( float i = 0; i < 5000000000; i += ( static_cast<float>( std::rand() ) / static_cast<float>( RAND_MAX ) ) * 1000.0f )
                total += std::sin( i );
        } );

    return EXIT_SUCCESS;
}
