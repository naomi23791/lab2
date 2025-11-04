#include "Pipe.h"
#include <limits>
#include <iostream>
#include <vector>
#include <algorithm>

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

const std::vector<int> Pipe::VALID_DIAMETERS = {500, 700, 1000, 1400};

bool Pipe::isValidDiameter(int diameter) {
    return std::find(VALID_DIAMETERS.begin(), VALID_DIAMETERS.end(), diameter) != VALID_DIAMETERS.end();
}
// ...existing code...

// Garder seulement une version de input(), celle avec la validation des diamètres :
void Pipe::input(int newId) {
    id = newId;
    std::cout << "Enter pipe name: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    getline(std::cin, name);
    
    while (true) {
        std::cout << "Length: ";
        if (std::cin >> length && length >= 0) break;
        std::cout << "Invalid input. Please enter a positive number.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    
    do {
        std::cout << "Enter pipe diameter (500/700/1000/1400): ";
        if (std::cin >> diameter && isValidDiameter(diameter)) break;
        std::cout << "Invalid diameter! Please use 500, 700, 1000, or 1400 mm.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } while (true);
    
    repair = false;
}

// ...existing code...

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