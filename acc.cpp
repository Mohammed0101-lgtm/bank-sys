// account header implememtation 
#include "acc.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <openssl/rand.h>
#include <openssl/sha.h>

// setters:
void account::setOwner(const std::string& o) {
    owner = o;
}

void account::setEmail(const std::string& e) {
    email = e;
}

void account::setPassword(std::string& p) {
    password = p;
}

void account::setNumber(unsigned int n) {
    number = n;
}

void account::setBalance(double b) {
    balance = b;
}

// json maker 
json account::toJson() const {
    return {{"owner", owner}, 
            {"email", email}, 
            {"password", password},
            {"number", number},
            {"balance", balance}};
}

// methods
void account::store() {
    //if dir not exist
    if (!std::filesystem::exists(Account_dir))
        std::filesystem::create_directory(Account_dir);
    
    if (!std::filesystem::exists(Account_dir))
        throw std::runtime_error("Failed to create directory");
    
    // check owner name
    if (owner.empty()) {
        std::cerr << "Owner name cannot be empty" << std::endl;
        return;
    }

    // process filepath
    std::string filename = owner;
    filename.erase(std::remove_if(filename.begin(), filename.end(), ::isspace), filename.end());
    transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

    std::string filepath = Account_dir + filename + File_ext;

    // store the json file 
    std::ofstream accFile(filepath, std::ios::out | std::ios::trunc);

    if (!accFile.is_open()) {
        std::cerr << "Error creating account file" << std::endl;
        return; 
    } else {
        accFile << toJson().dump(2);

        if (!accFile) 
            throw std::runtime_error("Error writing to file");
        
        accFile.close();
    }
}

void account::display() {
    // process filename
    std::string filename = owner;
    filename.erase(std::remove_if(filename.begin(), filename.end(), ::isspace), filename.end());
    transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

    std::string filepath = Account_dir + filename + File_ext;

    // copy data
    json data;
    std::ifstream inFile(filepath, std::ios::in);

    if (!inFile.is_open())
        throw std::runtime_error("Error opening file");
    
    try
    {
        inFile >> data;
    }
    catch(const json::parse_error& e)
    {
        std::cerr << "Error parsing json file" << std::endl;
        inFile.close();
        return;
    }
    inFile.close();

    // display account information
    std::cout << "----------------------------" << '\n';
    std::cout << "| " << owner << "'s account |" << '\n';
    std::cout << "Email: " << email << '\n';
    std::cout << "Number: " << number << '\n';
    std::cout << "Balance: " << balance << '\n';
    std::cout << "----------------------------" << '\n';
}

void account::erase() {
    // process filename
    std::string filename = owner;
    filename.erase(std::remove_if(filename.begin(), filename.end(), ::isspace), filename.end());
    transform(filename.begin(), filename.end(), filename.end(), ::tolower);

    std::string filepath = Account_dir + filename + File_ext;

    const char *file = filepath.c_str();

    if (!remove(file)) 
        throw std::runtime_error("Failed to remove account");
    else 
        std::cout << "Account removed successfully!" << std::endl;
}

// hashing func
std::string generateSalt() {
  const int saltLength = 16;
  unsigned char salt[saltLength];
  RAND_bytes(salt, saltLength);

  char saltString[saltLength * 2 + 1];
  for (int i = 0; i < saltLength; i++)
    sprintf(saltString + i * 2, "%02x", salt[i]);

  return saltString;
}

std::string bcryptHash(const std::string &password, const std::string &salt) {
  std::string trimmedpassword = password;
  trimmedpassword.erase(std::remove_if(trimmedpassword.begin(), trimmedpassword.end(), ::isspace), trimmedpassword.end());

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


