#ifndef ACC_HPP
#define ACC_HPP

#include <nlohmann/json.hpp>

const std::string Account_dir = "Accounts/";
const std::string File_ext = ".json";

class account
{
private:
    std::string type;
    std::string owner;
    std::string email;
    std::string password;
    std::string salt;
    double balance;
    double interest_rate;
    unsigned int number;
public:
    account() : balance(0.0) {}

    // setters:
    void setType(const std::string& t);
    void setInterestRate(double r);
    void setowner(const std::string& o);
    void setBalance(double b);
    void setNumber(unsigned int n);
    void setEmail(const std::string& e);
    void setPassword(std::string& p);

    // json maker 
    nlohmann::json toJson() const;

    // methods
    void store();
    void display();
    void remove_file();

    // getters
    std::string getType();
    double getInterestRate();
    std::string getOwner();
    double getBalance();
    unsigned int getNumber();
    std::string getEmail();
};

// hashing func
std::string generateSalt();
std::string bcryptHash(const std::string& password, const std::string& salt);

#endif // ACC_HPP
