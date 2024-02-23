// banking header
#ifndef BANK_H
#define BANK_H

#include <iostream>

class banking {
private:
public:
    void update(const char operation, const std::string& filepath);
    void withdraw(const std::string& filepath);
    void deposit(const std::string& filepath);
};

#endif // BANK_H