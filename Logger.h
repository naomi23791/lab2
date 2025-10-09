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
    // Basic log action (kept for generic traces)
    static void logAction(const std::string& action) {
        std::ofstream logfile("user_actions.log", std::ios::app);
        if (logfile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            logfile << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] "
                   << action << std::endl;
        }
    }

    // Pipe log in the exact multi-line format requested
    static void logPipeSimple(const std::string& name, double length,
                              double diameter, bool underRepair) {
        std::ofstream logfile("user_actions.log", std::ios::app);
        if (logfile.is_open()) {
            logfile <<"action: "<< pipeActionCounter <<" for the pipes"<< std::endl;
            logfile << name << std::endl;
            logfile << length << std::endl;
            logfile << diameter << std::endl;
            logfile << (underRepair ? "yes" : "no") << std::endl;
            pipeActionCounter++;
        }
    }

    // Overload: logAction with Pipe object and action label (adds message at the end)
    static void logAction(const std::string& actionLabel, int id, const std::string& name,
                          double length, double diameter, bool underRepair) {
        std::ofstream logfile("user_actions.log", std::ios::app);
        if (logfile.is_open()) {
            logfile << "action: "<<pipeActionCounter <<" for the pipe"<< std::endl;
            logfile << name << std::endl;
            logfile << length << std::endl;
            logfile << diameter << std::endl;
            logfile << (underRepair ? "yes" : "no") << std::endl;
            logfile << actionLabel << std::endl;
            pipeActionCounter++;
        }
    }

    // Company log in the exact multi-line format requested
    static void logCompanySimple(const std::string& name, int workshops,
                                 int workshopsInOperation, const std::string& classes) {
        std::ofstream logfile("user_actions.log", std::ios::app);
        if (logfile.is_open()) {
            logfile <<"action: "<< companyActionCounter <<" for the companies"<< std::endl;
            logfile << name << std::endl;
            logfile << workshops << std::endl;
            logfile << workshopsInOperation << std::endl;
            logfile << classes << std::endl;
            companyActionCounter++;
        }
    }

    // Overload: logAction with Company object and action label (adds message at the end)
    static void logAction(const std::string& actionLabel, int id, const std::string& name,
                          int workshops, int workshopsInOperation, const std::string& classes) {
        std::ofstream logfile("user_actions.log", std::ios::app);
        if (logfile.is_open()) {
            logfile <<"action: "<< companyActionCounter <<" for the companies"<< std::endl;
            logfile << name << std::endl;
            logfile << workshops << std::endl;
            logfile << workshopsInOperation << std::endl;
            logfile << classes << std::endl;
            logfile << actionLabel << std::endl;
            companyActionCounter++;
        }
    }

    // Generic operation logger
    template<typename Func>
    static void logOperation(const std::string& operationName, Func operation) {
        logAction("START: " + operationName);
        operation();
        logAction("COMPLETED: " + operationName);
    }
};

// Initialize static counters
int Logger::pipeActionCounter = 1;
int Logger::companyActionCounter = 1;