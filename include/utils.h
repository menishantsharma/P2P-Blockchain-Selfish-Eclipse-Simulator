#ifndef UTILS_H
#define UTILS_H

#include <vector>

using namespace std;

namespace utils {
    int generateRandomNumber(int x, int y);
    bool isConnected(const vector<vector<int>>&graph);
    double get_exponential_random_number(double mean);
    int getNewNTxnId();
    int getNewBlockId();
    vector<int> generateBinaryVector(int size, double percentage);
    vector<vector<double>> generateRandomMatrix(int num_nodes, double min_val = 0.1, double max_val = 5.0);
    void printProgressBar(int progress, int total, int barWidth);
}

#endif