#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <sstream>
#include <string>
#include <vector>


using namespace std;

struct Edge {
    int to;
    int edge_id;
    double time;
};

vector<bool> visited;
vector<double> dist;
vector<int> via;
ofstream trace("trace.txt");

int main(int argc, char** argv) {
    if (argc != 2){
        std::cerr << "usage: main <dir>\n";
        return 1;
    }
    int source = 3;
    int target = 1281;
    
    string dir = argv[1];
    vector<vector<Edge>> adj;
    vector<pair<int, int>> edge_endpoints;

    

    
    trace << "meta graph " << dir << '\n';
    trace << "meta source " << source << '\n';
    trace << "meta target " << target << '\n';

   
 

    trace.close();
}