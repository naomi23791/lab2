#include "main.h"
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <unordered_map>
#include <vector>
#include <functional>
#include <algorithm>
#include "Pipe.h"
#include "KC.h"
#include "Logger.h"
#include "GasNetwork.h"

using namespace std;

// -----------------------------------------------------------------
//  Helper pour éviter le warning « unused »
// -----------------------------------------------------------------
static inline void logAction(const std::string& action) { Logger::logAction(action); }

/*======================================================================
   SAVE / LOAD (inchangés)
======================================================================*/
void saveToFile(const std::unordered_map<int, Pipe>& pipes,
                const std::unordered_map<int, KC>& companies,
                const std::string& filename) {
    if (pipes.empty() && companies.empty()) {
        std::cout << "Nothing to save: no pipes or companies in memory.\n";
        logAction("Save aborted: nothing to save.");
        return;
    }
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Unable to open file for saving.\n";
        logAction("Failed to open file for saving: " + filename);
        return;
    }
    file << pipes.size() << std::endl;
    for (const auto& kv : pipes) file << kv.second << std::endl;
    file << companies.size() << std::endl;
    for (const auto& kv : companies) file << kv.second << std::endl;

    if (!file) {
        std::cout << "Error occurred during saving.\n";
        logAction("Error during saving: " + filename);
    } else {
        std::cout << "Data saved to file successfully.\n";
        logAction("Saved data to file: " + filename);
    }
}

bool loadFromFile(std::unordered_map<int, Pipe>& pipes,
                  std::unordered_map<int, KC>& companies,
                  std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Unable to open file for loading.\n";
        logAction("Failed to open file for loading: " + filename);
        return false;
    }
    size_t nPipes = 0, nCompanies = 0;
    if (!(file >> nPipes)) {
        std::cout << "File corrupted or invalid format (pipes count).\n";
        logAction("Corrupted file (pipes count): " + filename);
        return false;
    }
    pipes.clear();
    for (size_t i = 0; i < nPipes; ++i) {
        Pipe p;
        if (!(file >> p)) {
            std::cout << "File corrupted or invalid format (pipe data).\n";
            logAction("Corrupted file (pipe data): " + filename);
            return false;
        }
        int id = p.getId();
        if (id <= 0 || pipes.count(id)) {
            std::cout << "Invalid or duplicate pipe ID found: " << id << '\n';
            logAction("Invalid/duplicate pipe ID: " + std::to_string(id));
            return false;
        }
        pipes[id] = p;
        Logger::logAction("LOAD PIPE", p.getId(), p.getName(),
                         p.getLength(), p.getDiameter(), p.isRepair());
    }
    if (!(file >> nCompanies)) {
        std::cout << "File corrupted or invalid format (companies count).\n";
        logAction("Corrupted file (companies count): " + filename);
        return false;
    }
    companies.clear();
    for (size_t i = 0; i < nCompanies; ++i) {
        KC c;
        if (!(file >> c)) {
            std::cout << "File corrupted or invalid format (company data).\n";
            logAction("Corrupted file (company data): " + filename);
            return false;
        }
        int id = c.getId();
        if (id <= 0 || companies.count(id)) {
            std::cout << "Invalid or duplicate company ID found: " << id << '\n';
            logAction("Invalid/duplicate company ID: " + std::to_string(id));
            return false;
        }
        companies[id] = c;
        Logger::logAction("LOAD COMPANY", c.getId(), c.getName(),
                          c.getWorkshop(), c.getWorkshopInOperation(),
                          c.getClasses());
    }
    if (pipes.empty() && companies.empty()) {
        std::cout << "Nothing loaded: file contains no pipes or companies.\n";
        logAction("Load aborted: nothing in file.");
        return false;
    }
    std::cout << "Data loaded from file successfully.\n";
    logAction("Loaded data from file: " + filename);
    return true;
}

/*======================================================================
   AFFICHAGE du chemin le plus court **seul**
======================================================================*/
static void showShortestPathOnly(GasNetwork& network,
                                 const std::unordered_map<int, Pipe>& pipes,
                                 const std::unordered_map<int, KC>&   companies)
{
    if (network.isEmpty()) {
        std::cout << "The network is empty – you cannot compute a path.\n";
        std::cout << "Do you want to create the minimal data now? (1 for   yes / 0 for no) : ";
        int createNow; std::cin >> createNow;
        if (createNow != 1) { std::cout << "Operation cancelled.\n"; return; }

        /* ----- créer au moins 2 companies ----- */
        while (companies.size() < 2) {
            std::cout << "Creating a new company (need at least 2).\n";
            int newId = addCompany(const_cast<std::unordered_map<int, KC>&>(companies));
            (void)newId;
        }

        /* ----- créer au moins 1 pipe ----- */
        if (pipes.empty()) {
            std::cout << "Creating a new pipe (need at least 1).\n";
            int newId = addPipe(const_cast<std::unordered_map<int, Pipe>&>(pipes));
            (void)newId;
        }

        /* ----- enregistrer les pipes dans le réseau ----- */
        for (const auto& kv : pipes)
            network.registerPipe(kv.first, kv.second);
        std::cout << "Minimal data created. You can now request a shortest path.\n";
    }

    /* ----- demander les deux KC ----- */
    int source, sink;
    std::cout << "Enter source KC ID: "; std::cin >> source;
    std::cout << "Enter sink KC ID: "  ; std::cin >> sink;

    if (companies.find(source) == companies.end() ||
        companies.find(sink)   == companies.end()) {
        std::cout << "One or both KC IDs not found.\n";
        return;
    }

    std::vector<int> path = network.findShortestPath(source, sink, pipes);
    if (path.empty()) {
        std::cout << "No path could be found between the two companies.\n";
        return;
    }

    /* ----- affichage du résultat (identique à la partie du FlowAnalysis) ----- */
    std::cout << "\n--- Shortest Path (by distance) ---\n";
    float totalDist = 0.0f;
    using FlowType = GasNetwork::Edge::FlowType;
    FlowType bottleneck = std::numeric_limits<FlowType>::max();

    const auto& g = network.getGraph();   // accès en lecture au graphe

    for (size_t i = 0; i < path.size(); ++i) {
        int kc = path[i];
        if (companies.find(kc) != companies.end())
            std::cout << "  [" << i << "] KC " << kc << " (" << companies.at(kc).getName() << ")";
        else
            std::cout << "  [" << i << "] KC " << kc;

        if (i + 1 < path.size()) {
            int nxt = path[i + 1];
            const GasNetwork::Edge* eptr = nullptr;
            auto it = g.find(kc);
            if (it != g.end())
                for (const auto& e : it->second)
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
    std::cout << "  Number of hops (edges)       : " << (path.size() - 1) << "\n";
}

/*======================================================================
   GESTION DU RÉSEAU (menu « Network Management »)
======================================================================*/
void manageNetwork(std::unordered_map<int, Pipe>& pipes,
                  std::unordered_map<int, KC>&   companies,
                  GasNetwork& network)
{
    int choice;
    do {
        std::cout << "\n=== Network Management ===\n";
        std::cout << "1. Add connection (Pipe)\n";
        std::cout << "2. Display connections\n";
        std::cout << "3. Topological sort\n";
        std::cout << "4. Analyze flow (max-flow + shortest path)\n";
        std::cout << "5. Find shortest path only\n";
        std::cout << "0. Back\n";
        std::cout << "Your choice: ";
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            continue;
        }

        switch (choice) {
            /*-------------------------------------------------
               1 – Add connection (Pipe) – IMPLEMENTÉ
              -------------------------------------------------*/
            case 1: {
                bool abort = false;   // flag d’abandon

                /* ---------- 1️⃣  vérifier au moins 2 companies ---------- */
                while (companies.size() < 2 && !abort) {
                    std::cout << "You need at least 2 companies to create a connection.\n";
                        std::cout << "Create a new company now? (1 for yes / 0 for no): ";
                    int create; std::cin >> create;
                    if (create == 1) {
                        int newId = addCompany(companies);
                        (void)newId;
                    } else {
                        std::cout << "Cannot create a connection without enough companies.\n";
                        abort = true;
                    }
                }
                if (abort) break;

                /* ---------- 2️⃣  vérifier au moins 1 pipe ---------- */
                if (pipes.empty() && !abort) {
                    std::cout << "No pipes exist. Create one now? (1 for yes / 0 for no): ";
                    int create; std::cin >> create;
                    if (create == 1) {
                        int newId = addPipe(pipes);
                        (void)newId;
                        network.registerPipe(newId, pipes[newId]);   // enregistrement
                    } else {
                        std::cout << "Cannot create a connection without any pipe.\n";
                        abort = true;
                    }
                }
                if (abort) break;

                /* ---------- 3️⃣  saisie des KC source / dest ---------- */
                int from_kc, to_kc;
                std::cout << "Enter source KC ID: ";
                std::cin >> from_kc;
                std::cout << "Enter destination KC ID: ";
                std::cin >> to_kc;

                if (from_kc == to_kc) {
                    std::cout << "Source and destination cannot be the same.\n";
                    break;
                }
                if (companies.find(from_kc) == companies.end() ||
                    companies.find(to_kc)   == companies.end()) {
                    std::cout << "One or both KC IDs not found.\n";
                    break;
                }

                /* ---------- 4️⃣  diamètres disponibles (pipes non‑utilisés) ---------- */
                std::cout << "Available pipe diameters (unused pipes only): ";
                bool first = true;
                for (const auto& kv : pipes) {
                    if (!network.isPipeUsed(kv.first)) {
                        if (!first) std::cout << ", ";
                        std::cout << kv.second.getDiameter();
                        first = false;
                    }
                }
                std::cout << "\n";

                if (first) {   // aucun pipe libre
                    std::cout << "No unused pipes left. Create a new pipe? (1 for   yes / 0 for no): ";
                    int create; std::cin >> create;
                    if (create == 1) {
                        int newId = addPipe(pipes);
                        (void)newId;
                        network.registerPipe(newId, pipes[newId]);
                        std::cout << "New pipe added. Re‑displaying available diameters…\n";
                        first = true;
                        for (const auto& kv : pipes) {
                            if (!network.isPipeUsed(kv.first)) {
                                if (!first) std::cout << ", ";
                                std::cout << kv.second.getDiameter();
                                first = false;
                            }
                        }
                        std::cout << "\n";
                        if (first) {
                            std::cout << "Still no unused pipes – aborting.\n";
                            break;
                        }
                    } else {
                        std::cout << "Aborting connection creation.\n";
                        break;
                    }
                }

                /* ---------- 5️⃣  choix du diamètre ---------- */
                int desiredDiameter;
                std::cout << "Enter diameter of pipe to use (500/700/1000/1400): ";
                while (true) {
                    if (std::cin >> desiredDiameter &&
                        (desiredDiameter==500 || desiredDiameter==700 ||
                         desiredDiameter==1000||desiredDiameter==1400)) {
                        break;
                    } else {
                        std::cout << "Invalid input. Try again: ";
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    }
                }

                /* ---------- 6️⃣  recherche du premier pipe libre du diamètre choisi ---------- */
                int chosenPipeId = -1;
                for (const auto& kv : pipes) {
                    if (!network.isPipeUsed(kv.first) && kv.second.getDiameter() == desiredDiameter) {
                        chosenPipeId = kv.first;
                        break;          // le plus petit ID libre sera choisi
                    }
                }
                if (chosenPipeId == -1) {
                    std::cout << "No unused pipe with that diameter. Try another diameter.\n";
                    break;
                }

                /* ---------- 7️⃣  création de la connexion ---------- */
                if (network.connectionExists(from_kc, to_kc)) {
                    std::cout << "Error: Connection already exists between these KCs!\n";
                    break;
                }

                if (network.addConnection(from_kc, to_kc, chosenPipeId)) {
                    std::cout << "Connection created successfully!\n";
                    logAction("Added connection: KC" + std::to_string(from_kc) +
                              " -> KC" + std::to_string(to_kc) +
                              " using pipe " + std::to_string(chosenPipeId));
                    network.updatePipeInNetwork(chosenPipeId, pipes[chosenPipeId]);
                } else {
                    std::cout << "Failed to create connection (would create cycle)!\n";
                }
                break;
            }

            /*-------------------------------------------------
               2 – Display connections (inchangée)
              -------------------------------------------------*/
            case 2:
                if (network.isEmpty())
                    std::cout << "Network is empty. Create some connections first.\n";
                else
                    network.displayConnections();
                break;

            /*-------------------------------------------------
               3 – Topological sort (inchangée)
              -------------------------------------------------*/
            case 3: {
                if (network.isEmpty())
                    std::cout << "Cannot perform topological sort: network is empty!\n";
                else if (network.hasCycle())
                    std::cout << "Network contains cycles! Topological sort is impossible.\n";
                else {
                    std::vector<int> sorted = network.topologicalSort(companies);
                    if (!sorted.empty()) {
                        std::cout << "\nTopological sort result:\n";
                        for (size_t i = 0; i < sorted.size(); ++i)
                            std::cout << "KC " << sorted[i] << " (" << companies.at(sorted[i]).getName() << ")\n";
                    }
                }
                break;
            }

            /*-------------------------------------------------
               4 – Analyse flow (max‑flow + shortest path)
              -------------------------------------------------*/
            case 4: {
                if (network.isEmpty()) {
                    std::cout << "Network is empty.\n";
                } else {
                    int source, sink;
                    std::cout << "Enter source KC: ";
                    std::cin >> source;
                    std::cout << "Enter sink KC: ";
                    std::cin >> sink;

                    if (companies.find(source) == companies.end() ||
                        companies.find(sink)   == companies.end()) {
                        std::cout << "One or both KC IDs not found.\n";
                        break;
                    }
                    network.displayFlowAnalysis(source, sink, companies, pipes);
                }
                break;
            }

            /*-------------------------------------------------
               5 – Find shortest path only (déjà implémenté)
              -------------------------------------------------*/
            case 5: {
                showShortestPathOnly(network, pipes, companies);
                break;
            }

            case 0: break;
            default: std::cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}

/*======================================================================
   SUPPRESSION DE PIPE / COMPANY (inchangés)
======================================================================*/
void deletePipe(std::unordered_map<int, Pipe>& pipes, GasNetwork& network) {
    if (pipes.empty()) { std::cout << "No pipes to delete.\n"; return; }
    std::cout << "Available pipes:\n";
    for (auto& kv : pipes) {
        std::cout << "ID " << kv.first << ": " << kv.second.getName();
        if (network.isPipeInNetwork(kv.first)) std::cout << " (Used in network)";
        std::cout << "\n";
    }
    int id; std::cout << "Enter pipe ID to delete: "; std::cin >> id;
    if (!pipes.count(id)) { std::cout << "Pipe not found.\n"; return; }
    if (network.isPipeInNetwork(id)) {
        char c; std::cout << "Pipe is used – remove its connection first? (y/n): ";
        std::cin >> c;
        if (c=='y'||c=='Y') network.removeConnectionByPipe(id);
        else { std::cout << "Deletion cancelled.\n"; return; }
    }
    pipes.erase(id);
    std::cout << "Pipe deleted successfully.\n";
    Logger::logAction("Deleted pipe " + std::to_string(id));
}

void deleteKC(std::unordered_map<int, KC>& companies, GasNetwork& network) {
    if (companies.empty()) { std::cout << "No companies to delete.\n"; return; }
    std::cout << "Available companies:\n";
    for (auto& kv : companies) {
        std::cout << "ID " << kv.first << ": " << kv.second.getName();
        if (!network.canDeleteKC(kv.first)) std::cout << " (Used in network)";
        std::cout << "\n";
    }
    int id; std::cout << "Enter company ID to delete: "; std::cin >> id;
    if (!companies.count(id)) { std::cout << "Company not found.\n"; return; }
    if (!network.canDeleteKC(id)) {
        std::cout << "This company is used in the network and cannot be deleted.\n";
        return;
    }
    companies.erase(id);
    std::cout << "Company deleted successfully.\n";
    Logger::logAction("Deleted company " + std::to_string(id));
}

/*======================================================================
   MAIN
======================================================================*/
int main() {
    std::unordered_map<int, Pipe> pipes;
    std::unordered_map<int, KC>   companies;
    GasNetwork network;
    int choice;
    std::string filename = "data.txt";

    do {
        std::cout << "\n==== Main Menu ====\n";
        std::cout << "1. Add a pipe\n";
        std::cout << "2. Add a company (KC)\n";
        std::cout << "3. Manage pipes\n";
        std::cout << "4. Manage companies\n";
        std::cout << "5. Save to file\n";
        std::cout << "6. Load from file\n";
        std::cout << "7. Change file name\n";
        std::cout << "8. Network management\n";
        std::cout << "9. Delete pipe\n";
        std::cout << "10. Delete company\n";
        std::cout << "0. Quit\n";
        std::cout << "Your choice: ";
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            continue;
        }

        switch (choice) {
            case 1: {
                int newId = addPipe(pipes);
                (void)newId;                         // silence warning
                network.registerPipe(newId, pipes[newId]); // enregistrement du pipe
                logAction("Added pipe");
                break;
            }
            case 2: {
                int newId = addCompany(companies);
                (void)newId;
                logAction("Added company");
                break;
            }
            case 3: managePipes(pipes, network); break;
            case 4: manageCompanies(companies);   break;
            case 5: saveToFile(pipes, companies, filename); break;
            case 6: {
                loadFromFile(pipes, companies, filename);
                break;
            }
            case 7: std::cout << "Enter filename: "; std::cin >> filename;
                    logAction("Set filename: " + filename);
                    break;
            case 8: manageNetwork(pipes, companies, network); break;
            case 9: deletePipe(pipes, network); break;
            case 10: deleteKC(companies, network); break;
            case 0: std::cout << "Goodbye!\n"; logAction("Exited program"); break;
            default: std::cout << "Invalid choice.\n";
        }
    } while (choice != 0);
    return 0;
}
