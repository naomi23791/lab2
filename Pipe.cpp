#include "Pipe.h"
#include <limits>
#include <iostream>

Pipe::Pipe(int id_, const std::string& name_, float length_, int diameter_, bool repair_)
    : id(id_), name(name_), length(length_), diameter(diameter_), repair(repair_) {}

Pipe::Pipe() : id(0), name(""), length(0), diameter(0), repair(false) {}

int Pipe::getId() const { return id; }
const std::string& Pipe::getName() const { return name; }
float Pipe::getLength() const { return length; }
int Pipe::getDiameter() const { return diameter; }
bool Pipe::isRepair() const { return repair; }
void Pipe::setRepair(bool r) { repair = r; }
void Pipe::setId(int newId) { id = newId; }

void Pipe::input(int newId) {
    id = newId;
    std::cout << "Enter the informations of pipe in this order:\nname, length, diameter, repair (0 for no, 1 for yes)\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Name: ";
    getline(std::cin, name);
    while (true) {
        std::cout << "Length: ";
        if (std::cin >> length && length >= 0) break;
        std::cout << "Invalid input. Please enter a number.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    while (true) {
        std::cout << "Diameter: ";
        if (std::cin >> diameter && diameter > 0) break;
        std::cout << "Invalid input. Please enter an integer.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    int repairInput;
    while (true) {
        std::cout << "Repair (0 for no, 1 for yes): ";
        if (std::cin >> repairInput && (repairInput == 0 || repairInput == 1)) {
            repair = repairInput;
            break;
        }
        std::cout << "Invalid input. Please enter 0 or 1.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void Pipe::display() const {
    std::cout << "\nPipe ID: " << id << std::endl;
    std::cout << "name: " << name << std::endl;
    std::cout << "length: " << length << std::endl;
    std::cout << "diameter: " << diameter << std::endl;
    std::cout << "repair or not: " << (repair ? "yes" : "no") << std::endl;
}

void Pipe::editRepair() {
    int repairInput;
    while (true) {
        std::cout << "Set repair status (0 for no, 1 for yes): ";
        if (std::cin >> repairInput && (repairInput == 0 || repairInput == 1)) {
            repair = repairInput;
            break;
        }
        std::cout << "Invalid input. Please enter 0 or 1.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

std::ostream& operator<<(std::ostream& os, const Pipe& p) {
    os << p.id << "\t" << p.name << "\t" << p.length << "\t" << p.diameter << "\t" << p.repair;
    return os;
}

std::istream& operator>>(std::istream& is, Pipe& p) {
    is >> p.id;
    is.ignore();
    getline(is, p.name, '\t');
    is >> p.length;
    is.ignore();
    is >> p.diameter;
    is.ignore();
    is >> p.repair;
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return is;
}

// Implémentation des fonctions de gestion des pipes dans le namespace PipeManager
namespace PipeManager {
    
void addPipe(std::unordered_map<int, Pipe>& pipes, int& nextPipeId) {
    Pipe p;
    p.input(nextPipeId);
    pipes[nextPipeId] = p;
    std::cout << "Pipe added with ID: " << nextPipeId << std::endl;
    Logger::logPipeSimple(p.getName(), p.getLength(), p.getDiameter(), p.isRepair());
    nextPipeId++;
}

void batchEditPipes(std::unordered_map<int, Pipe>& pipes, const std::vector<int>& ids) {
    if (ids.empty()) {
        std::cout << "No pipes to edit.\n";
        return;
    }
    std::cout << "Edit all found pipes (1) or select IDs (2)? ";
    int mode;
    std::cin >> mode;
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
            Logger::logAction("EDIT PIPE", id, pipes[id].getName(), pipes[id].getLength(), pipes[id].getDiameter(), pipes[id].isRepair());
        }
    }
    std::cout << "Batch edit complete.\n";
}

void displayAll(const std::unordered_map<int, Pipe>& pipes) {
    if (pipes.empty()) {
        std::cout << "No objects available.\n";
        return;
    }
    for (std::unordered_map<int, Pipe>::const_iterator it = pipes.begin(); it != pipes.end(); ++it) {
        it->second.display();
    }
}

std::vector<int> searchObjects(const std::unordered_map<int, Pipe>& pipes, std::function<bool(const Pipe&)> filter) {
    std::vector<int> foundIds;
    for (std::unordered_map<int, Pipe>::const_iterator it = pipes.begin(); it != pipes.end(); ++it) {
        if (filter(it->second)) {
            it->second.display();
            foundIds.push_back(it->first);
        }
    }
    if (foundIds.empty()) std::cout << "No objects found.\n";
    return foundIds;
}

// void managePipes(std::unordered_map<int, Pipe>& pipes) {
//     int subchoice;
//     std::vector<int> lastSearch;
//     do {
//         std::cout << "\n--- Pipe Management ---\n";
//         std::cout << "1. Search pipes\n";
//         std::cout << "2. Display all pipes\n";
//         std::cout << "0. Back\n";
//         std::cout << "Your choice: ";
//         std::cin >> subchoice;
//         if (std::cin.fail()) {
//             std::cin.clear(); 
//             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//             std::cout << "Invalid input.\n"; 
//             continue;
//         }
//         switch (subchoice) {
//             case 1: {
//                 std::cout << "Filter by:\n1. Name\n2. Repair status\nYour choice: ";
//                 int filter; 
//                 std::cin >> filter;
//                 if (filter == 1) {
//                     std::string name;
//                     std::cout << "Enter name: ";
//                     std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//                     getline(std::cin, name);
//                     lastSearch = searchObjects(pipes, [name](const Pipe& p){ return p.getName() == name; });
//                 } else if (filter == 2) {
//                     int repair;
//                     std::cout << "Repair status (0/1): "; 
//                     std::cin >> repair;
//                     lastSearch = searchObjects(pipes, [repair](const Pipe& p){ return p.isRepair() == (repair != 0); });
//                 } else {
//                     std::cout << "Invalid filter.\n";
//                     break;
//                 }
//                 if (!lastSearch.empty()) {
//                     int act;
//                     do {
//                         std::cout << "\nActions on found pipes:\n";
//                         std::cout << "1. Display\n2. Edit\n3. Delete\n4. Batch edit\n0. Back\nYour choice: ";
//                         std::cin >> act;
//                         if (std::cin.fail()) { 
//                             std::cin.clear(); 
//                             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
//                             continue; 
//                         }
//                         switch (act) {
//                             case 1: {
//                                 size_t shown = 0;
//                                 for (size_t i = 0; i < lastSearch.size(); ++i) {
//                                     int idShow = lastSearch[i];
//                                     std::unordered_map<int, Pipe>::const_iterator itP = pipes.find(idShow);
//                                     if (itP != pipes.end()) {
//                                         itP->second.display();
//                                         shown++;
//                                     }
//                                 }
//                                 if (shown == 0) std::cout << "No objects found.\n";
//                                 break;
//                             }
//                             case 2: {
//                                 int id;
//                                 std::cout << "Enter ID to edit: "; 
//                                 std::cin >> id;
//                                 if (pipes.count(id)) {
//                                     pipes[id].editRepair();
//                                     Logger::logAction("EDIT PIPE", id, pipes[id].getName(), pipes[id].getLength(), pipes[id].getDiameter(), pipes[id].isRepair());
//                                 }
//                                 else std::cout << "Not found.\n";
//                                 break;
//                             }
//                             case 3: {
//                                 int id;
//                                 std::cout << "Enter ID to delete: "; 
//                                 std::cin >> id;
//                                 if (pipes.count(id)) { 
//                                     Logger::logAction("DELETE PIPE", id, pipes[id].getName(), pipes[id].getLength(), pipes[id].getDiameter(), pipes[id].isRepair()); 
//                                     pipes.erase(id); 
//                                     std::cout << "Deleted.\n"; 
//                                 }
//                                 else std::cout << "Not found.\n";
//                                 break;
//                             }
//                             case 4:
//                                 batchEditPipes(pipes, lastSearch);
//                                 break;
//                             case 0: break;
//                             default: std::cout << "Invalid.\n";
//                         }
//                     } while (act != 0);
//                 }
//                 break;
//             }
//             case 2:
//                 displayAll(pipes);
//                 break;
//             case 0: break;
//             default: std::cout << "Invalid.\n";
//         }
//     } while (subchoice != 0);
// }

// void deletePipe(std::unordered_map<int, Pipe>& pipes, GasNetwork& network) {
//     if (pipes.empty()) {
//         std::cout << "No pipes to delete.\n";
//         return;
//     }

//     std::cout << "Available pipes:\n";
//     for (const auto& pair : pipes) {
//         std::cout << "ID " << pair.first << ": " << pair.second.getName();
//         if (network.isPipeInNetwork(pair.first)) {
//             std::cout << " (Used in network)";
//         }
//         std::cout << "\n";
//     }

//     int id;
//     std::cout << "Enter pipe ID to delete: ";
//     std::cin >> id;

//     if (pipes.find(id) == pipes.end()) {
//         std::cout << "Pipe not found.\n";
//         return;
//     }

//     if (network.isPipeInNetwork(id)) {
//         char choice;
//         std::cout << "This pipe is being used in the network.\n";
//         std::cout << "Do you want to remove its connection first? (y/n): ";
//         std::cin >> choice;
        
//         if (choice == 'y' || choice == 'Y') {
//             network.removeConnectionByPipe(id);
//             std::cout << "Connection removed.\n";
//         } else {
//             std::cout << "Deletion cancelled.\n";
//             return;
//         }
//     }

//     pipes.erase(id);
//     std::cout << "Pipe deleted successfully.\n";
//     Logger::logAction("Deleted pipe " + std::to_string(id));
// }

// } // namespace PipeManager