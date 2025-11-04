#include "KC.h"
#include <limits>
#include <iostream>

KC::KC(int id_, const std::string& name_, int workshop_, int workshop_in_operation_, const std::string& classes_)
    : id(id_), name(name_), workshop(workshop_), workshop_in_operation(workshop_in_operation_), classes(classes_) {}

KC::KC() : id(0), name(""), workshop(0), workshop_in_operation(0), classes("") {}

int KC::getId() const { return id; }
const std::string& KC::getName() const { return name; }
int KC::getWorkshop() const { return workshop; }
int KC::getWorkshopInOperation() const { return workshop_in_operation; }
const std::string& KC::getClasses() const { return classes; }
void KC::setId(int newId) { id = newId; }

void KC::input(int newId) {
    id = newId;
    std::cout << "Enter the informations of the company in this order:\nname, number of workshops, number of workshops in operation, classes\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Name: ";
    getline(std::cin, name);
    while (true) {
        std::cout << "Number of workshops: ";
        if (std::cin >> workshop && workshop > 0) break;
        std::cout << "Invalid input. the Number of workshops must be a positive integer.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    while (true) {
        std::cout << "Number of workshops in operation: ";
        if (std::cin >> workshop_in_operation && workshop_in_operation <= workshop) break;
        std::cout << "Invalid input. Please enter an integer least than " << workshop << ".\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Classes: ";
    getline(std::cin, classes);
}

void KC::display() const {
    std::cout << "\nCompany ID: " << id << std::endl;
    std::cout << "name: " << name << std::endl;
    std::cout << "workshop: " << workshop << std::endl;
    std::cout << "workshop_in_operation: " << workshop_in_operation << std::endl;
    std::cout << "classes: " << classes << std::endl;
}

void KC::editWorkshops() {
    int choice;
    std::cout << "Do you want to start (1) or stop (0) a workshop? ";
    std::cin >> choice;
    if (choice == 1) {
        if (workshop_in_operation < workshop) {
            workshop_in_operation++;
        } else {
            std::cout << "All workshops are already in operation." << std::endl;
        }
    } else if (choice == 0) {
        if (workshop_in_operation > 0) {
            workshop_in_operation--;
        } else {
            std::cout << "No workshops are in operation." << std::endl;
        }
    } else {
        std::cout << "Invalid choice." << std::endl;
    }
}

std::ostream& operator<<(std::ostream& os, const KC& c) {
    os << c.id << "\t" << c.name << "\t" << c.workshop << "\t" << c.workshop_in_operation << "\t" << c.classes;
    return os;
}

std::istream& operator>>(std::istream& is, KC& c) {
    is >> c.id;
    is.ignore();
    getline(is, c.name, '\t');
    is >> c.workshop;
    is.ignore();
    is >> c.workshop_in_operation;
    is.ignore();
    getline(is, c.classes);
    return is;
}