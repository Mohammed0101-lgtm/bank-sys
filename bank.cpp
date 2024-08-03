#include "bank.hpp"
#include "acc.hpp"
#include "trans.hpp"
#include "config.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <ctime>
#include <algorithm>
#include <sstream>


#define MAX_AMOUNT 1000000

std::string transDir = "transactions/";
std::string trans_file = "trans.csv";

std::string convert_to_filename(std::string& string) {
    string.erase(std::remove_if(string.begin(), string.end(), ::isspace), string.end());
    std::transform(string.begin(), string.end(), string.begin(), ::tolower);
    string = Account_dir + string + File_ext;
    return string;
}

void banking::update(const char operation, const std::string& filepath) {
    nlohmann::json data = get_jsonData(filepath);

    double balance = data.at("balance");

    double amount;
    std::cout << "Enter the" << (operation == '+' ? " deposit" : " withdrawal") << " amount: ";
    std::cin >> amount;
    if (std::cin.fail())
        throw std::runtime_error("Failed to register input");

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (amount < 0 || amount > (operation == '+' ? MAX_AMOUNT : balance))
        throw std::out_of_range("input out of range");

    balance = balance + static_cast<double>(operation == '+' ? amount : -amount);
    data.at("balance") = balance;

    std::ofstream outFile(filepath, std::ios::out | std::ios::trunc);

    if (!outFile.is_open())
        throw std::runtime_error("Error opening file");
    
    outFile << data.dump(2);
    outFile.close();

    std::cout << (operation == '+' ? "Deposit" : "withdrawal") << " Successful!" << std::endl;
}

void banking::deposit(const std::string& filepath) {
    update('+', filepath);
} 

void banking::withdraw(const std::string& filepath) {
    update('-', filepath);
}

std::string get_time() {
    time_t t = time(&t);
    char *date = ctime(&t);
    if (date == NULL) {
        std::cerr << "Failed to get transaction time" << std::endl;
        return "";
    }

    std::vector<std::string> tokens;
    std::vector<std::string> date_tokens(3); 
    std::istringstream ss(date);
    std::string token;

    while (ss >> token) {
        tokens.push_back(token);
    }

    date_tokens[0] = tokens[2];

    if      (tokens[1] == "Jan") { date_tokens[1] = "01"; } 
    else if (tokens[1] == "Feb") { date_tokens[1] = "02"; }  
    else if (tokens[1] == "Mar") { date_tokens[1] = "03"; } 
    else if (tokens[1] == "Apr") { date_tokens[1] = "04"; } 
    else if (tokens[1] == "May") { date_tokens[1] = "05"; } 
    else if (tokens[1] == "Jun") { date_tokens[1] = "06"; } 
    else if (tokens[1] == "Jul") { date_tokens[1] = "07"; } 
    else if (tokens[1] == "Aug") { date_tokens[1] = "08"; } 
    else if (tokens[1] == "Sep") { date_tokens[1] = "09"; } 
    else if (tokens[1] == "Oct") { date_tokens[1] = "10"; } 
    else if (tokens[1] == "Nov") { date_tokens[1] = "11"; } 
    else if (tokens[1] == "Dec") { date_tokens[1] = "12"; } 

    date_tokens[2] = tokens[4];

    std::string str = date_tokens[0] + "/" + date_tokens[1] + "/" + date_tokens[2];

    return str;
}
 
int make_transaction(const std::string& from, const std::string& to, unsigned int amount, const std::string& from_filepath, const std::string& to_filepath) {
    nlohmann::json data_1 = get_jsonData(from_filepath);
    nlohmann::json data_2 = get_jsonData(to_filepath);

    unsigned int from_balance, to_balance;
    from_balance = data_1["balance"];
    to_balance = data_2["balance"];

    if (from_balance < amount) {
        std::cerr << "Insufficient funds!" << std::endl;
        return -1;
    }

    from_balance -= amount;
    to_balance += amount;

    data_1.at("balance") = from_balance;
    data_2.at("balance") = to_balance;

    transaction trans;
    trans.amount = amount;
    trans.from = from;
    trans.to = to;
    trans.time = get_time();

    if (isValid_trans(transDir, trans) == false) {
        std::cerr << "Invalid transaction!" << std::endl;
        return -1;
    } 
    
    if (!std::filesystem::exists(transDir)) {
        std::filesystem::create_directory("transactions/");

        if (!std::filesystem::exists(transDir)) {
            throw std::runtime_error("Failed to create file!");
        }
    }

    std::string filepath = transDir + trans_file;
    if (write_transaction(filepath, trans) != 0) {
        std::cerr << "Failed to store transaction information!" << std::endl;
        return -1;
    }

    std::ofstream file_1(from_filepath, std::ios::out | std::ios::trunc);
    if (!file_1.is_open()) {
        std::cerr << "Failed to open file: " << from << std::endl;
        return -1;
    }

    file_1 << data_1.dump(2);
    file_1.close();

    std::ofstream file_2(to_filepath, std::ios::out | std::ios::trunc);
    if (!file_2.is_open()) {
        std::cerr << "Failed to open file: " << to << std::endl;
        return -1;
    }

    file_2 << data_2.dump(2);
    file_2.close();

    return 0;
}

void banking::make_trans(const std::string& filepath) {
    nlohmann::json data_1 = get_jsonData(filepath);

    std::string from = data_1.at("owner");
    std::string from_filepath = from;
    from_filepath = convert_to_filename(from_filepath);

    std::string to = get_string("Enter beneficiary");
    if (to == from) {
        std::cerr << "Option is unavailable!" << '\n';
        return;
    }
    std::string to_filepath = to;
    to_filepath = convert_to_filename(to_filepath);

    unsigned int amount;
    std::cout << "Enter amount: ";
    std::cin >> amount;

    make_transaction(from, to, amount, from_filepath, to_filepath);
}