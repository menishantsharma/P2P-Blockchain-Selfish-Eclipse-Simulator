#include <random>
#include <queue>
#include <vector>
#include <string>
#include <iostream>

#include "util.h"

using namespace std;

namespace utils {
    mt19937& getGenerator() {
        static random_device rd;
        static mt19937 gen(rd());
        return gen;
    }

    int generateRandomNumber(int x, int y) {
        uniform_int_distribution<> dis(x, y);
        return dis(getGenerator());
    }

    double get_exponential_random_number(double mean) {
        exponential_distribution<double> exp_dist(1.0 / mean);
        return exp_dist(getGenerator());
    }

    bool isConnected(const vector<vector<int>>&graph) {
        vector<bool> visited(graph.size(), false);
        queue<int> queue;
        queue.push(0);
        visited[0] = true;

        while(!queue.empty()) {
            int node = queue.front();
            queue.pop();

            for(auto peer: graph[node]) {
                if(!visited[peer]) {
                    visited[peer] = true;
                    queue.push(peer);
                }
            }
        }

        for(bool v: visited) {
            if(!v) return false;
        }

        return true;
    }

    vector<int> generateBinaryVector(int size, double percentage) {
        int numOnes = static_cast<int>(size * (percentage / 100.0));
        vector<int> vec(size, 0);

        for (int i = 0; i < numOnes; i++) {
            vec[i] = 1;
        }

        shuffle(vec.begin(), vec.end(), getGenerator());

        return vec;
    }

    vector<vector<double>> generateRandomMatrix(int num_nodes, double min_val, double max_val) {
        vector<vector<double>> matrix(num_nodes, vector<double>(num_nodes));

        uniform_real_distribution<double> dis(min_val, max_val);

        for (int i = 0; i < num_nodes; i++) {
            for (int j = 0; j < num_nodes; j++) {
                matrix[i][j] = dis(getGenerator());
            }
        }

        return matrix;
    }

    void printProgressBar(int progress, int total, int barWidth = 50) {
        float percentage = (float)progress / total;
        int pos = barWidth * percentage;
    
        cout << "[";
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) cout << "=";  // Filled part
            else if (i == pos) cout << ">"; // Moving cursor
            else cout << " ";  // Empty part
        }
        cout << "] " << int(percentage * 100.0) << "%\r";  // Overwrite the line
        cout.flush();
    }
}
