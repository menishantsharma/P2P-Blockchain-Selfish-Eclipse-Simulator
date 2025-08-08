#include <vector>
#include <iostream>
#include <fstream>
#include "utils.h"
#include "network.h"

using namespace std;

void Network::create_network() {
    vector<vector<int>> temp(n);
    const int MAX_ATTEMPTS = 100; 

    while(true) {
        temp.clear();
        temp.resize(n);
        bool failed = false;

        for(int i = 0; i < n && !failed; i++) {
            int num_connections = utils::generateRandomNumber(min_degree, max_degree);
            int attempts = 0;

            while (temp[i].size() < static_cast<size_t>(num_connections)) {
                if(attempts++ > MAX_ATTEMPTS) {
                    failed = true;
                    break;
                }
                int j = utils::generateRandomNumber(0, n-1);
                if(i != j && find(temp[i].begin(), temp[i].end(), j) == temp[i].end() && temp[j].size() < static_cast<size_t>(max_degree)) {
                    temp[i].push_back(j);
                    temp[j].push_back(i);
                }
            }
        }

        if(failed) continue;

        if(utils::isConnected(temp)) break;
    }

    graph = temp;
}

void Network::print_network() {
    for(int i = 0; i < n; i++) {
        cout << i << " & ";
        int j = 0;
        for(j = 0; j < graph[i].size()-1; j++) {
            cout << graph[i][j] << ", ";
        }

        cout << graph[i][j] << " \\\\";
        cout << endl;
    }
}

double Network::calculate_latency(int sender, int receiver, double message_size, bool is_sender_slow, bool is_receiver_slow) {
    
    double cij = (is_sender_slow || is_receiver_slow) ? 5.0 : 100.0;
    double dij = utils::get_exponential_random_number(96.0/(cij * 1024));
    return dij + rho[sender][receiver] + message_size / (cij * 128);
}

void Network::visualize_network(string filename, int malicious_nodes) {
    filesystem::create_directories("vis/");

    ofstream file("vis/" + filename + ".dot");
    if(!file) {
        cout << "Error opening file" << endl;
        return;
    }

    file << "graph G {" << endl;
    for(int i = 0; i < n; i++) {
        if(i < malicious_nodes) file << i << " [fillcolor=red, style=filled];" << endl;
        else file << i << " [fillcolor=green, style=filled];" << endl;
        
        bool flag = false;
        for(int j = 0; j < graph[i].size(); j++) if(graph[i][j] >= malicious_nodes) { flag = true; break; }
        
        if(!flag and i >= malicious_nodes) file << i << " [fillcolor=blue, style=filled];" << endl;

        for(int j = 0; j < graph[i].size(); j++) {
            if(i < graph[i][j]) file << i << " -- " << graph[i][j] << ";" << endl;
        }
    }

    file << "}" << endl;
    file.close();

    string command = "dot -Tpng vis/" + filename + ".dot" + " -o vis/" + filename + ".png";
    system(command.c_str());
    command = "rm vis/" + filename + ".dot";
    system(command.c_str());
}

void Network::save_network(string filename) {
    filesystem::create_directories("vis");

    ofstream file("vis/" + filename + ".txt");
    if(!file) {
        cout << "Error opening file" << endl;
        return;
    }

    for(int i = 0; i < n; i++) {
        file << i << " & ";
        int j = 0;
        for(j = 0; j < graph[i].size()-1; j++) {
            file << graph[i][j] << ", ";
        }

        file << graph[i][j] << " \\\\" << endl;
    }

    file.close();
}