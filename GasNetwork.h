#pragma once
#include <unordered_map>
#include <vector>
#include <queue>
#include <limits>
#include "Pipe.h"
#include "KC.h"

class GasNetwork {
public:
    /* -------------------------------------------------------------
       Edge – rendu public afin d’être visible depuis le code client
       ------------------------------------------------------------- */
    struct Edge {
        int   to;                // KC destination
        int   pipe_id;           // identifiant du tuyau utilisé
        using FlowType = long long;          // capacité très grande (public)
        FlowType capacity;       // 0 si le tuyau est en réparation
        FlowType flow;           // (non utilisé dans l’algorithme)
        float weight;            // longueur du tuyau ou +inf si en réparation
    };

private:
    /* -------------------------------------------------------------
       Données internes (toujours privées)
       ------------------------------------------------------------- */
    std::unordered_map<int, std::vector<Edge>> graph;   // graphe orienté
    std::unordered_map<int, Pipe>                pipes;   // toutes les tuyaux connus

    /* -------------------------------------------------------------
       Fonction auxiliaire de détection de cycles (DFS)
       ------------------------------------------------------------- */
    bool hasCycleDFS(int u,
                    std::unordered_map<int,int>& color,
                    const std::unordered_map<int,std::vector<Edge>>& g) const;

public:
    GasNetwork();

    /* -----------------------------------------------------------------
       Opérations de base
       ----------------------------------------------------------------- */
    bool addConnection(int from, int to, int pipe_id);   // crée une arête
    bool connectionExists(int from, int to) const;
    bool isPipeUsed(int pipe_id) const;
    bool isPipeInNetwork(int pipe_id) const;
    void removeConnectionByPipe(int pipe_id);
    bool canDeleteKC(int kc_id) const;
    bool hasCycle() const;
    bool isEmpty() const;
    void displayConnections() const;
    std::vector<int> topologicalSort(const std::unordered_map<int, KC>& companies) const;

    /* -----------------------------------------------------------------
       Accesseurs publics supplémentaires
       ----------------------------------------------------------------- */
    // Accès en lecture au graphe (necessaire pour les affichages externes)
    const std::unordered_map<int, std::vector<Edge>>& getGraph() const { return graph; }

    // Enregistrement d’un tuyau dès sa création (appelé depuis le menu principal)
    void registerPipe(int pipe_id, const Pipe& pipe);

    // Mise à jour d’un tuyau déjà présent dans le réseau (ex. changement de répar.)
    void updatePipeInNetwork(int pipe_id, const Pipe& pipe);

    // Calcul du débit maximal (const – ne modifie rien)
    long long calculateMaxFlow(int source, int sink) const;

    // Recherche du plus court chemin (Dijkstra – const)
    std::vector<int> findShortestPath(int source, int sink,
                                      const std::unordered_map<int, Pipe>& pipes) const;

    // Affichage complet (max‑flow + shortest path) – const
    void displayFlowAnalysis(int source, int sink,
                             const std::unordered_map<int, KC>& companies,
                             const std::unordered_map<int, Pipe>& pipes) const;
};
