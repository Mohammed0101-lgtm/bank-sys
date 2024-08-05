#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>

#include "trans.hpp"

int write_transaction(const std::string& filepath, transaction& trans) {
    std::ofstream file(filepath, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Failed to open filepath!" << std::endl;
        return -1;
    }
 
    file << trans.from   << ", "  << trans.to   << ", "       
         << trans.amount << ", "  << trans.time << std::endl;

    file.close();
    
    return 0;
}

transaction parse_line(const std::string& line) {
    transaction              trans;
    std::vector<std::string> tokens;
    std::stringstream        ss(line);
    std::string              token;

    while (getline(ss, token, ',')) {
        token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
        if (!token.empty()) 
            tokens.push_back(token);
    }

    if (tokens.size() != 5) 
        throw std::runtime_error("Invalid transaction format");

    trans.from = tokens[0];
    trans.to   = tokens[1];
    
    std::copy(tokens[2].begin(), tokens[2].end(), trans.location);
    trans.location[tokens[2].size()] = '\0';
    trans.time                       = tokens[3];
    trans.amount                     = std::stoi(tokens[4]);

    return trans;
}   

std::vector<transaction> load_transactions(const std::string& filepath) {
    std::vector<transaction> data;

    std::ifstream file(filepath, std::ios::in);
    if (!file.is_open()) 
        return data;

    std::string line;
    while (getline(file, line)) {
        try {
            transaction trans = parse_line(line);
            data.push_back(trans);
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse line: " << e.what() << std::endl;
        }
    }

    file.close();
    return data;
}

bool isValid_trans(const std::string& filepath, transaction trans) {
    std::vector<transaction> data = load_transactions(filepath);
    if (data.empty()) {
        return true;
    }

    // Add custom transaction validity conditions if needed

    return true;
}
