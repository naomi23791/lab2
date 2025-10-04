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