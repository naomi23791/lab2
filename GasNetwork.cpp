#include "GasNetwork.h"
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <queue>
#include <set>
#include <map>
#include <functional>
#include <vector>
#include <limits>
#include <cmath>

/*======================================================================
   Constructeur
======================================================================*/
GasNetwork::GasNetwork() {}

/*======================================================================
   BASE HELPERS
======================================================================*/
bool GasNetwork::connectionExists(int from, int to) const {
    auto it = graph.find(from);
    if (it == graph.end()) return false;
    for (const Edge& e : it->second)
        if (e.to == to) return true;
    return false;
}

/*======================================================================
   REGISTER PIPE – enregistrement dès la création
======================================================================*/
void GasNetwork::registerPipe(int pipe_id, const Pipe& pipe) {
    pipes[pipe_id] = pipe;                     // on garde les infos du tuyau
}

/*======================================================================
   ADD CONNECTION – initialise capacity/weight, empêche les cycles
======================================================================*/
bool GasNetwork::addConnection(int from, int to, int pipe_id) {
    if (connectionExists(from, to)) return false;          // déjà existante

    // le tuyau doit être présent dans le registre de pipes
    auto pIt = pipes.find(pipe_id);
    if (pIt == pipes.end()) {
        std::cerr << "Pipe ID " << pipe_id << " not registered in the network.\n";
        return false;
    }
    const Pipe& p = pIt->second;

    Edge e;
    e.to       = to;
    e.pipe_id  = pipe_id;
    e.capacity = static_cast<Edge::FlowType>(p.getCapacity());   
    e.flow     = 0;
    e.weight   = p.getWeight();                                

    // insertion temporaire → test de cycle
    graph[from].push_back(e);
    if (hasCycle()) {                     // crée un cycle → annuler
        graph[from].pop_back();
        return false;
    }
    return true;
}

/*======================================================================
   PIPE‑RELATED QUERIES
======================================================================*/
bool GasNetwork::isPipeUsed(int pipe_id) const {
    for (const auto& kv : graph)
        for (const Edge& e : kv.second)
            if (e.pipe_id == pipe_id) return true;
    return false;
}
bool GasNetwork::isPipeInNetwork(int pipe_id) const { return isPipeUsed(pipe_id); }

void GasNetwork::removeConnectionByPipe(int pipe_id) {
    for (auto& kv : graph) {
        auto& vec = kv.second;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
                                 [pipe_id](const Edge& e){ return e.pipe_id == pipe_id; }),
                  vec.end());
    }
}

/*======================================================================
   KC‑RELATED QUERIES
======================================================================*/
bool GasNetwork::canDeleteKC(int kc_id) const {
    if (graph.find(kc_id) != graph.end()) return false;          // KC source
    for (const auto& kv : graph)
        for (const Edge& e : kv.second)
            if (e.to == kc_id) return false;                    // KC destination
    return true;
}

/*======================================================================
   CYCLE DETECTION (DFS + coloriage)
======================================================================*/
bool GasNetwork::hasCycleDFS(int u,
                             std::unordered_map<int,int>& color,
                             const std::unordered_map<int,std::vector<Edge>>& g) const {
    color[u] = 1;                                            // gris
    auto it = g.find(u);
    if (it != g.end()) {
        for (const Edge& e : it->second) {
            int v = e.to;
            if (color[v] == 1) return true;                  // back‑edge → cycle
            if (color[v] == 0 && hasCycleDFS(v, color, g)) return true;
        }
    }
    color[u] = 2;                                            // noir
    return false;
}

/* detection de cycle (prend en compte toutes les KC, même celles qui ne sont que destinations) */
bool GasNetwork::hasCycle() const {
    std::unordered_map<int,int> color;
    // on crée un sommet pour chaque KC présent soit en tant que source soit en tant que destination
    for (const auto& kv : graph) {
        color[kv.first] = 0;
        for (const Edge& e : kv.second) color[e.to] = 0;
    }
    for (auto& kv : color)
        if (kv.second == 0 && hasCycleDFS(kv.first, color, graph)) return true;
    return false;
}

/*======================================================================
   EMPTY / DISPLAY
======================================================================*/
bool GasNetwork::isEmpty() const { return graph.empty(); }

void GasNetwork::displayConnections() const {
    std::cout << "\n=== Network Connections ===\n";
    for (const auto& kv : graph) {
        int from = kv.first;
        for (const Edge& e : kv.second)
            std::cout << "KC " << from << " -> KC " << e.to
                      << " (Pipe ID: " << e.pipe_id << ")\n";
    }
}

/*======================================================================
   TOPOLOGICAL SORT (Kahn)
======================================================================*/
std::vector<int> GasNetwork::topologicalSort(const std::unordered_map<int, KC>& companies) const {
    std::unordered_map<int,int> indeg;
    std::unordered_map<int,std::vector<int>> adj;

    for (const auto& kv : companies) {
        indeg[kv.first] = 0;
        adj[kv.first] = {};
    }

    for (const auto& kv : graph) {
        int from = kv.first;
        for (const Edge& e : kv.second) {
            adj[from].push_back(e.to);
            indeg[e.to]++;                     // comptage indegree
        }
    }

    std::queue<int> q;
    for (const auto& kv : indeg)
        if (kv.second == 0) q.push(kv.first);

    std::vector<int> order;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (int v : adj[u])
            if (--indeg[v] == 0) q.push(v);
    }

    if (order.size() != companies.size()) {
        std::cout << "Topological sort failed: cycle detected!\n";
        return {};
    }
    return order;
}

/*======================================================================
   UPDATE PIPE – après modification de l’état (réparation / opération)
======================================================================*/
void GasNetwork::updatePipeInNetwork(int pipe_id, const Pipe& pipe) {
    for (auto& kv : graph)
        for (Edge& e : kv.second)
            if (e.pipe_id == pipe_id) {
                e.capacity = static_cast<Edge::FlowType>(pipe.getCapacity());
                e.weight   = pipe.getWeight();
            }
}


long long GasNetwork::calculateMaxFlow(int source, int sink) const {
    // --------- 1️⃣  collecte de tous les nœuds ----------
    std::set<int> nodes{source, sink};
    for (const auto& kv : graph) {
        nodes.insert(kv.first);
        for (const Edge& e : kv.second) nodes.insert(e.to);
    }
    if (!nodes.count(source) || !nodes.count(sink)) {
        std::cout << "Source or sink not present in the network.\n";
        return 0;
    }

    // --------- 2️⃣  index ↔ id ----------
    std::map<int,int> idx;
    int i = 0;
    for (int v : nodes) idx[v] = i++;

    int n = nodes.size();
    std::vector<std::vector<long long>> cap(n,
        std::vector<long long>(n, 0));

    // --------- 3️⃣  remplissage de la matrice ----------
    for (const auto& kv : graph) {
        int u = idx.at(kv.first);
        for (const Edge& e : kv.second) {
            int v = idx.at(e.to);
            cap[u][v] += e.capacity;                 // somme des capacités multiples
        }
    }

    int s = idx.at(source);
    int t = idx.at(sink);
    long long maxFlow = 0;
    std::vector<int> parent(n);

    // --------- 4️⃣  boucle principale (BFS) ----------
    while (true) {
        std::fill(parent.begin(), parent.end(), -1);
        parent[s] = -2;                               // source marquée

        std::queue<int> q;
        q.push(s);

        // BFS : recherche d’un chemin augmentant
        while (!q.empty() && parent[t] == -1) {
            int u = q.front(); q.pop();
            for (int v = 0; v < n; ++v)
                if (parent[v] == -1 && cap[u][v] > 0) {
                    parent[v] = u;
                    q.push(v);
                }
        }
        if (parent[t] == -1) break;                     // plus de chemin

        long long pathFlow = std::numeric_limits<long long>::max();
        for (int v = t; v != s; v = parent[v])
            pathFlow = std::min(pathFlow, cap[parent[v]][v]);

        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            cap[u][v] -= pathFlow;
            cap[v][u] += pathFlow;
        }
        maxFlow += pathFlow;
    }
    return maxFlow;
}

/*======================================================================
   SHORTEST PATH – Dijkstra (priority_queue)
   (marqué const)
======================================================================*/
std::vector<int> GasNetwork::findShortestPath(int source, int sink,
                                    const std::unordered_map<int, Pipe>& /*pipes*/) const {
    using Weight = float;
    const Weight INF = std::numeric_limits<Weight>::infinity();

    // --------- distances initiales ----------
    std::unordered_map<int,Weight> dist;
    std::unordered_map<int,int>    parent;
    for (const auto& kv : graph) {
        dist[kv.first] = INF; parent[kv.first] = -1;
        for (const Edge& e : kv.second) {
            dist[e.to] = INF; parent[e.to] = -1;
        }
    }
    if (!dist.count(source) || !dist.count(sink)) {
        std::cout << "Source or sink not present in the network.\n";
        return {};
    }
    dist[source] = 0.0f;

    // --------- min‑heap ----------
    using PQItem = std::pair<Weight,int>;               // (dist, node)
    std::priority_queue<PQItem,
                        std::vector<PQItem>,
                        std::greater<PQItem>> pq;
    pq.emplace(0.0f, source);

    while (!pq.empty()) {
        auto top = pq.top(); pq.pop();
        Weight d = top.first;
        int    u = top.second;
        if (d != dist[u]) continue;                     // entrée périmée
        if (u == sink) break;                            // arrivé

        auto it = graph.find(u);
        if (it == graph.end()) continue;
        for (const Edge& e : it->second) {
            if (std::isinf(e.weight)) continue;           // tuyau en réparation
            int v = e.to;
            Weight nd = d + e.weight;
            if (nd < dist[v]) {
                dist[v]   = nd;
                parent[v] = u;
                pq.emplace(nd, v);
            }
        }
    }

    if (dist[sink] == INF) {
        std::cout << "No path found from " << source << " to " << sink << ".\n";
        return {};
    }

    // reconstruction du chemin
    std::vector<int> path;
    for (int cur = sink; cur != -1; cur = parent[cur])
        path.push_back(cur);
    std::reverse(path.begin(), path.end());
    return path;
}

/*======================================================================
   DISPLAY FLOW ANALYSIS (max‑flow + shortest path)
======================================================================*/
void GasNetwork::displayFlowAnalysis(int source, int sink,
        const std::unordered_map<int, KC>& companies,
        const std::unordered_map<int, Pipe>& pipes) const {

    std::cout << "\n=== Flow Analysis from KC " << source
              << " to KC " << sink << " ===\n";

    if (companies.find(source) != companies.end())
        std::cout << "Source: KC " << source << " (" << companies.at(source).getName() << ")\n";
    if (companies.find(sink) != companies.end())
        std::cout << "Sink:   KC " << sink   << " (" << companies.at(sink).getName()   << ")\n";

    if (source == sink) {
        std::cout << "Source and sink are identical – nothing to analyse.\n";
        return;
    }

    // -------------------- MAX‑FLOW --------------------
    long long maxFlow = calculateMaxFlow(source, sink);
    std::cout << "\n--- Maximum Flow ---\n";
    std::cout << "Maximum flow from KC " << source << " to KC " << sink
              << " : " << maxFlow << " m³/h\n";

    // -------------------- SHORTEST PATH --------------------
    std::vector<int> shortestPath = findShortestPath(source, sink, pipes);
    if (shortestPath.empty()) {
        std::cout << "\n--- Shortest Path ---\n";
        std::cout << "No path found from KC " << source << " to KC " << sink << ".\n";
        return;
    }

    std::cout << "\n--- Shortest Path (by distance) ---\n";
    float totalDist = 0.0f;
    using FlowType = Edge::FlowType;
    FlowType bottleneck = std::numeric_limits<FlowType>::max();

    for (size_t i = 0; i < shortestPath.size(); ++i) {
        int kc = shortestPath[i];
        if (companies.find(kc) != companies.end())
            std::cout << "  [" << i << "] KC " << kc << " ("
                      << companies.at(kc).getName() << ")";
        else
            std::cout << "  [" << i << "] KC " << kc;

        if (i + 1 < shortestPath.size()) {
            int nxt = shortestPath[i + 1];
            const Edge* eptr = nullptr;
            auto it = graph.find(kc);
            if (it != graph.end())
                for (const Edge& e : it->second)
                    if (e.to == nxt) { eptr = &e; break; }

            if (eptr) {
                const Pipe& p = pipes.at(eptr->pipe_id);
                totalDist += p.getLength();
                bottleneck = std::min(bottleneck,
                                     static_cast<FlowType>(p.getCapacity()));
                std::cout << "\n        --[Pipe ID:" << p.getId()
                          << " Name:" << p.getName()
                          << " Len:" << (int)p.getLength()
                          << "m Cap:" << p.getCapacity()
                          << " m^3/h]--> ";
            }
        }
        std::cout << "\n";
    }

    std::cout << "\nPath statistics:\n";
    std::cout << "  Total distance               : " << totalDist << " m\n";
    if (bottleneck == std::numeric_limits<FlowType>::max())
        std::cout << "  Path capacity (bottleneck)   : undefined (no pipe traversed)\n";
    else
        std::cout << "  Path capacity (bottleneck)   : " << bottleneck << " m^3/h\n";
    std::cout << "  Number of hops (edges)       : " << (shortestPath.size() - 1) << "\n";
}
