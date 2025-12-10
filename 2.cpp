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
#include "RedirectWrapper.h"  // ← AJOUTEZ CETTE LIGNE
#include "Logger.h"
using namespace std;

// --- Simple passthrough for legacy calls ---
static inline void logAction(const std::string& action) { Logger::logAction(action); }

// --- Save/Load --

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
                                if (pipes.count(id)) { Logger::logAction("DELETE PIPE"); pipes.erase(id); cout << "Deleted.\n"; }
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
                                if (companies.count(id)) { Logger::logAction("DELETE COMPANY"); companies.erase(id); cout << "Deleted.\n"; }
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
