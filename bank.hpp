// banking header
#ifndef BANK_H
#define BANK_H

#include <iostream>

class banking {
private:
    int var = 0;
public:
    void update(const char operation, const std::string& filepath);
    void withdraw(const std::string& filepath);
    void deposit(const std::string& filepath);
    void make_trans(const std::string& filepath);
    void make_loan(const std::string& filepath);
};

#endif // BANK_H