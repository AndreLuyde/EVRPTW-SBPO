#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <climits>
#include <vector>
#include <cstring>
#include <string>
#include <set>
#include <map>
#include <cctype>
#include <iomanip>
#include <algorithm>
#include <list>
#include <stack>
#include <queue>
#include <numeric>
#include <sstream>
#include <fstream>
#include <random>
#include <exception>
#include <stdexcept>
#include <omp.h>
#include <chrono>
#include <random>
#include <thread>
#include <unordered_set>
#include <unordered_map>

using namespace std;

#define INF 987654321
#define EPS 1.0e-5 
#define PI 3.14159265358979323846

static default_random_engine generator((int) chrono::system_clock::now().time_since_epoch().count());
// static default_random_engine generator(11235813);
const int SHIFT_CHAR = -SCHAR_MIN;



#endif