// KC.h
#pragma once
#include <string>
#include <iostream>
#include <unordered_map>

class KC {
private:
    int id;
    std::string name;
    int workshop;
    int workshop_in_operation;
    std::string classes;
public:
    KC(int id_, const std::string& name_, int workshop_, int workshop_in_operation_, const std::string& classes_);
    KC();
    int getId() const;
    const std::string& getName() const;
    int getWorkshop() const;
    int getWorkshopInOperation() const;
    const std::string& getClasses() const;
    void setId(int newId);
    void input(int newId);
    void display() const;
    void editWorkshops();
    friend std::ostream& operator<<(std::ostream& os, const KC& c);
    friend std::istream& operator>>(std::istream& is, KC& c);
};

// Fonctions de gestion des KC
// Adds a company to the map and returns the assigned ID
int addCompany(std::unordered_map<int, KC>& companies);
void manageCompanies(std::unordered_map<int, KC>& companies);