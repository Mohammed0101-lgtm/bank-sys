#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "trans.hpp"

int write_transaction(const std::string& filepath, Transaction& trans) {
    std::ofstream file(filepath, std::ios::app);
    if (!file.is_open())
    {
        std::cerr << "Failed to open filepath!" << std::endl;
        return -1;
    }

    file << trans.from << ", " << trans.to << ", " << trans.amount << ", " << trans.time << std::endl;

    file.close();

    return 0;
}

Transaction parse_line(const std::string& line) {
    Transaction              trans;
    std::vector<std::string> tokens;
    std::stringstream        ss(line);
    std::string              token;

    while (getline(ss, token, ','))
    {
        token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
        if (!token.empty())
            tokens.push_back(token);
    }

    if (tokens.size() != 5)
        throw std::runtime_error("Invalid Transaction format");

    trans.from = tokens[0];
    trans.to   = tokens[1];

    std::copy(tokens[2].begin(), tokens[2].end(), trans.location);
    trans.location[tokens[2].size()] = '\0';
    trans.time                       = tokens[3];
    trans.amount                     = std::stoi(tokens[4]);

    return trans;
}

std::vector<Transaction> load_transactions(const std::string& filepath) {
    std::vector<Transaction> data;

    std::ifstream file(filepath, std::ios::in);
    if (!file.is_open())
        return data;

    std::string line;
    while (getline(file, line))
    {
        try
        {
            Transaction trans = parse_line(line);
            data.push_back(trans);
        } catch (const std::exception& e)
        { std::cerr << "Failed to parse line: " << e.what() << std::endl; }
    }

    file.close();
    return data;
}

bool isValid_trans(const std::string& filepath, Transaction trans) {
    std::vector<Transaction> data = load_transactions(filepath);
    if (data.empty())
        return true;

    // Add custom Transaction validity conditions if needed

    return true;
}
