#include <random>
#include <chrono>


namespace GRIT {

std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());  

}
