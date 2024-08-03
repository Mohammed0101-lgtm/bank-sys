#ifndef TRANS_HPP
#define TRANS_HPP

#include <string>

struct transaction {
    char* location;
    std::string from;
    std::string to;
    std::string time;
    unsigned int amount;
};

typedef struct transaction transaction;

int write_transaction(const std::string& filepath, transaction& trans);
transaction parse_line(const std::string& line);
std::vector<transaction> load_transactions(const std::string& filepath);
bool isValid_trans(const std::string& filepath, transaction trans);

#endif // trans_hpp