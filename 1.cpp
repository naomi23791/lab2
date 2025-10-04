#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <vector>

using namespace std;
using std::vector;


// Modification: utilisation de std::vector pour gérer plusieurs objets et ajout d'identifiants uniques

class Pipe {
private:
    int id;
    string name;
    float length;
    int diameter;
    bool repair;
public:
    Pipe(int id_, const string& name_, float length_, int diameter_, bool repair_)
        : id(id_), name(name_), length(length_), diameter(diameter_), repair(repair_) {}
    Pipe() : id(0), name(""), length(0), diameter(0), repair(false) {}
    int getId() const { return id; }
    const string& getName() const { return name; }
    float getLength() const { return length; }
    int getDiameter() const { return diameter; }
    bool isRepair() const { return repair; }
    void setRepair(bool r) { repair = r; }
    void setId(int newId) { id = newId; }
    void input(int newId);
    void display() const;
    void editRepair();
    friend ostream& operator<<(ostream& os, const Pipe& p);
    friend istream& operator>>(istream& is, Pipe& p);
};

class KC {
private:
    int id;
    string name;
    int workshop;
    int workshop_in_operation;
    string classes;
public:
    KC(int id_, const string& name_, int workshop_, int workshop_in_operation_, const string& classes_)
        : id(id_), name(name_), workshop(workshop_), workshop_in_operation(workshop_in_operation_), classes(classes_) {}
    KC() : id(0), name(""), workshop(0), workshop_in_operation(0), classes("") {}
    int getId() const { return id; }
    const string& getName() const { return name; }
    int getWorkshop() const { return workshop; }
    int getWorkshopInOperation() const { return workshop_in_operation; }
    const string& getClasses() const { return classes; }
    void setId(int newId) { id = newId; }
    void input(int newId);
    void display() const;
    void editWorkshops();
    friend ostream& operator<<(ostream& os, const KC& c);
    friend istream& operator>>(istream& is, KC& c);
};

// Explication (français):
// On utilise maintenant des vecteurs pour stocker plusieurs objets (tuyaux et stations de compression),
// et chaque objet possède un identifiant unique (id) pour permettre une gestion et une identification faciles.

// Fonction de log (journalisation)
void logAction(const std::string& action) {
    std::ofstream log("user_actions.log", std::ios::app);
    if (log.is_open()) {
        log << action << std::endl;
    }
}

// Sauvegarde des objets dans un fichier
void saveToFile(const vector<Pipe>& pipes, const vector<KC>& companies, const string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        cout << "Unable to open file for saving." << endl;
        logAction("Failed to open file for saving: " + filename);
        return;
    }
    file << pipes.size() << std::endl;
    for (const auto& p : pipes) {
        file << p << std::endl;
    }
    file << companies.size() << std::endl;
    for (const auto& c : companies) {
        file << c << std::endl;
    }
    cout << "Data saved to file successfully." << endl;
    logAction("Saved data to file: " + filename);
}

// Chargement des objets depuis un fichier
bool loadFromFile(vector<Pipe>& pipes, vector<KC>& companies, string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        cout << "Unable to open file for loading." << endl;
        logAction("Failed to open file for loading: " + filename);
        return false;
    }
    pipes.clear();
    companies.clear();
    size_t nPipes, nCompanies;
    file >> nPipes;
    for (size_t i = 0; i < nPipes; ++i) {
        Pipe p;
        file >> p;
        pipes.push_back(p);
    }
    file >> nCompanies;
    for (size_t i = 0; i < nCompanies; ++i) {
        KC c;
        file >> c;
        companies.push_back(c);
    }
    cout << "Data loaded from file successfully." << endl;
    logAction("Loaded data from file: " + filename);
    return true;
}

// Explication (français):
// Ajout d'une fonction de log pour enregistrer toutes les actions de l'utilisateur dans un fichier séparé.
// Ajout de la possibilité de choisir le nom du fichier pour la sauvegarde et le chargement des données.
// Recherche de tuyaux par nom ou par statut de réparation
vector<int> searchPipes(const vector<Pipe>& pipes) {
    cout << "Filter pipes by:\n1. Name\n2. Repair status\nEnter choice: ";
    int filter;
    cin >> filter;
    vector<int> foundIds;
    if (filter == 1) {
        string name;
        cout << "Enter name to search: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, name);
        for (const auto& p : pipes) {
            if (p.getName() == name) {
                p.display();
                foundIds.push_back(p.getId());
            }
        }
    } else if (filter == 2) {
        int repair;
        cout << "Enter repair status (0 for no, 1 for yes): ";
        cin >> repair;
        for (const auto& p : pipes) {
            if (p.isRepair() == (repair != 0)) {
                p.display();
                foundIds.push_back(p.getId());
            }
        }
    }
    if (foundIds.empty()) cout << "No pipes found.\n";
    return foundIds;
}

// Recherche de stations de compression par nom ou pourcentage d'ateliers non utilisés
vector<int> searchCompanies(const vector<KC>& companies) {
    cout << "Filter companies by:\n1. Name\n2. % of unused workshops\nEnter choice: ";
    int filter;
    cin >> filter;
    vector<int> foundIds;
    if (filter == 1) {
        string name;
        cout << "Enter name to search: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, name);
        for (const auto& c : companies) {
            if (c.getName() == name) {
                c.display();
                foundIds.push_back(c.getId());
            }
        }
    } else if (filter == 2) {
        double percent;
        cout << "Enter minimum percent of unused workshops: ";
        cin >> percent;
        for (const auto& c : companies) {
            if (c.getWorkshop() > 0) {
                double unused = 100.0 * (c.getWorkshop() - c.getWorkshopInOperation()) / c.getWorkshop();
                if (unused >= percent) {
                    c.display();
                    foundIds.push_back(c.getId());
                }
            }
        }
    }
    if (foundIds.empty()) cout << "No companies found.\n";
    return foundIds;
}

// Edition par lot des tuyaux trouvés (par ID)
void batchEditPipes(vector<Pipe>& pipes, const vector<int>& ids) {
    if (ids.empty()) {
        cout << "No pipes to edit.\n";
        return;
    }
    cout << "Edit all found pipes (1) or select IDs (2)? ";
    int mode;
    cin >> mode;
    vector<int> toEdit = ids;
    if (mode == 2) {
        toEdit.clear();
        cout << "Enter IDs to edit (end with -1): ";
        int id;
        while (cin >> id && id != -1) toEdit.push_back(id);
    }
    for (auto& p : pipes) {
        for (int id : toEdit) {
            if (p.getId() == id) {
                p.editRepair();
            }
        }
    }
    cout << "Batch edit complete.\n";
}

// Explication (français):
// Ajout de fonctions pour filtrer (rechercher) les objets selon différents critères et pour éditer en lot les tuyaux trouvés.
// Modification d'un tuyau par ID
void editPipeById(vector<Pipe>& pipes) {
    if (pipes.empty()) {
        cout << "No pipes to edit.\n";
        return;
    }
    int id;
    cout << "Enter pipe ID to edit: ";
    cin >> id;
    for (auto& p : pipes) {
        if (p.getId() == id) {
            p.editRepair();
            cout << "Pipe updated.\n";
            return;
        }
    }
    cout << "Pipe with this ID not found.\n";
}

// Suppression d'un tuyau par ID
void deletePipeById(vector<Pipe>& pipes) {
    if (pipes.empty()) {
        cout << "No pipes to delete.\n";
        return;
    }
    int id;
    cout << "Enter pipe ID to delete: ";
    cin >> id;
    for (auto it = pipes.begin(); it != pipes.end(); ++it) {
        if (it->getId() == id) {
            pipes.erase(it);
            cout << "Pipe deleted.\n";
            return;
        }
    }
    cout << "Pipe with this ID not found.\n";
}

// Modification d'une station de compression par ID
void editCompanyById(vector<KC>& companies) {
    if (companies.empty()) {
        cout << "No companies to edit.\n";
        return;
    }
    int id;
    cout << "Enter company ID to edit: ";
    cin >> id;
    for (auto& c : companies) {
        if (c.getId() == id) {
            c.editWorkshops();
            cout << "Company updated.\n";
            return;
        }
    }
    cout << "Company with this ID not found.\n";
}

// Suppression d'une station de compression par ID
void deleteCompanyById(vector<KC>& companies) {
    if (companies.empty()) {
        cout << "No companies to delete.\n";
        return;
    }
    int id;
    cout << "Enter company ID to delete: ";
    cin >> id;
    for (auto it = companies.begin(); it != companies.end(); ++it) {
        if (it->getId() == id) {
            companies.erase(it);
            cout << "Company deleted.\n";
            return;
        }
    }
    cout << "Company with this ID not found.\n";
}

// Explication (français):
// Ajout de fonctions pour modifier ou supprimer un objet (tuyau ou station de compression) par son identifiant unique.
// Affichage de tous les tuyaux
void displayAllPipes(const vector<Pipe>& pipes) {
    if (pipes.empty()) {
        cout << "No pipes available.\n";
        return;
    }
    for (const auto& p : pipes) {
        p.display();
    }
}

// Affichage de toutes les stations de compression
void displayAllCompanies(const vector<KC>& companies) {
    if (companies.empty()) {
        cout << "No companies available.\n";
        return;
    }
    for (const auto& c : companies) {
        c.display();
    }
}

// Explication (français):
// Ajout de fonctions pour afficher tous les objets (tuyaux et stations de compression) présents dans les vecteurs.



// Nouvelle fonction pour ajouter un tuyau dans le vecteur

void addPipe(vector<Pipe>& pipes, int& nextPipeId) {
    Pipe p;
    p.input(nextPipeId++);
    pipes.push_back(p);
    cout << "Pipe added with ID: " << p.getId() << endl;
}


// Nouvelle fonction pour ajouter une station de compression dans le vecteur

void addCompany(vector<KC>& companies, int& nextCompanyId) {
    KC c;
    c.input(nextCompanyId++);
    companies.push_back(c);
    cout << "Company added with ID: " << c.getId() << endl;
}


// --- Pipe class methods ---
void Pipe::input(int newId) {
    id = newId;
    cout << "Enter the informations of pipe in this order:\nname, length, diameter, repair (0 for no, 1 for yes)\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Name: ";
    getline(cin, name);
    while (true) {
        cout << "Length: ";
        if (cin >> length && length >= 0) break;
        cout << "Invalid input. Please enter a number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    while (true) {
        cout << "Diameter: ";
        if (cin >> diameter && diameter > 0) break;
        cout << "Invalid input. Please enter an integer.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    int repairInput;
    while (true) {
        cout << "Repair (0 for no, 1 for yes): ";
        if (cin >> repairInput && (repairInput == 0 || repairInput == 1)) {
            repair = repairInput;
            break;
        }
        cout << "Invalid input. Please enter 0 or 1.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void Pipe::display() const {
    cout << "\nPipe ID: " << id << endl;
    cout << "name: " << name << endl;
    cout << "length: " << length << endl;
    cout << "diameter: " << diameter << endl;
    cout << "repair or not: " << (repair ? "yes" : "no") << endl;
}

void Pipe::editRepair() {
    int repairInput;
    while (true) {
        cout << "Set repair status (0 for no, 1 for yes): ";
        if (cin >> repairInput && (repairInput == 0 || repairInput == 1)) {
            repair = repairInput;
            break;
        }
        cout << "Invalid input. Please enter 0 or 1.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

ostream& operator<<(ostream& os, const Pipe& p) {
    os << p.id << "\t" << p.name << "\t" << p.length << "\t" << p.diameter << "\t" << p.repair;
    return os;
}

istream& operator>>(istream& is, Pipe& p) {
    is >> p.id;
    is.ignore();
    getline(is, p.name, '\t');
    is >> p.length;
    is.ignore();
    is >> p.diameter;
    is.ignore();
    is >> p.repair;
    is.ignore(numeric_limits<streamsize>::max(), '\n');
    return is;
}

// --- KC class methods ---
void KC::input(int newId) {
    id = newId;
    cout << "Enter the informations of the company in this order:\nname, number of workshops, number of workshops in operation, classes\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Name: ";
    getline(cin, name);
    while (true) {
        cout << "Number of workshops: ";
        if (cin >> workshop && workshop > 0) break;
        cout << "Invalid input. Please enter an integer.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    while (true) {
        cout << "Number of workshops in operation: ";
        if (cin >> workshop_in_operation && workshop_in_operation <= workshop) break;
        cout << "Invalid input. Please enter an integer.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Classes: ";
    getline(cin, classes);
}

void KC::display() const {
    cout << "\nCompany ID: " << id << endl;
    cout << "name: " << name << endl;
    cout << "workshop: " << workshop << endl;
    cout << "workshop_in_operation: " << workshop_in_operation << endl;
    cout << "classes: " << classes << endl;
}

void KC::editWorkshops() {
    int choice;
    cout << "Do you want to start (1) or stop (0) a workshop? ";
    cin >> choice;
    if (choice == 1) {
        if (workshop_in_operation < workshop) {
            workshop_in_operation++;
        } else {
            cout << "All workshops are already in operation." << endl;
        }
    } else if (choice == 0) {
        if (workshop_in_operation > 0) {
            workshop_in_operation--;
        } else {
            cout << "No workshops are in operation." << endl;
        }
    } else {
        cout << "Invalid choice." << endl;
    }
}

ostream& operator<<(ostream& os, const KC& c) {
    os << c.id << "\t" << c.name << "\t" << c.workshop << "\t" << c.workshop_in_operation << "\t" << c.classes;
    return os;
}

istream& operator>>(istream& is, KC& c) {
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


// Suppression de la fonction privée loadFromFile pour les objets individuels
int main() {
    vector<Pipe> pipes;
    vector<KC> companies;
    int nextPipeId = 1;
    int nextCompanyId = 1;
    int choice;
    string filename = "data.txt";
    vector<int> lastPipeSearch;
    vector<int> lastCompanySearch;

    do {
        cout << "\nMenu:\n";
        cout << "1. add a pipe\n";
        cout << "2. add a company\n";
        cout << "3. display all pipes\n";
        cout << "4. display all companies\n";
        cout << "5. edit a pipe by ID\n";
        cout << "6. delete a pipe by ID\n";
        cout << "7. edit a company by ID\n";
        cout << "8. delete a company by ID\n";
        cout << "9. search pipes\n";
        cout << "10. search companies\n";
        cout << "11. batch edit pipes\n";
        cout << "12. save to file\n";
        cout << "13. load from file\n";
        cout << "14. change file name\n";
        cout << "0. quit\n";
        cout << "Your choice: ";
        cin >> choice;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cout << "Invalid input. Please try again." << endl;
            continue;
        }
        switch (choice) {
                    case 1:
                        addPipe(pipes, nextPipeId);
                        logAction("Added pipe");
                        break;
                    case 2:
                        addCompany(companies, nextCompanyId);
                        logAction("Added company");
                        break;
                    case 3:
                        if (pipes.empty() || companies.empty()) {
                            cout << "You must add at least one pipe and one company before using this option." << endl;
                        } else {
                            displayAllPipes(pipes);
                            logAction("Displayed all pipes");
                        }
                        break;
                    case 4:
                        if (pipes.empty() || companies.empty()) {
                            cout << "You must add at least one pipe and one company before using this option." << endl;
                        } else {
                            displayAllCompanies(companies);
                            logAction("Displayed all companies");
                        }
                        break;
                    case 5:
                        if (pipes.empty() || companies.empty()) {
                            cout << "You must add at least one pipe and one company before using this option." << endl;
                        } else {
                            editPipeById(pipes);
                            logAction("Edited pipe by ID");
                        }
                        break;
                    case 6:
                        if (pipes.empty() || companies.empty()) {
                            cout << "You must add at least one pipe and one company before using this option." << endl;
                        } else {
                            deletePipeById(pipes);
                            logAction("Deleted pipe by ID");
                        }
                        break;
                    case 7:
                        if (pipes.empty() || companies.empty()) {
                            cout << "You must add at least one pipe and one company before using this option." << endl;
                        } else {
                            editCompanyById(companies);
                            logAction("Edited company by ID");
                        }
                        break;
                    case 8:
                        if (pipes.empty() || companies.empty()) {
                            cout << "You must add at least one pipe and one company before using this option." << endl;
                        } else {
                            deleteCompanyById(companies);
                            logAction("Deleted company by ID");
                        }
                        break;
                    case 9:
                        if (pipes.empty() || companies.empty()) {
                            cout << "You must add at least one pipe and one company before using this option." << endl;
                        } else {
                            lastPipeSearch = searchPipes(pipes);
                            logAction("Searched pipes");
                        }
                        break;
                    case 10:
                        if (pipes.empty() || companies.empty()) {
                            cout << "You must add at least one pipe and one company before using this option." << endl;
                        } else {
                            lastCompanySearch = searchCompanies(companies);
                            logAction("Searched companies");
                        }
                        break;
                    case 11:
                        if (pipes.empty() || companies.empty()) {
                            cout << "You must add at least one pipe and one company before using this option." << endl;
                        } else {
                            batchEditPipes(pipes, lastPipeSearch);
                            logAction("Batch edited pipes");
                        }
                        break;
                    case 12:
                        if (pipes.empty() || companies.empty()) {
                            cout << "You must add at least one pipe and one company before using this option." << endl;
                        } else {
                            saveToFile(pipes, companies, filename);
                        }
                        break;
                    case 13:
                        loadFromFile(pipes, companies, filename);
                        // После загрузки обновить nextPipeId и nextCompanyId
                        if (!pipes.empty()) {
                            nextPipeId = pipes.back().getId() + 1;
                        } else {
                            nextPipeId = 1;
                        }
                        if (!companies.empty()) {
                            nextCompanyId = companies.back().getId() + 1;
                        } else {
                            nextCompanyId = 1;
                        }
                        break;
                    case 14:
                        cout << "Enter filename: ";
                        cin >> filename;
                        logAction("Set filename: " + filename);
                        break;
                    case 0:
                        cout << "Goodbye!" << endl;
                        logAction("Exited program");
                        break;
                    default:
                        cout << "Invalid choice. Please try again." << endl;
                        break;
                }
            } while (choice != 0);

            return 0;
        }

// Explication (français):
// - Correction des doublons de variables et de la structure du main.
// - Correction de la gestion des IDs après chargement.
// - Ajout de la vérification d'entrée utilisateur.
// - Suppression des fonctions obsolètes et amélioration de la lisibilité.