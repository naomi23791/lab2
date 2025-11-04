#pragma once
#include <unordered_map>
#include "Pipe.h"
#include "KC.h"
#include "GasNetwork.h"

// Functions to delete pipes and companies
void deletePipe(std::unordered_map<int, Pipe>& pipes, GasNetwork& network);
void deleteKC(std::unordered_map<int, KC>& companies, GasNetwork& network);