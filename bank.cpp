// banking implementation
#include "bank.hpp"
#include "acc.hpp"
#include <fstream>

#define MAX_AMOUNT 1000000

using json = nlohmann::json;

void banking::update(const char operation, const std::string& filepath) {
    json data;
    std::ifstream inFile(filepath, std::ios::in);
    
    if (!inFile.is_open()) 
        throw std::runtime_error("Error opening file");
    
    try
        inFile >> data;
    catch(const json::parse_error& e) {
        std::cerr << "Error parsing json file" << std::endl;
        inFile.close();
        return;
    }

    inFile.close();
    double balance = data["balance"];

    double amount;
    std::cout << "Enter the" << (operation == '+' ? "deposit" : "withdrawal") << " amount: ";
    std::cin >> amount;
    if (std::cin.fail())
        throw std::runtime_error("Failed to register input");

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (amount < 0 || amount > (operation == '+' ? MAX_AMOUNT : balance))
        throw std::out_of_range("input out of range");

    balance = balance + (operation == '+' ? amount : -amount);

    std::ofstream outFile(filepath, std::ios::out | std::ios::trunc);

    if (!outFile.is_open())
        throw std::runtime_error("Error opening file");
    
    outFile << data.dump(2);
    outFile.close();

    std::cout << (operation == '+' ? "Deposit" : "withdrawal") << "Successful!" << std::endl;
}

void banking::deposit(const std::string& filepath)  { update('+', filepath); } 
void banking::withdraw(const std::string& filepath) { update('-', filepath); }




