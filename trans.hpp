#ifndef TRANS_HPP
#define TRANS_HPP

#include <string>

struct Transaction
{
    char*        location;
    std::string  from;
    std::string  to;
    std::string  time;
    unsigned int amount;
};

int                      write_transaction(const std::string& filepath, Transaction& trans);
Transaction              parse_line(const std::string& line);
std::vector<Transaction> load_transactions(const std::string& filepath);
bool                     isValid_trans(const std::string& filepath, Transaction trans);

#endif  // trans_hpp