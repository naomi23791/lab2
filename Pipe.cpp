#include <iostream>
#include "Logger.h"
#include "Pipe.h"
#include "GasNetwork.h"
#include <functional>
#include <algorithm>
#include <limits>
#include <cmath>

// ---------------------------------------------------------------------
//  Helper : retourne le plus petit ID libre (évite les doublons)
// ---------------------------------------------------------------------
static int getNextId(const std::unordered_map<int, Pipe>& pipes) {
    int id = 1;
    while (pipes.count(id)) ++id;
    return id;
}

/* --------------------------------------------------------------------
   addPipe utilise le helper pour générer un ID sans trous
   -------------------------------------------------------------------- */
int addPipe(std::unordered_map<int, Pipe>& pipes) {
    int newId = getNextId(pipes);
    Pipe p;
    p.input(newId);
    pipes[newId] = p;
    std::cout << "Pipe added with ID: " << newId << std::endl;
    Logger::logPipeSimple(p.getName(), p.getLength(),
                          p.getDiameter(), p.isRepair());
    return newId;
}

/* --------------------------------------------------------------------
   Le reste du fichier est exactement votre version d’origine.
   -------------------------------------------------------------------- */

void batchEditPipes(std::unordered_map<int, Pipe>& pipes,
                    const std::vector<int>& ids, GasNetwork& network) {
    if (ids.empty()) {
        std::cout << "No pipes to edit.\n";
        return;
    }
    std::cout << "Edit all found pipes (1) or select IDs (2)? ";
    int mode; std::cin >> mode;
    std::vector<int> toEdit = ids;
    if (mode == 2) {
        toEdit.clear();
        std::cout << "Enter IDs to edit (end with -1): ";
        int id;
        while (std::cin >> id && id != -1) toEdit.push_back(id);
    }
    for (size_t i = 0; i < toEdit.size(); ++i) {
        int id = toEdit[i];
        if (pipes.count(id)) {
            pipes[id].editRepair();
            Logger::logAction("EDIT PIPE", id, pipes[id].getName(),
                             pipes[id].getLength(), pipes[id].getDiameter(),
                             pipes[id].isRepair());
            if (network.isPipeInNetwork(id))
                network.updatePipeInNetwork(id, pipes[id]);
        }
    }
    std::cout << "Batch edit complete.\n";
}

/* --------------------------------------------------------------------
   Templates displayAll / searchObjects (identiques à votre version)
   -------------------------------------------------------------------- */
template<typename T>
void displayAll(const std::unordered_map<int, T>& objects) {
    if (objects.empty()) {
        std::cout << "No objects available.\n";
        return;
    }
    std::vector<int> keys;
    for (const auto& kv : objects) keys.push_back(kv.first);
    std::sort(keys.begin(), keys.end());
    for (int id : keys) objects.at(id).display();
}

template<typename T>
std::vector<int> searchObjects(const std::unordered_map<int, T>& objects,
                               std::function<bool(const T&)> filter) {
    std::vector<int> found;
    for (const auto& kv : objects)
        if (filter(kv.second)) {
            kv.second.display();
            found.push_back(kv.first);
        }
    if (found.empty()) std::cout << "No objects found.\n";
    return found;
}

/* --------------------------------------------------------------------
   Menu de gestion des pipes (identique à votre version)
   -------------------------------------------------------------------- */
void managePipes(std::unordered_map<int, Pipe>& pipes, GasNetwork& network) {
    int subchoice;
    std::vector<int> lastSearch;
    do {
        std::cout << "\n--- Pipe Management ---\n";
        std::cout << "1. Search pipes\n";
        std::cout << "2. Display all pipes\n";
        std::cout << "0. Back\n";
        std::cout << "Your choice: ";
        std::cin >> subchoice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            continue;
        }
        switch (subchoice) {
            case 1: {
                if (pipes.empty()) {
                    std::cout << "No pipes exist in the system.\n";
                    std::cout << "Do you want to create a new pipe? (1-yes/0-no): ";
                    int createPipe; std::cin >> createPipe;
                    if (createPipe == 1) {
                        (void)addPipe(pipes);
                    }
                    break;
                }
                std::cout << "Filter by:\n1. Name\n2. Repair status\nYour choice: ";
                int filter; std::cin >> filter;
                if (filter == 1) {
                    std::string name;
                    std::cout << "Enter name: ";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::getline(std::cin, name);
                    lastSearch = searchObjects<Pipe>(pipes,
                        [name](const Pipe& p){ return p.getName() == name; });
                } else if (filter == 2) {
                    int rep; std::cout << "Repair status (0/1): "; std::cin >> rep;
                    lastSearch = searchObjects<Pipe>(pipes,
                        [rep](const Pipe& p){ return p.isRepair() == (rep != 0); });
                } else {
                    std::cout << "Invalid filter.\n";
                    break;
                }
                if (!lastSearch.empty()) {
                    int act;
                    do {
                        std::cout << "\nActions on found pipes:\n";
                        std::cout << "1. Display\n2. Edit\n3. Delete\n4. Batch edit\n0. Back\nYour choice: ";
                        std::cin >> act;
                        if (std::cin.fail()) {
                            std::cin.clear();
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                            continue;
                        }
                        switch (act) {
                            case 1: {
                                for (int id : lastSearch) {
                                    auto it = pipes.find(id);
                                    if (it != pipes.end()) it->second.display();
                                }
                                break;
                            }
                            case 2: {
                                int id; std::cout << "Enter ID to edit: "; std::cin >> id;
                                if (pipes.count(id)) {
                                    pipes[id].editRepair();
                                    Logger::logAction("EDIT PIPE", id, pipes[id].getName(),
                                                     pipes[id].getLength(),
                                                     pipes[id].getDiameter(),
                                                     pipes[id].isRepair());
                                    if (network.isPipeInNetwork(id))
                                        network.updatePipeInNetwork(id, pipes[id]);
                                } else std::cout << "Not found.\n";
                                break;
                            }
                            case 3: {
                                int id; std::cout << "Enter ID to delete: "; std::cin >> id;
                                if (pipes.count(id)) {
                                    if (network.isPipeInNetwork(id)) {
                                        std::cout << "Pipe is used in network. Remove connections first? (1-yes/0-no): ";
                                        int ch; std::cin >> ch;
                                        if (ch == 1) network.removeConnectionByPipe(id);
                                        else { std::cout << "Delete cancelled.\n"; break; }
                                    }
                                    Logger::logAction("DELETE PIPE", id, pipes[id].getName(),
                                                     pipes[id].getLength(),
                                                     pipes[id].getDiameter(),
                                                     pipes[id].isRepair());
                                    pipes.erase(id);
                                    std::cout << "Deleted.\n";
                                } else std::cout << "Not found.\n";
                                break;
                            }
                            case 4:
                                batchEditPipes(pipes, lastSearch, network);
                                break;
                            case 0: break;
                            default: std::cout << "Invalid.\n";
                        }
                    } while (act != 0);
                }
                break;
            }
            case 2:
                displayAll(pipes);
                break;
            case 0:
                break;
            default:
                std::cout << "Invalid.\n";
        }
    } while (subchoice != 0);
}

/* --------------------------------------------------------------------
   Implémentation de la classe Pipe (identique à votre version)
   -------------------------------------------------------------------- */
const int Pipe::CAPACITY_500  = 60;
const int Pipe::CAPACITY_700  = 130;
const int Pipe::CAPACITY_1000 = 300;
const int Pipe::CAPACITY_1400 = 650;

Pipe::Pipe(int id_, const std::string& name_, float length_,
           int diameter_, bool repair_)
    : id(id_), name(name_), length(length_), diameter(diameter_), repair(repair_) {}

Pipe::Pipe() : id(0), name(""), length(0.0f), diameter(0), repair(false) {}

int Pipe::getId() const { return id; }
const std::string& Pipe::getName() const { return name; }
float Pipe::getLength() const { return length; }
int Pipe::getDiameter() const { return diameter; }
bool Pipe::isRepair() const { return repair; }
void Pipe::setRepair(bool r) { repair = r; }
void Pipe::setId(int newId) { id = newId; }

int Pipe::getCapacity() const {
    if (repair) return 0;
    switch (diameter) {
        case 500:  return CAPACITY_500;
        case 700:  return CAPACITY_700;
        case 1000: return CAPACITY_1000;
        case 1400: return CAPACITY_1400;
        default:   return 0;
    }
}
float Pipe::getWeight() const {
    return repair ? std::numeric_limits<float>::infinity() : length;
}

/* Alternative formula – not used by default */
int Pipe::calculateCapacityByFormula(int diameter, float length) {
    if (length == 0) return 0;
    float d5 = std::pow(static_cast<float>(diameter), 5.0f);
    return static_cast<int>(std::sqrt(d5 / length) * 0.1f);
}

void Pipe::input(int newId) {
    id = newId;
    std::cout << "Enter the informations of pipe in this order:\n"
                 "name, length, diameter, repair (0 for no, 1 for yes)\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Name: "; std::getline(std::cin, name);
    while (true) {
        std::cout << "Length: ";
        if (std::cin >> length && length >= 0) break;
        std::cout << "Invalid input. Please enter a number.\n";
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    int d;
    while (true) {
        std::cout << "Diameter (500/700/1000/1400): ";
        if (std::cin >> d && (d==500||d==700||d==1000||d==1400)) { diameter = d; break; }
        std::cout << "Invalid input. Please enter 500, 700, 1000, or 1400.\n";
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    int r;
    while (true) {
        std::cout << "Repair (0 for no, 1 for yes): ";
        if (std::cin >> r && (r==0||r==1)) { repair = static_cast<bool>(r); break; }
        std::cout << "Invalid input. Please enter 0 or 1.\n";
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void Pipe::display() const {
    std::cout << "\nPipe ID: " << id
              << "\nName: " << name
              << "\nLength: " << length << " m"
              << "\nDiameter: " << diameter << " mm"
              << "\nRepair status: " << (repair ? "YES (under repair)" : "NO (operational)")
              << "\nCapacity: " << getCapacity() << " m³/h\n";
    if (repair)
        std::cout << "Weight (for pathfinding): INFINITY (pipe unusable)\n";
    else
        std::cout << "Weight (for pathfinding): " << getWeight() << " m\n";
}

void Pipe::editRepair() {
    int r;
    while (true) {
        std::cout << "Set repair status (0 for no, 1 for yes): ";
        if (std::cin >> r && (r==0||r==1)) { repair = static_cast<bool>(r); break; }
        std::cout << "Invalid input. Please enter 0 or 1.\n";
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

/* Operators */
std::ostream& operator<<(std::ostream& os, const Pipe& p) {
    os << p.id << '\t' << p.name << '\t' << p.length << '\t'
       << p.diameter << '\t' << p.repair;
    return os;
}
std::istream& operator>>(std::istream& is, Pipe& p) {
    is >> p.id; is.ignore();
    std::getline(is, p.name, '\t');
    is >> p.length; is.ignore();
    is >> p.diameter; is.ignore();
    is >> p.repair; is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return is;
}
