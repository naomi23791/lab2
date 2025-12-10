// Pipe.h
#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <limits>

class GasNetwork;   // forward declaration (pas besoin d’inclure ici)

class Pipe {
private:
    int    id;
    std::string name;
    float  length;    // en mètres
    int    diameter;  // en millimètres (500, 700, 1000, 1400)
    bool   repair;   // true = en réparation (indisponible)

    // Capacités fixes (m³/h) tirées du tableau 1.1 du sujet
    static const int CAPACITY_500; //60m³/h
    static const int CAPACITY_700; //130m³/h
    static const int CAPACITY_1000; //300m³/h
    static const int CAPACITY_1400; //650m³/h

public:
    // ----- constructeurs -------------------------------------------------
    Pipe(int id_, const std::string& name_, float length_, int diameter_, bool repair_);
    Pipe();

    // ----- getters / setters ---------------------------------------------
    int          getId() const;
    const std::string& getName() const;
    float        getLength() const;
    int          getDiameter() const;
    bool         isRepair() const;
    void         setRepair(bool r);
    void         setId(int newId);

    // ----- I/O -----------------------------------------------------------
    void input(int newId);          // lecture interactive
    void display() const;           // affichage détaillé
    void editRepair();              // changer le flag « repair »

    // ----- calculs --------------------------------------------------------
    int  getCapacity() const;      // 0 si en réparation, sinon selon diamètre
    float getWeight() const;       // +∞ si en réparation, sinon = longueur

    // Méthode statique optionnelle (formule alternative)
    static int calculateCapacityByFormula(int diameter, float length);

    // ----- opérateurs ----------------------------------------------------
    friend std::ostream& operator<<(std::ostream& os, const Pipe& p);
    friend std::istream& operator>>(std::istream& is, Pipe& p);
};

// ---------------------------------------------------------------------
//  Fonctions utilitaires (déclarées ici, définies dans Pipe.cpp)
// ---------------------------------------------------------------------
int addPipe(std::unordered_map<int, Pipe>& pipes);
void managePipes(std::unordered_map<int, Pipe>& pipes, GasNetwork& network);
void batchEditPipes(std::unordered_map<int, Pipe>& pipes,
                   const std::vector<int>& ids, GasNetwork& network);

template<typename T>
void displayAll(const std::unordered_map<int, T>& objects);

template<typename T>
std::vector<int> searchObjects(const std::unordered_map<int, T>& objects,
                               std::function<bool(const T&)> filter);
