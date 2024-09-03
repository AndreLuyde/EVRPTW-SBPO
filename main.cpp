//============================================================================
// Name        : main.cpp
// Author      : André Luyde
// Version     : 1 - Schneider
// Copyright   : Your copyright notice
// Description : VNS for EVRPTW
//============================================================================

#include "solution/solution.h"

using namespace std;

int compare(const void * a, const void * b);

int main(int argc, char* argv[]) {
	vector<string> arguments(argv + 1, argv + argc);
    cout << fixed << setprecision(2);
    string inputFileName = arguments[0];

    Data data(inputFileName);

    int vns_max = stoi(argv[2]);

    data.orderRequests = data.requests;    
    qsort(&data.orderRequests.front(), data.orderRequests.size(), sizeof(Request), compare);

    clock_t start = clock();

    Solution solution;
    solution.VNS(data, vns_max);
    
    clock_t end = clock();
    double finalTime = ((double) end - start) / ((double) CLOCKS_PER_SEC);

    solution.showSolution();
    cout << finalTime << "(s)" << endl;
	return 0;
}

int compare(const void * a, const void * b) {
    Request *idA = (Request *) a;
    Request *idB = (Request *) b;
    if (idA->twA == idB->twA)
        return 0;
    return (idA->twA < idB->twA) ? -1 : 1;
}
