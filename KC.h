#pragma once
#include <string>
#include <iostream>

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