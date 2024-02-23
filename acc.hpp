// account header
#ifndef ACC_H
#define ACC_H

#include "/Users/mac/c proj/json.hpp"

using json = nlohmann::json;

const std::string Account_dir = "Accounts/";
const std::string File_ext = ".json";

class account
{
private:
    std::string owner;
    std::string email;
    std::string password;
    double balance;
    unsigned int number;
public:
    account() : balance(0.0) {}

    // setters:
    void setOwner(const std::string& o);
    void setBalance(double b);
    void setNumber(unsigned int n);
    void setEmail(const std::string& e);
    void setPassword(std::string& p);

    // json maker 
    json toJson() const;

    // methods
    void store();
    void display();
    void erase();
};

// hashing func
std::string generateSalt();
std::string bcryptHash(const std::string& password, const std::string& salt);

#endif // ACC_H
