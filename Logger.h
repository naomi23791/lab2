#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>

class Logger {
private:
    static int pipeActionCounter;
    static int companyActionCounter;

public:
    // -----------------------------------------------------------------
    //  Log simple (texte libre)
    // -----------------------------------------------------------------
    static void logAction(const std::string& action) {
        std::ofstream logfile("user_actions.log", std::ios::app);
        if (logfile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto t   = std::chrono::system_clock::to_time_t(now);
            logfile << "[" << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S")
                    << "] " << action << std::endl;
        }
    }

    // -----------------------------------------------------------------
    //  PIPE – log « simple » (format demandé)
    // -----------------------------------------------------------------
    static void logPipeSimple(const std::string& name, double length,
                              double diameter, bool underRepair) {
        std::ofstream logfile("user_actions.log", std::ios::app);
        if (logfile.is_open()) {
            logfile << "action" << pipeActionCounter << " for the pipes" << std::endl;
            logfile << name << std::endl;
            logfile << length << std::endl;
            logfile << diameter << std::endl;
            logfile << (underRepair ? "yes" : "no") << std::endl;
            ++pipeActionCounter;
        }
    }

    // -----------------------------------------------------------------
    //  PIPE – overload avec libellé d’action
    // -----------------------------------------------------------------
    static void logAction(const std::string& actionLabel, int id,
                          const std::string& name, double length,
                          double diameter, bool underRepair) {
        (void)id;   // le format de log ne prévoit pas l’ID → on le « silence »
        std::ofstream logfile("user_actions.log", std::ios::app);
        if (logfile.is_open()) {
            logfile << "action" << pipeActionCounter << " for the pipe" << std::endl;
            logfile << name << std::endl;
            logfile << length << std::endl;
            logfile << diameter << std::endl;
            logfile << (underRepair ? "yes" : "no") << std::endl;
            logfile << actionLabel << std::endl;
            ++pipeActionCounter;
        }
    }

    // -----------------------------------------------------------------
    //  COMPANY – log « simple » (format demandé)
    // -----------------------------------------------------------------
    static void logCompanySimple(const std::string& name, int workshops,
                                 int workshopsInOperation,
                                 const std::string& classes) {
        std::ofstream logfile("user_actions.log", std::ios::app);
        if (logfile.is_open()) {
            logfile << "action" << companyActionCounter << " for the companies" << std::endl;
            logfile << name << std::endl;
            logfile << workshops << std::endl;
            logfile << workshopsInOperation << std::endl;
            logfile << classes << std::endl;
            ++companyActionCounter;
        }
    }

    // -----------------------------------------------------------------
    //  COMPANY – overload avec libellé d’action
    // -----------------------------------------------------------------
    static void logAction(const std::string& actionLabel, int id,
                          const std::string& name, int workshops,
                          int workshopsInOperation, const std::string& classes) {
        (void)id;   // idem, l’ID n’est pas utilisé dans le format texte
        std::ofstream logfile("user_actions.log", std::ios::app);
        if (logfile.is_open()) {
            logfile << "action" << companyActionCounter << " for the companies" << std::endl;
            logfile << name << std::endl;
            logfile << workshops << std::endl;
            logfile << workshopsInOperation << std::endl;
            logfile << classes << std::endl;
            logfile << actionLabel << std::endl;
            ++companyActionCounter;
        }
    }

    // -----------------------------------------------------------------
    //  Opération générique (START / COMPLETED)
    // -----------------------------------------------------------------
    template<typename Func>
    static void logOperation(const std::string& operationName, Func operation) {
        logAction("START: " + operationName);
        operation();
        logAction("COMPLETED: " + operationName);
    }
};
