
#include "KC.h"
#include <unordered_map>
#include <vector>
#include <limits>
#include <string>
#include <algorithm>
#include "Logger.h"

int addCompany(std::unordered_map<int, KC>& companies) {
    // compute next id as max existing id + 1
    int nextCompanyId = 0;
    for (std::unordered_map<int, KC>::const_iterator it = companies.begin(); it != companies.end(); ++it)
        if (it->first > nextCompanyId) nextCompanyId = it->first;
    nextCompanyId++;

    KC c;
    c.input(nextCompanyId);
    companies[nextCompanyId] = c;
    std::cout << "Company added with ID: " << nextCompanyId << std::endl;
    Logger::logCompanySimple(c.getName(), c.getWorkshop(), c.getWorkshopInOperation(), c.getClasses());
    return nextCompanyId;
}

void manageCompanies(std::unordered_map<int, KC>& companies) {
    int subchoice;
    std::vector<int> lastSearch;
    do {
        std::cout << "\n--- Company Management ---\n";
        std::cout << "1. Search companies\n";
        std::cout << "2. Display all companies\n";
        std::cout << "0. Back\n";
        std::cout << "Your choice: ";
        std::cin >> subchoice;
        if (std::cin.fail()) {
            std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n"; continue;
        }
        switch (subchoice) {
            case 1: {
                if (companies.empty()) {
                    std::cout << "No companies exist in the system.\n";
                    std::cout << "Do you want to create a new company? (1-yes/0-no): ";
                    int createCompany;
                    std::cin >> createCompany;
                    if (createCompany == 1) {
                        int newId = addCompany(companies);
                        (void)newId;
                    }
                    break;
                }
                
                std::cout << "Filter by:\n1. Name\n2. % unused workshops\nYour choice: ";
                int filter; std::cin >> filter;
                if (filter == 1) {
                    std::string name;
                    std::cout << "Enter name: ";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    getline(std::cin, name);
                    lastSearch.clear();
                    for (std::unordered_map<int, KC>::const_iterator it = companies.begin(); it != companies.end(); ++it) {
                        if (it->second.getName() == name) {
                            it->second.display();
                            lastSearch.push_back(it->first);
                        }
                    }
                } else if (filter == 2) {
                    double percent;
                    std::cout << "Minimum % unused: "; std::cin >> percent;
                    lastSearch.clear();
                    for (std::unordered_map<int, KC>::const_iterator it = companies.begin(); it != companies.end(); ++it) {
                        if (it->second.getWorkshop() == 0) continue;
                        double unused = 100.0 * (it->second.getWorkshop() - it->second.getWorkshopInOperation()) / it->second.getWorkshop();
                        if (unused >= percent) {
                            it->second.display();
                            lastSearch.push_back(it->first);
                        }
                    }
                } else {
                    std::cout << "Invalid filter.\n";
                    break;
                }
                if (!lastSearch.empty()) {
                    int act;
                    do {
                        std::cout << "\nActions on found companies:\n";
                        std::cout << "1. Display\n2. Edit\n3. Delete\n0. Back\nYour choice: ";
                        std::cin >> act;
                        if (std::cin.fail()) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); continue; }
                        switch (act) {
                            case 1: {
                                size_t shown = 0;
                                for (size_t i = 0; i < lastSearch.size(); ++i) {
                                    int idShow = lastSearch[i];
                                    std::unordered_map<int, KC>::const_iterator itC = companies.find(idShow);
                                    if (itC != companies.end()) {
                                        itC->second.display();
                                        shown++;
                                    }
                                }
                                if (shown == 0) std::cout << "No objects found.\n";
                                break;
                            }
                            case 2: {
                                int id;
                                std::cout << "Enter ID to edit: "; std::cin >> id;
                                if (companies.count(id)) { companies[id].editWorkshops();
                                    Logger::logAction("EDIT COMPANY", id, companies[id].getName(), companies[id].getWorkshop(), companies[id].getWorkshopInOperation(), companies[id].getClasses()); }
                                else std::cout << "Not found.\n";
                                break;
                            }
                            case 3: {
                                int id;
                                std::cout << "Enter ID to delete: "; std::cin >> id;
                                if (companies.count(id)) { Logger::logAction("DELETE COMPANY", id, companies[id].getName(), companies[id].getWorkshop(), companies[id].getWorkshopInOperation(), companies[id].getClasses()); companies.erase(id); std::cout << "Deleted.\n"; }
                                else std::cout << "Not found.\n";
                                break;
                            }
                            case 0: break;
                            default: std::cout << "Invalid.\n";
                        }
                    } while (act != 0);
                }
                break;
            }
            case 2: {
                if (companies.empty()) {
                    std::cout << "No companies available.\n";
                } else {
                    std::vector<int> ids;
                    for (std::unordered_map<int, KC>::const_iterator it = companies.begin(); it != companies.end(); ++it) ids.push_back(it->first);
                    std::sort(ids.begin(), ids.end());
                    for (size_t i = 0; i < ids.size(); ++i) companies.at(ids[i]).display();
                }
                break;
            }
                break;
            case 0: break;
            default: std::cout << "Invalid.\n";
        }
    } while (subchoice != 0);
}
#include "KC.h"
#include "Logger.h"
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