#pragma once
#include <unordered_map>
#include <vector>
#include <queue>
#include "Pipe.h"
#include "KC.h"

struct Connection {
    int from_kc;
    int to_kc;
    int pipe_id;
};

class GasNetwork {
private:
    std::vector<Connection> connections;
    std::unordered_map<int, std::vector<int>> adjacencyList;
    std::vector<bool> used;
    std::vector<int> sortedIds;

public:
    bool addConnection(int from_kc, int to_kc, int pipe_id);
    bool hasCycle();
    std::vector<int> topologicalSort(const std::unordered_map<int, KC>& kcs);
    void displayConnections() const;
    bool isPipeUsed(int pipe_id) const;
    void clear();
    void saveToFile(std::ofstream& file) const;
    void loadFromFile(std::ifstream& file);
    bool connectionExists(int from_kc, int to_kc) const;
    bool isEmpty() const { return connections.empty(); }
    bool canDeleteKC(int kc_id) const;
    bool canDeletePipe(int pipe_id) const;
    void removeConnection(int pipe_id);
    void removeConnectionByPipe(int pipe_id);
    void removeConnectionsWithKC(int kc_id);
    bool isPipeInNetwork(int pipe_id) const;

private:
    bool dfs(int v, std::unordered_map<int, int>& visited);
    bool dfsCheckCycle(int v, std::unordered_map<int, int>& visited);
    bool hasCycleUtil(int v, std::unordered_map<int, bool>& visited,std::unordered_map<int, bool>& recursionStack);
    
};