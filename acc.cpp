// account header implememtation
#include "acc.hpp"
#include "config.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <openssl/sha.h>
#include <openssl/rand.h>

// setters:
void account::setowner(const std::string& o)    { owner         = o; }
void account::setEmail(const std::string& e)    { email         = e; }
void account::setPassword(std::string& p)       { password      = p; }
void account::setNumber(unsigned int n)         { number        = n; }
void account::setBalance(double b)              { balance       = b; }
void account::setType(const std::string& t)     { type          = t; }
void account::setInterestRate(double r)         { interest_rate = r; }

// getters:
std::string account::getEmail()     { return email;         }
std::string account::getType()      { return type;          }
std::string account::getOwner()     { return owner;         }
double account::getInterestRate()   { return interest_rate; }
double account::getBalance()        { return balance;       }
unsigned int account::getNumber()   { return number;        }

// json maker
nlohmann::json account::toJson() const {
    return {{"type"    , type          },
            {"owner"   , owner         },
            {"email"   , email         },
            {"password", password      },
            {"salt"    , salt          },
            {"number"  , number        },
            {"balance" , balance       },
            {"rate"    , interest_rate}};
}

void account::store() {
    if (!std::filesystem::exists(Account_dir)) {
        std::filesystem::create_directory(Account_dir);
        if (!std::filesystem::exists(Account_dir))
            throw std::runtime_error("Failed to create directory");
    }

    if (owner.empty()) {
        std::cerr << "owner name cannot be empty" << std::endl;
        return;
    }

    // Process filename
    std::string filename = owner;
    filename.erase(std::remove_if(filename.begin(), filename.end(), ::isspace), filename.end());
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

    std::string filepath = Account_dir + filename + File_ext;
    std::ofstream accFile(filepath, std::ios::out | std::ios::trunc);

    // Generate salt and hash the password
    std::string salt           = generateSalt();
    std::string hashedPassword = bcryptHash(password, salt);
    password                   = base64_encode(hashedPassword);
    std::string encodedSalt    = base64_encode(salt);

    if (!accFile.is_open()) {
        std::cerr << "Error creating account file" << std::endl;
        return;
    } else {
        // Modify toJson to include encoded salt
        auto json_data       = toJson();
        json_data.at("salt") = encodedSalt;
        accFile << json_data.dump(2);

        if (!accFile) {
            throw std::runtime_error("Error writing to file");
        }
        
        accFile.close();
    }
}


void account::display() {
    account acc;

    std::string filename = owner;
    filename.erase(std::remove_if(filename.begin(), filename.end(), ::isspace), filename.end());
    transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

    std::string filepath  = Account_dir + filename + File_ext;
    nlohmann::json js_data = get_jsonData(filepath);

    std::cout << "+--------------------------+"  << '\n';
    std::cout << "| " << owner << "'s account |" << '\n';
    std::cout << "+--------------------------+"  << '\n';
    
    std::cout << "Email   : " << js_data.at("email")   << '\n';
    std::cout << "Number  : " << js_data.at("number")  << '\n';
    std::cout << "Balance : " << js_data.at("balance") << '\n';
    
    std::cout << "----------------------------" << '\n';
}
// hashing func
std::string generateSalt() {
    const size_t saltLength = 16;
    unsigned char salt[saltLength];
    RAND_bytes(salt, saltLength);
    char saltString[saltLength * 2 + 1];
    
    for (size_t i = 0; i < saltLength; i++) 
        snprintf(saltString + i * 2, 3, "%02x", salt[i]);

    return std::string(saltString);
}


std::string bcryptHash(const std::string &password, const std::string &salt) {
    std::string trimmedpassword = password;
    trimmedpassword.erase(std::remove_if(trimmedpassword.begin(), 
                          trimmedpassword.end(), ::isspace), trimmedpassword.end());

    const int rounds = 12;

    std::string hash;
    hash.resize(SHA256_DIGEST_LENGTH);

    int result =
        PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                            reinterpret_cast<const unsigned char *>(salt.c_str()),
                            salt.length(), rounds, EVP_sha256(), hash.length(),
                            reinterpret_cast<unsigned char *>(&hash[0]));

    if (result == 0)
        throw std::runtime_error("Error in hashing process");

    return hash;
}

