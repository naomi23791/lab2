#include "GasNetwork.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <algorithm>  // Pour std::remove_if et std::remove

bool GasNetwork::hasCycle() {
    std::unordered_map<int, bool> visited;
    std::unordered_map<int, bool> recursionStack;
    
    // Initialize
    for (const auto& conn : connections) {
        visited[conn.from_kc] = false;
        visited[conn.to_kc] = false;
        recursionStack[conn.from_kc] = false;
        recursionStack[conn.to_kc] = false;
    }
    
    // Check for cycle starting from each node
    for (const auto& pair : adjacencyList) {
        if (!visited[pair.first]) {
            if (hasCycleUtil(pair.first, visited, recursionStack)) {
                return true;
            }
        }
    }
    return false;
}

bool GasNetwork::hasCycleUtil(int v, std::unordered_map<int, bool>& visited,std::unordered_map<int, bool>& recursionStack) {
    visited[v] = true;
    recursionStack[v] = true;
    
    // Check all adjacent vertices
    if (adjacencyList.count(v)) {
        for (int u : adjacencyList[v]) {
            // If vertex is not visited and its recursive call returns true
            if (!visited[u] && hasCycleUtil(u, visited, recursionStack)) {
                return true;
            }
            // If vertex is in recursion stack, we found a cycle
            else if (recursionStack[u]) {
                return true;
            }
        }
    }
    
    recursionStack[v] = false;
    return false;
}

bool GasNetwork::addConnection(int from_kc, int to_kc, int pipe_id) {
    // Check prerequisites
    if (from_kc == to_kc) {
        std::cout << "Error: Cannot connect a KC to itself!\n";
        return false;
    }

    if (connectionExists(from_kc, to_kc)) {
        std::cout << "Error: Connection already exists between these KCs!\n";
        return false;
    }

    // Test if adding this connection would create a cycle
    if (adjacencyList.find(from_kc) == adjacencyList.end()) {
        adjacencyList[from_kc] = std::vector<int>();
    }
    adjacencyList[from_kc].push_back(to_kc);
    
    if (hasCycle()) {
        // Remove the test connection from adjacencyList
        adjacencyList[from_kc].pop_back();
        if (adjacencyList[from_kc].empty()) {
            adjacencyList.erase(from_kc);
        }
        std::cout << "Error: Adding this connection would create a cycle!\n";
        std::cout << "The network must remain acyclic for topological sorting.\n";
        return false;
    }
    
    // If no cycle, add the connection permanently
    connections.push_back({from_kc, to_kc, pipe_id});
    return true;
}

// bool GasNetwork::hasCycle() {
//     std::unordered_map<int, int> visited;  // 0=non visité, 1=en cours, 2=terminé
    
//     // Initialiser tous les nœuds comme non visités
//     for (const auto& conn : connections) {
//         visited[conn.from_kc] = 0;
//         visited[conn.to_kc] = 0;
//     }
    
//     // Vérifier chaque nœud non visité
//     for (const auto& pair : adjacencyList) {
//         if (visited[pair.first] == 0) {
//             if (dfsCheckCycle(pair.first, visited)) {
//                 return true;  // Cycle trouvé
//             }
//         }
//     }
//     return false;
// }
bool GasNetwork::dfs(int v, std::unordered_map<int, int>& visited) {
    visited[v] = 1;  // Marquer comme en cours de visite
    
    // Vérifier tous les voisins
    if (adjacencyList.count(v)) {
        for (int u : adjacencyList[v]) {
            if (visited[u] == 1) {  // Nœud déjà en cours = cycle
                return true;
            }
            if (visited[u] == 0 && dfs(u, visited)) {  // Nœud non visité
                return true;
            }
        }
    }
    
    visited[v] = 2;  // Marquer comme complètement visité
    return false;
}

std::vector<int> GasNetwork::topologicalSort(const std::unordered_map<int, KC>& kcs) {
    if (connections.empty()) {
        std::cout << "Cannot perform topological sort: network is empty!\n";
        return std::vector<int>();
    }

    if (hasCycle()) {
        std::cout << "Cannot perform topological sort: network contains cycles!\n";
        return std::vector<int>();
    }

    sortedIds.clear();
    std::unordered_map<int, int> inDegree;
    
    // Initialize in-degree for all KCs
    for (const auto& pair : kcs) {
        inDegree[pair.first] = 0;
    }
    
    // Calculate in-degree
    for (const auto& conn : connections) {
        inDegree[conn.to_kc]++;
    }
    
    // Find sources (nodes with in-degree 0)
    std::queue<int> q;
    for (const auto& pair : kcs) {
        if (inDegree[pair.first] == 0) {
            q.push(pair.first);
        }
    }
    
    while (!q.empty()) {
        int v = q.front();
        q.pop();
        sortedIds.push_back(v);
        
        // Pour chaque voisin de v
        if (adjacencyList.count(v)) {
            for (int u : adjacencyList[v]) {
                inDegree[u]--;
                if (inDegree[u] == 0) {
                    q.push(u);
                }
            }
        }
    }
    
    return sortedIds;
}

bool GasNetwork::connectionExists(int from_kc, int to_kc) const {
    for (const auto& conn : connections) {
        if (conn.from_kc == from_kc && conn.to_kc == to_kc) {
            return true;
        }
    }
    return false;
}

void GasNetwork::displayConnections() const {
    if (connections.empty()) {
        std::cout << "No connections in the network.\n";
        std::cout << "To create a network, you need:\n";
        std::cout << "1. At least 2 KCs\n";
        std::cout << "2. At least 1 pipe\n";
        std::cout << "Use the main menu options 1 and 2 to create these first.\n";
        return;
    }
    std::cout << "\nNetwork Connections:\n";
    std::cout << "Format: Source -> Destination (Pipe ID)\n";
    
    // Grouper par source
    std::unordered_map<int, std::vector<std::pair<int, int>>> sourceGroups;
    for (const auto& conn : connections) {
        sourceGroups[conn.from_kc].push_back({conn.to_kc, conn.pipe_id});
    }
    
    // Afficher chaque source et ses destinations
    for (const auto& group : sourceGroups) {
        std::cout << "\nFrom KC " << group.first << ":\n";
        for (const auto& dest : group.second) {
            std::cout << "  --> KC " << dest.first 
                     << " (Pipe ID: " << dest.second << ")\n";
        }
    }
}

bool GasNetwork::isPipeUsed(int pipe_id) const {
    for (const auto& conn : connections) {
        if (conn.pipe_id == pipe_id) return true;
    }
    return false;
}

void GasNetwork::clear() {
    connections.clear();
    adjacencyList.clear();
    sortedIds.clear();
}

void GasNetwork::saveToFile(std::ofstream& file) const {
    file << connections.size() << '\n';
    for (const auto& conn : connections) {
        file << conn.from_kc << ' ' << conn.to_kc << ' ' << conn.pipe_id << '\n';
    }
}

bool GasNetwork::dfsCheckCycle(int v, std::unordered_map<int, int>& visited) {
    visited[v] = 1;  // Marquer comme en cours de visite
    
    // Vérifier tous les voisins
    if (adjacencyList.count(v)) {
        for (int u : adjacencyList[v]) {
            if (visited[u] == 1) {  // Nœud déjà en cours = cycle
                return true;
            }
            if (visited[u] == 0 && dfsCheckCycle(u, visited)) {  // Nœud non visité
                return true;
            }
        }
    }
    
    visited[v] = 2;  // Marquer comme complètement visité
    return false;
}

void GasNetwork::loadFromFile(std::ifstream& file) {
    clear();
    size_t count;
    file >> count;
    for (size_t i = 0; i < count; ++i) {
        Connection conn;
        file >> conn.from_kc >> conn.to_kc >> conn.pipe_id;
        addConnection(conn.from_kc, conn.to_kc, conn.pipe_id);
    }
}

bool GasNetwork::canDeletePipe(int pipe_id) const {
    return !isPipeUsed(pipe_id);
}

bool GasNetwork::canDeleteKC(int kc_id) const {
    for (const auto& conn : connections) {
        if (conn.from_kc == kc_id || conn.to_kc == kc_id) {
            return false;
        }
    }
    return true;
}

void GasNetwork::removeConnection(int pipe_id) {
    auto new_end = std::remove_if(connections.begin(), connections.end(),
        [pipe_id](const Connection& conn) { return conn.pipe_id == pipe_id; });
    connections.erase(new_end, connections.end());

    // Update adjacencyList
    for (auto it = adjacencyList.begin(); it != adjacencyList.end();) {
        bool found = false;
        for (const auto& conn : connections) {
            if (conn.from_kc == it->first) {
                found = true;
                break;
            }
        }
        if (!found) {
            it = adjacencyList.erase(it);
        } else {
            ++it;
        }
    }
}

void GasNetwork::removeConnectionByPipe(int pipe_id) {
    auto it = std::find_if(connections.begin(), connections.end(),
        [pipe_id](const Connection& conn) { return conn.pipe_id == pipe_id; });
    
    if (it != connections.end()) {
        int from_kc = it->from_kc;
        int to_kc = it->to_kc;
        
        // Remove from connections vector
        connections.erase(it);
        
        // Update adjacencyList
        if (adjacencyList.find(from_kc) != adjacencyList.end()) {
            auto& vec = adjacencyList[from_kc];
            vec.erase(std::remove(vec.begin(), vec.end(), to_kc), vec.end());
            if (vec.empty()) {
                adjacencyList.erase(from_kc);
            }
        }
    }
}

void GasNetwork::removeConnectionsWithKC(int kc_id) {
    auto new_end = std::remove_if(connections.begin(), connections.end(),
        [kc_id](const Connection& conn) { 
            return conn.from_kc == kc_id || conn.to_kc == kc_id; 
        });
    connections.erase(new_end, connections.end());

    // Remove from adjacencyList
    adjacencyList.erase(kc_id);
    
    // Remove this KC from other KC's adjacency lists
    for (auto& pair : adjacencyList) {
        auto& vec = pair.second;
        auto it = std::remove(vec.begin(), vec.end(), kc_id);
        vec.erase(it, vec.end());
    }
}

bool GasNetwork::isPipeInNetwork(int pipe_id) const {
    return std::any_of(connections.begin(), connections.end(),
        [pipe_id](const Connection& conn) { return conn.pipe_id == pipe_id; });
}