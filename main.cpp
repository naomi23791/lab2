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
#include "RedirectWrapper.h"
#include "Logger.h"
#include "GasNetwork.h"
using namespace std;

// --- Simple passthrough for legacy calls ---
static inline void logAction(const std::string& action) { Logger::logAction(action); }

// Add as a global variable:
GasNetwork network;

// --- Add ---
void addPipe(unordered_map<int, Pipe>& pipes, int& nextPipeId) {
    Pipe p;
    p.input(nextPipeId);
    pipes[nextPipeId] = p;
    cout << "Pipe added with ID: " << nextPipeId << endl;
    Logger::logPipeSimple(p.getName(), p.getLength(), p.getDiameter(), p.isRepair());
    nextPipeId++;
}

void addCompany(unordered_map<int, KC>& companies, int& nextCompanyId) {
    KC c;
    c.input(nextCompanyId);
    companies[nextCompanyId] = c;
    cout << "Company added with ID: " << nextCompanyId << endl;
    Logger::logCompanySimple(c.getName(), c.getWorkshop(), c.getWorkshopInOperation(), c.getClasses());
    nextCompanyId++;
}

void manageNetwork(unordered_map<int, Pipe>& pipes, unordered_map<int, KC>& companies, int& nextPipeId,int& nextCompanyId) {
    
    if (companies.size() < 2) {
        cout << "\nError: At least 2 KCs are required to create a network.\n";
        cout << "Current number of KCs: " << companies.size() << "\n";
        cout << "Would you like to create a KC now? (1-Yes/0-No): ";
        int choice;
        while (true) {
            cin >> choice;
            if (choice == 1 || choice == 0) break;
            cout << "Invalid choice. Please enter 1 for Yes or 0 for No: ";
        }
        if (choice == 1) {
            addCompany(companies, nextCompanyId);
        }
        return;
    }

    if (pipes.empty()) {
        cout << "\nError: At least 1 pipe is required to create a network.\n";
        cout << "Would you like to create a pipe now? (1-Yes/0-No): ";
        int choice;
        while (true) {
            cin >> choice;
            if (choice == 1 || choice == 0) break;
            cout << "Invalid choice. Please enter 1 for Yes or 0 for No: ";
        }
        if (choice == 1) {
            addPipe(pipes, nextPipeId);
        }
        return;
    }
    
    int choice;
    do {
        cout << "\n=== Network Management ===\n";
        cout << "1. Connect KCs with pipe\n";
        cout << "2. Display network\n";
        cout << "3. Topological sort\n";
        cout << "0. Back\n";
        cout << "Your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                int from_kc, to_kc, pipe_id;
                cout << "Enter source KC ID: ";
                while (!(cin >> from_kc) || companies.find(from_kc) == companies.end()) {
                    cout << "Invalid KC ID. Available KCs are:\n";
                    for (const auto& kc : companies) {
                        cout << "ID " << kc.first << ": " << kc.second.getName() << "\n";
                    }
                    cout << "Enter source KC ID: ";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }

                cout << "Enter destination KC ID: ";
                while (!(cin >> to_kc) || companies.find(to_kc) == companies.end() || from_kc == to_kc) {
                    if (from_kc == to_kc) {
                        cout << "Source and destination cannot be the same!\n";
                    } else {
                        cout << "Invalid KC ID. Available KCs are:\n";
                        for (const auto& kc : companies) {
                            cout << "ID " << kc.first << ": " << kc.second.getName() << "\n";
                        }
                    }
                    cout << "Enter destination KC ID: ";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }

                // Afficher les pipes disponibles
                cout << "Available pipes:\n";
                bool hasPipes = false;
                for (const auto& pair : pipes) {
                    if (!network.isPipeUsed(pair.first)) {
                        cout << "ID " << pair.first << ": " << pair.second.getName() 
                             << " (diameter: " << pair.second.getDiameter() << ")\n";
                        hasPipes = true;
                    }
                }

                if (!hasPipes) {
                    cout << "No available pipes. Create a new pipe now? (1-Yes/0-No): ";
                    int createChoice;
                    while (true) {
                        if (cin >> createChoice && (createChoice == 1 || createChoice == 0)) break;
                        cout << "Invalid choice. Enter 1 for Yes or 0 for No: ";
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                    if (createChoice == 1) {
                        addPipe(pipes, nextPipeId);
                        int continueChoice;
                        cout << "Do you want to continue creating the connection now? (1-Yes/0-No): ";
                        while (true) {
                            if (cin >> continueChoice && (continueChoice == 1 || continueChoice == 0)) break;
                            cout << "Invalid choice. Enter 1 for Yes or 0 for No: ";
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        }
                        if (continueChoice == 0) {
                            cout << "Returning to Network Management.\n";
                            break;
                        }
                        // sinon, on poursuit vers la sélection par diamètre juste après
                    } else {
                        cout << "No pipe selected. Returning to Network Management.\n";
                        break;
                    }
                }

                // Sélection en une seule étape par diamètre (500/700/1000/1400) puis choix auto
                int desired_diameter;
                cout << "Enter the diameter of the pipe to use (500/700/1000/1400): ";
                while (true) {
                    if (cin >> desired_diameter && (desired_diameter == 500 || desired_diameter == 700 || desired_diameter == 1000 || desired_diameter == 1400)) {
                        int chosen_id = -1;
                        for (const auto& pair : pipes) {
                            if (!network.isPipeUsed(pair.first) && pair.second.getDiameter() == desired_diameter) {
                                if (chosen_id == -1 || pair.first < chosen_id) {
                                    chosen_id = pair.first; // choisir le plus petit ID disponible pour stabilité
                                }
                            }
                        }
                        if (chosen_id != -1) {
                            pipe_id = chosen_id;
                            break;
                        }
                        cout << "No available pipes with this diameter. Try another among available: ";
                        bool first = true;
                        for (const auto& pair : pipes) {
                            if (!network.isPipeUsed(pair.first)) {
                                if (!first) cout << ", ";
                                cout << pair.second.getDiameter();
                                first = false;
                            }
                        }
                        cout << "\nEnter the diameter of the pipe to use (500/700/1000/1400): ";
                    } else {
                        cout << "Invalid input. Enter one of 500, 700, 1000, 1400: ";
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                }

                if (network.connectionExists(from_kc, to_kc)) {
                    cout << "Error: Connection already exists between these KCs!\n";
                    break;
                }

                if (network.addConnection(from_kc, to_kc, pipe_id)) {
                    cout << "Connection created successfully!\n";
                    logAction("Added connection: KC" + to_string(from_kc) + " -> KC" + to_string(to_kc) + " using pipe " + to_string(pipe_id));
                } else {
                    cout << "Failed to create connection (would create cycle)!\n";
                }
                break;
            }
            case 2:
                if (network.isEmpty()) {
                    cout << "Network is empty. Create some connections first using option 1.\n";
                } else {
                    network.displayConnections();
                }
                break;
            case 3: {
                if (network.isEmpty()) {
                    cout << "Cannot perform topological sort: network is empty!\n";
                    cout << "Create some connections first using option 1.\n";
                } else if (network.hasCycle()) {
                    cout << "Network contains cycles! Topological sort is impossible.\n";
                } else {
                    vector<int> sorted = network.topologicalSort(companies);
                    if (!sorted.empty()) {
                        cout << "\nTopological sort result:\n";
                        for (int id : sorted) {
                            cout << "KC " << id << " (" << companies[id].getName() << ")\n";
                        }
                    }
                }
                break;
            }
            case 0:
                break;
            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}

void saveToFile(const unordered_map<int, Pipe>& pipes, const unordered_map<int, KC>& companies, const string& filename) {
    if (pipes.empty() && companies.empty()) {
        cout << "Nothing to save: no pipes or companies in memory." << endl;
        logAction("Save aborted: nothing to save.");
        return;
    }
    std::ofstream file(filename);
    if (!file.is_open()) {
        cout << "Unable to open file for saving." << endl;
        logAction("Failed to open file for saving: " + filename);
        return;
    }
    file << pipes.size() << std::endl;
    for (unordered_map<int, Pipe>::const_iterator it = pipes.begin(); it != pipes.end(); ++it) {
        file << it->second << std::endl;
    }
    file << companies.size() << std::endl;
    for (unordered_map<int, KC>::const_iterator it = companies.begin(); it != companies.end(); ++it) {
        file << it->second << std::endl;
    }
    if (!file) {
        cout << "Error occurred during saving." << endl;
        logAction("Error occurred during saving: " + filename);
    } else {
        cout << "Data saved to file successfully." << endl;
        logAction("Saved data to file: " + filename);
    }
}

bool loadFromFile(unordered_map<int, Pipe>& pipes, unordered_map<int, KC>& companies, string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        cout << "Unable to open file for loading." << endl;
        logAction("Failed to open file for loading: " + filename);
        return false;
    }
    size_t nPipes = 0, nCompanies = 0;
    if (!(file >> nPipes)) {
        cout << "File corrupted or invalid format (pipes count)." << endl;
        logAction("Corrupted file (pipes count): " + filename);
        return false;
    }
    if (nPipes == 0 && nCompanies == 0) {
        cout << "No pipes and companies found in file." << endl;
    }
    pipes.clear();
    for (size_t i = 0; i < nPipes; ++i) {
        Pipe p;
        if (!(file >> p)) {
            cout << "File corrupted or invalid format (pipe data)." << endl;
            logAction("Corrupted file (pipe data): " + filename);
            return false;
        }
        int id = p.getId();
        if (id <= 0 || pipes.count(id)) {
            cout << "Invalid or duplicate pipe ID found: " << id << endl;
            logAction("Invalid/duplicate pipe ID: " + std::to_string(id));
            return false;
        }
        pipes[id] = p;
        // Log each loaded pipe
        Logger::logAction("LOAD PIPE", p.getId(), p.getName(), p.getLength(), p.getDiameter(), p.isRepair());
    }
    if (!(file >> nCompanies)) {
        cout << "File corrupted or invalid format (companies count)." << endl;
        logAction("Corrupted file (companies count): " + filename);
        return false;
    }
    // if (nCompanies == 0) {
    //     cout << "No companies found in file." << endl;
    // }
    companies.clear();
    for (size_t i = 0; i < nCompanies; ++i) {
        KC c;
        if (!(file >> c)) {
            cout << "File corrupted or invalid format (company data)." << endl;
            logAction("Corrupted file (company data): " + filename);
            return false;
        }
        int id = c.getId();
        if (id <= 0 || companies.count(id)) {
            cout << "Invalid or duplicate company ID found: " << id << endl;
            logAction("Invalid/duplicate company ID: " + std::to_string(id));
            return false;
        }
        companies[id] = c;
        // Log each loaded company
        Logger::logAction("LOAD COMPANY", c.getId(), c.getName(), c.getWorkshop(), c.getWorkshopInOperation(), c.getClasses());
    }
    if (pipes.empty() && companies.empty()) {
        cout << "Nothing loaded: file contains no pipes or companies." << endl;
        logAction("Load aborted: nothing in file.");
        return false;
    }
    
    cout << "Data loaded from file successfully." << endl;
    logAction("Loaded data from file: " + filename);
    return true;
    
    
}

// --- Batch Edit ---
void batchEditPipes(unordered_map<int, Pipe>& pipes, const vector<int>& ids) {
    if (ids.empty()) {
        cout << "No pipes to edit.\n";
        return;
    }
    cout << "Edit all found pipes (1) or select IDs (2)? ";
    int mode;
    cin >> mode;
    vector<int> toEdit = ids;
    if (mode == 2) {
        toEdit.clear();
        cout << "Enter IDs to edit (end with -1): ";
        int id;
        while (cin >> id && id != -1) toEdit.push_back(id);
    }
    for (size_t i = 0; i < toEdit.size(); ++i) {
        int id = toEdit[i];
        if (pipes.count(id)) {
            pipes[id].editRepair();
            Logger::logAction("EDIT PIPE", id, pipes[id].getName(), pipes[id].getLength(), pipes[id].getDiameter(), pipes[id].isRepair());
        }
    }
    cout << "Batch edit complete.\n";
}

// Fonctions génériques pour affichage, édition, suppression
template<typename T>
void displayAll(const unordered_map<int, T>& objects) {
    if (objects.empty()) {
        cout << "No objects available.\n";
        return;
    }
    for (typename unordered_map<int, T>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
        it->second.display();
    }
}

template<typename T>
void editById(unordered_map<int, T>& objects, void (T::*editFunc)()) {
    if (objects.empty()) {
        cout << "No objects to edit.\n";
        return;
    }
    int id;
    cout << "Enter ID to edit: ";
    cin >> id;
    if (objects.count(id)) {
        (objects[id].*editFunc)();
        cout << "Object updated.\n";
    } else {
        cout << "Object with this ID not found.\n";
    }
}

template<typename T>
void deleteById(unordered_map<int, T>& objects) {
    if (objects.empty()) {
        cout << "No objects to delete.\n";
        return;
    }
    int id;
    cout << "Enter ID to delete: ";
    cin >> id;
    if (objects.erase(id)) {
        cout << "Object deleted.\n";
    } else {
        cout << "Object with this ID not found.\n";
    }
}

// Recherche et actions sur résultats
template<typename T>
vector<int> searchObjects(const unordered_map<int, T>& objects, function<bool(const T&)> filter) {
    vector<int> foundIds;
    for (typename unordered_map<int, T>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
        if (filter(it->second)) {
            it->second.display();
            foundIds.push_back(it->first);
        }
    }
    if (foundIds.empty()) cout << "No objects found.\n";
    return foundIds;
}

// Sous-menu pour gérer les pipes
void managePipes(unordered_map<int, Pipe>& pipes) {
    int subchoice;
    vector<int> lastSearch;
    do {
        cout << "\n--- Pipe Management ---\n";
        cout << "1. Search pipes\n";
        cout << "2. Display all pipes\n";
        cout << "0. Back\n";
        cout << "Your choice: ";
        cin >> subchoice;
        if (cin.fail()) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n"; continue;
        }
        switch (subchoice) {
            case 1: {
                cout << "Filter by:\n1. Name\n2. Repair status\nYour choice: ";
                int filter; cin >> filter;
                if (filter == 1) {
                    string name;
                    cout << "Enter name: ";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    getline(cin, name);
                    lastSearch = searchObjects<Pipe>(pipes, [name](const Pipe& p){ return p.getName() == name; });
                } else if (filter == 2) {
                    int repair;
                    cout << "Repair status (0/1): "; cin >> repair;
                    lastSearch = searchObjects<Pipe>(pipes, [repair](const Pipe& p){ return p.isRepair() == (repair != 0); });
                } else {
                    cout << "Invalid filter.\n";
                    break;
                }
                if (!lastSearch.empty()) {
                    int act;
                    do {
                        cout << "\nActions on found pipes:\n";
                        cout << "1. Display\n2. Edit\n3. Delete\n4. Batch edit\n0. Back\nYour choice: ";
                        cin >> act;
                        if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); continue; }
                        switch (act) {
                            case 1: {
                                size_t shown = 0;
                                for (size_t i = 0; i < lastSearch.size(); ++i) {
                                    int idShow = lastSearch[i];
                                    unordered_map<int, Pipe>::const_iterator itP = pipes.find(idShow);
                                    if (itP != pipes.end()) {
                                        itP->second.display();
                                        shown++;
                                    }
                                }
                                if (shown == 0) cout << "No objects found.\n";
                                break;
                            }
                            case 2: {
                                int id;
                                cout << "Enter ID to edit: "; cin >> id;
                                if (pipes.count(id)) {
                                    pipes[id].editRepair();
                                    Logger::logAction("EDIT PIPE", id, pipes[id].getName(), pipes[id].getLength(), pipes[id].getDiameter(), pipes[id].isRepair());
                                }
                                else cout << "Not found.\n";
                                break;
                            }
                            case 3: {
                                int id;
                                cout << "Enter ID to delete: "; cin >> id;
                                if (pipes.count(id)) { Logger::logAction("DELETE PIPE", id, pipes[id].getName(), pipes[id].getLength(), pipes[id].getDiameter(), pipes[id].isRepair()); pipes.erase(id); cout << "Deleted.\n"; }
                                else cout << "Not found.\n";
                                break;
                            }
                            case 4:
                                batchEditPipes(pipes, lastSearch);
                                break;
                            case 0: break;
                            default: cout << "Invalid.\n";
                        }
                    } while (act != 0);
                }
                break;
            }
            case 2:
                displayAll(pipes);
                break;
            case 0: break;
            default: cout << "Invalid.\n";
        }
    } while (subchoice != 0);
}

// Sous-menu pour gérer les companies
void manageCompanies(unordered_map<int, KC>& companies) {
    int subchoice;
    vector<int> lastSearch;
    do {
        cout << "\n--- Company Management ---\n";
        cout << "1. Search companies\n";
        cout << "2. Display all companies\n";
        cout << "0. Back\n";
        cout << "Your choice: ";
        cin >> subchoice;
        if (cin.fail()) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n"; continue;
        }
        switch (subchoice) {
            case 1: {
                cout << "Filter by:\n1. Name\n2. % unused workshops\nYour choice: ";
                int filter; cin >> filter;
                if (filter == 1) {
                    string name;
                    cout << "Enter name: ";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    getline(cin, name);
                    lastSearch = searchObjects<KC>(companies, [name](const KC& c){ return c.getName() == name; });
                } else if (filter == 2) {
                    double percent;
                    cout << "Minimum % unused: "; cin >> percent;
                    lastSearch = searchObjects<KC>(companies, [percent](const KC& c){
                        if (c.getWorkshop() == 0) return false;
                        double unused = 100.0 * (c.getWorkshop() - c.getWorkshopInOperation()) / c.getWorkshop();
                        return unused >= percent;
                    });
                } else {
                    cout << "Invalid filter.\n";
                    break;
                }
                if (!lastSearch.empty()) {
                    int act;
                    do {
                        cout << "\nActions on found companies:\n";
                        cout << "1. Display\n2. Edit\n3. Delete\n0. Back\nYour choice: ";
                        cin >> act;
                        if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); continue; }
                        switch (act) {
                            case 1: {
                                size_t shown = 0;
                                for (size_t i = 0; i < lastSearch.size(); ++i) {
                                    int idShow = lastSearch[i];
                                    unordered_map<int, KC>::const_iterator itC = companies.find(idShow);
                                    if (itC != companies.end()) {
                                        itC->second.display();
                                        shown++;
                                    }
                                }
                                if (shown == 0) cout << "No objects found.\n";
                                break;
                            }
                            case 2: {
                                int id;
                                cout << "Enter ID to edit: "; cin >> id;
                                if (companies.count(id)) { companies[id].editWorkshops();
                                    Logger::logAction("EDIT COMPANY", id, companies[id].getName(), companies[id].getWorkshop(), companies[id].getWorkshopInOperation(), companies[id].getClasses()); }
                                else cout << "Not found.\n";
                                break;
                            }
                            case 3: {
                                int id;
                                cout << "Enter ID to delete: "; cin >> id;
                                if (companies.count(id)) { Logger::logAction("DELETE COMPANY", id, companies[id].getName(), companies[id].getWorkshop(), companies[id].getWorkshopInOperation(), companies[id].getClasses()); companies.erase(id); cout << "Deleted.\n"; }
                                else cout << "Not found.\n";
                                break;
                            }
                            case 0: break;
                            default: cout << "Invalid.\n";
                        }
                    } while (act != 0);
                }
                break;
            }
            case 2:
                displayAll(companies);
                break;
            case 0: break;
            default: cout << "Invalid.\n";
        }
    } while (subchoice != 0);
}

// --- Main ---
int main() {
    unordered_map<int, Pipe> pipes;
    unordered_map<int, KC> companies;
    int nextPipeId = 1, nextCompanyId = 1, choice;
    string filename = "data.txt";

    do {
        cout << "\n==== Main Menu ====\n";
        cout << "1. Add a pipe\n";
        cout << "2. Add a company\n";
        cout << "3. Manage pipes\n";
        cout << "4. Manage companies\n";
        cout << "5. Save to file\n";
        cout << "6. Load from file\n";
        cout << "7. Change file name\n";
        cout << "8. Manage Network\n";
        cout << "9. Delete pipe\n";
        cout << "10. Delete company\n";
        cout << "0. Quit\n";
        cout << "Your choice: ";
        cin >> choice;
        if (cin.fail()) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n"; continue;
        }
        switch (choice) {
            case 1:
                addPipe(pipes, nextPipeId);
                logAction("Added pipe");
                break;
            case 2:
                addCompany(companies, nextCompanyId);
                logAction("Added company");
                break;
            case 3:
                managePipes(pipes);
                break;
            case 4:
                manageCompanies(companies);
                break;
            case 5:
                saveToFile(pipes, companies, filename);
                break;
            case 6:
                loadFromFile(pipes, companies, filename);
                // Mise à jour des IDs après chargement
                if (!pipes.empty()) {
                    int maxId = 0;
                    for (unordered_map<int, Pipe>::const_iterator it = pipes.begin(); it != pipes.end(); ++it)
                        if (it->first > maxId) maxId = it->first;
                    nextPipeId = maxId + 1;
                } else nextPipeId = 1;
                if (!companies.empty()) {
                    int maxId = 0;
                    for (unordered_map<int, KC>::const_iterator it = companies.begin(); it != companies.end(); ++it)
                        if (it->first > maxId) maxId = it->first;
                    nextCompanyId = maxId + 1;
                } else nextCompanyId = 1;
                break;
            case 7:
                cout << "Enter filename: ";
                cin >> filename;
                logAction("Set filename: " + filename);
                break;
            case 8:
                manageNetwork(pipes, companies, nextPipeId, nextCompanyId);
                break;
            case 9:
                deletePipe(pipes, network);
                break;
            case 10:
                deleteKC(companies, network);
                break;
            case 0:
                cout << "Goodbye!" << endl;
                logAction("Exited program");
                break;
            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}

void deletePipe(std::unordered_map<int, Pipe>& pipes, GasNetwork& network) {
    if (pipes.empty()) {
        std::cout << "No pipes to delete.\n";
        return;
    }

    std::cout << "Available pipes:\n";
    for (const auto& pair : pipes) {
        std::cout << "ID " << pair.first << ": " << pair.second.getName();
        if (network.isPipeInNetwork(pair.first)) {
            std::cout << " (Used in network)";
        }
        std::cout << "\n";
    }

    int id;
    std::cout << "Enter pipe ID to delete: ";
    std::cin >> id;

    if (pipes.find(id) == pipes.end()) {
        std::cout << "Pipe not found.\n";
        return;
    }

    if (network.isPipeInNetwork(id)) {
        char choice;
        std::cout << "This pipe is being used in the network.\n";
        std::cout << "Do you want to remove its connection first? (y/n): ";
        std::cin >> choice;
        
        if (choice == 'y' || choice == 'Y') {
            network.removeConnectionByPipe(id);
            std::cout << "Connection removed.\n";
        } else {
            std::cout << "Deletion cancelled.\n";
            return;
        }
    }

    pipes.erase(id);
    std::cout << "Pipe deleted successfully.\n";
    Logger::logAction("Deleted pipe " + std::to_string(id));
}

void deleteKC(std::unordered_map<int, KC>& companies, GasNetwork& network) {
    if (companies.empty()) {
        std::cout << "No companies to delete.\n";
        return;
    }

    std::cout << "Available companies:\n";
    for (const auto& pair : companies) {
        std::cout << "ID " << pair.first << ": " << pair.second.getName() << "\n";
    }

    int id;
    std::cout << "Enter company ID to delete: ";
    std::cin >> id;

    if (companies.find(id) == companies.end()) {
        std::cout << "Company not found.\n";
        return;
    }

    if (!network.canDeleteKC(id)) {
        std::cout << "Cannot delete company: it has active connections in the network.\n";
        std::cout << "Remove all connections with this company first.\n";
        return;
    }

    companies.erase(id);
    std::cout << "Company deleted successfully.\n";
    Logger::logAction("Deleted company " + std::to_string(id));
}
