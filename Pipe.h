#pragma once
#include <string>
#include <iostream>

class Pipe {
private:
    int id;
    std::string name;
    float length;
    int diameter;
    bool repair;
public:
    Pipe(int id_, const std::string& name_, float length_, int diameter_, bool repair_);
    Pipe();
    int getId() const;
    const std::string& getName() const;
    float getLength() const;
    int getDiameter() const;
    bool isRepair() const;
    void setRepair(bool r);
    void setId(int newId);
    void input(int newId);
    void display() const;
    void editRepair();
    friend std::ostream& operator<<(std::ostream& os, const Pipe& p);
    friend std::istream& operator>>(std::istream& is, Pipe& p);
};