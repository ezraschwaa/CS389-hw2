#include <iostream>
#include <cmath>
#include <string>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <chrono>
#include <algorithm>

#include "cache.h"
#include "tests.cc"

using namespace std; 
using namespace std::chrono; 

int 
main()
{
auto start = high_resolution_clock::now(); 

auto stop = high_resolution_clock::now(); 


auto duration = duration_cast<microseconds>(stop - start); 

cout << "Time taken by function: "
     << duration.count() << " microseconds" << endl; 
  

}