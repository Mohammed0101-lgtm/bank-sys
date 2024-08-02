// input output manager implementation
#include "in_out.hpp"
#include "bank.hpp"
#include "acc.hpp"

#include <regex>
#include <fstream>
#include <sstream>

// helper funcs
std::string getOwnerName() {
    std::string fullname;
    std::cout << "Enter fullname: ";
    std::getline(std::cin, fullname);

    for (char letter : fullname) {
        if (!isalpha(letter) && !isspace(letter) && letter != '\n') {
            std::cerr << "Invalid character detected: " << letter << '\n';
            exit(EXIT_FAILURE);
        }
    }

    size_t len = fullname.length();
    if (len > 0 && fullname[len - 1] == '\n')
        fullname[len - 1] = '\0';
    
    return fullname;
}

bool isValidEmail(const std::string& email) {
    const std::regex email_regex(
        "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");

    return std::regex_match(email, email_regex);
}

std::string getEmail() {
    std::string email;
    std::cout << "Enter email adress: ";
    std::getline(std::cin, email);

    while (!isValidEmail(email)) {
        std::cout << "Invalid Email format!" << '\n';
        std::cout << "Email format should be: example@exaple.example" << '\n';
        std::cout << "Enter email adress again: ";
        std::getline(std::cin, email);
    }

    size_t len = email.length();
    if (len > 0 && email[len - 1] == '\n')
        email[len - 1] = '\0';
    
    email.erase(std::remove_if(email.begin(), email.end(), ::isspace), email.end());
    transform(email.begin(), email.end(), email.begin(), ::tolower);

    return email;
}

bool strongPassword(const std::string& password) {
    int lowerCase = 0, digit = 0, upperCase = 0;

    if (password.length() < 6)
        return false;

    for (int i = 0, n = password.length(); i < n - 2; i++) {
        if (password[i] == password[i + 1] && password[i] == password[i + 2])
            return false;

        if (isdigit(password[i]))      digit++;
        else if (islower(password[i])) lowerCase++;
        else if (isupper(password[i])) upperCase++;
    }

    if (lowerCase == 0 || upperCase == 0 || digit == 0)
        return false;

    return true;
}

std::string getPassword() {
    std::string password;
    std::cout << "Enter password: ";
    std::getline(std::cin,password);

    while (!strongPassword(password)) {
        std::cout << "Password is weak, you're password should meet the following requirements:" << '\n';
        std::cout << "- Have six characters or more" << '\n';
        std::cout << "- At least one upper case letter" << '\n';
        std::cout << "- At least one lower case letter" << '\n';
        std::cout << "- At least one digit" << '\n';
        std::cout << "- Shouldn't have three same consecutive characters" << '\n';
        std::cout << "Try again: ";
        std::getline(std::cin, password);
    }

    return password;
}

bool match(const std::string &name, const std::string &accountOwner) {
    std::string strippedAccountOwner = accountOwner;
    strippedAccountOwner.erase(std::remove(strippedAccountOwner.begin(),
                                            strippedAccountOwner.end(), '\"'),
                                strippedAccountOwner.end());

    // Tokenize entered name
    std::vector<std::string> enteredTokens;
    std::istringstream enteredStream(name);
    std::string enteredToken;
    
    while (enteredStream >> enteredToken) 
        enteredTokens.push_back(enteredToken);

    // Tokenize account owner name
    std::vector<std::string> accountOwnerTokens;
    std::istringstream ownerStream(strippedAccountOwner);
    std::string ownerToken;
    
    while (ownerStream >> ownerToken) 
        accountOwnerTokens.push_back(ownerToken);


    // Check if all entered tokens are present in the account owner name
    for (const std::string &enteredToken : enteredTokens) {
        if (std::find(accountOwnerTokens.begin(), accountOwnerTokens.end(),
                    enteredToken) == accountOwnerTokens.end())
        return false;
    }

    return true;
}

std::vector<std::string> getAccList() {
    std::vector<std::string> accOwners;

    for (const auto &entry : std::filesystem::directory_iterator(Account_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == File_ext) {
            json data;

            std::ifstream inFile(entry.path(), std::ios::in);

            if (!inFile.is_open())
                throw std::runtime_error("Error opening file");

            try 
                inFile >> data;
            catch(const json::parse_error& e) {
                std::cerr << "Error parsing json file: " << entry.path() <<std::endl;
                inFile.close();
                exit(EXIT_FAILURE);
            }

            inFile.close();

            accOwners.push_back(data["Owner"]);
        }
    }
    return accOwners;
}

bool validPassword(const std::string &filepath, std::string &pass) {
    // copy file data to memory
    json data;

    std::ifstream inFile(filepath, std::ios::in);

    if (!inFile.is_open())
        throw std::runtime_error("Error opening file");

    try {   
        inFile >> data;
    }
    catch(const json::parse_error& e) {
        std::cerr << "Error parsing json file: " << std::endl;
        inFile.close();
        exit(EXIT_FAILURE);
    }

    inFile.close();

    // check if the hash key exists in json data
    if (data.find("Hash") == data.end()) {
        std::cerr << "Error: 'Hash' key not found in JSON data." << std::endl;
        return false;
    }

    // Compare hashed passwords
    std::string storedHash = data["Hash"];
    std::string salt       = generateSalt();
    std::string inputHash  = bcryptHash(pass, salt);
   
    storedHash.erase(std::remove_if(storedHash.begin(), storedHash.end(), ::isspace), storedHash.end());
    inputHash.erase(std::remove_if(inputHash.begin(), inputHash.end(), ::isspace), inputHash.end());

    if (storedHash == inputHash)
        return true;
    else
        return false;
}



// class funcs
void io::create_acc() {
    account acc;

    std::string ownerName = getOwnerName();
    std::string email     = getEmail();
    std::string password  = getPassword();
    
    acc.setOwner(ownerName);
    acc.setEmail(email);
    acc.setPassword(password);

    // get number
    unsigned int counter = 1;
    if (!std::filesystem::is_directory(Account_dir))
         std::filesystem::create_directory(Account_dir);
    if (!std::filesystem::exists(Account_dir))
        throw std::runtime_error("Failed to create account");

    for (const auto& entry : std::filesystem::directory_iterator(Account_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == File_ext) 
            counter++;
    }
    
    acc.setNumber(counter);
    acc.setBalance(0.0);
    acc.store();
}

void io::search_acc() {
    account acc;
    // get owner name:
    std::string owner = getOwnerName();

    /*-----iterate dir for account-----*/
    try {
        for (const auto &entry : std::filesystem::directory_iterator(Account_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == File_ext) {
                const std::string filePath = entry.path().string();

                // display data if "file found":
                if (std::filesystem::exists(filePath)) {

                    json data;
                    std::ifstream inFile(filePath, std::ios::in);

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

                    if (match(owner, data["owner"])) {
                        std::cout << "Account found!" << '\n';
                        std::cout << "------------------------------" << '\n';
                        std::cout << "1. Display account information" << '\n';
                        std::cout << "2. Return" << '\n';
                        std::cout << "------------------------------" << '\n';
                        
                        int choice;
                        std::cout << '\n' << "Enter choice: ";
                        std::cin >> choice;
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        
                        if (choice == 1) {
                            acc.setOwner(owner);
                            acc.display();
                            return;
                        } 
                        else if (choice == 2) 
                            return;
                    }
                }
                // throw error if "file not found":
                else
                    throw std::runtime_error("File not found");
            }
        }
        // account not found:
        throw std::runtime_error("Account not found");
    } catch (const std::exception &e) 
        std::cerr << "Error: " << e.what() << std::endl;
}

void io::remove_acc() {
    account acc;
    std::string owner = getOwnerName();

    for (const auto &entry : std::filesystem::directory_iterator(Account_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == File_ext) {
            json data;

            std::ifstream inFile(entry.path(), std::ios::in);

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

            // remove account if found
            if (match(owner, data["Owner"])) {
                acc.setOwner(data["Owner"]);
                acc.erase();
                
                exit(EXIT_SUCCESS);
            }
        }
    }
    std::cerr << "Account not found!" << std::endl;
}

void io::display_acc_list() {
    // get the list of accounts owners names:
    std::vector<std::string> acc_list = getAccList();

    // display names:
    std::cout << "-----------------" << '\n';
    std::cout << "| Accounts list |" << '\n';
    std::cout << "-----------------" << '\n';
    
    for (std::string owner : acc_list)
        std::cout << "* " << owner << "." << '\n';
    
    std::cout << "-----------------" << '\n';

    /*---- ask for for account display----*/
    // get user choice:
    char choice;
    std::cout << "Display an account information(Y/N): ";
    std::cin  >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // display if choice 'y':
    if (choice == 'y' || choice == 'Y') {
        // get the account owner name:
        std::string owner;
        std::cout << "Enter the owner's name: ";
        std::getline(std::cin, owner);

        for (char letter : owner) 
            if (!isalpha(letter) && !isspace(letter) && letter != '\n') {
                std::cerr << "Invalid character detected." << std::endl;
                exit(EXIT_FAILURE);
            }

        size_t len = owner.length();
        if (len > 0 && owner[len - 1] == '\n')
            owner[len - 1] = '\0';

        // display account info:
        account acco;
        acco.setOwner(owner);
        acco.display();
    }
    // ignore if choice 'n'
    else if (choice == 'n' || choice == 'N')
        return;
    else
        throw std::runtime_error("Entered undefined character.");
}

void accessAccount(const std::string& filepath) {
    std::cout << "------------------------------" << '\n';
    std::cout << "1. Display account information" << '\n';
    std::cout << "2. Deposit" << '\n';
    std::cout << "3. Withdraw" << '\n';
    std::cout << "0. Exit" << '\n';
    std::cout << "------------------------------" << '\n';

    int choice;
    std::cout << "Enter choice: ";
    std::cin  >> choice;

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    while (choice < 0 || choice > 3) {
        std::cerr << "Error: incorrect choice, try again: ";
        std::cin  >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    account acc;
    json    data;

    std::ifstream inFile(filepath, std::ios::in);

    if (!inFile.is_open())
        throw std::runtime_error("Error opening file");

    try 
        inFile >> data;
    catch(const json::parse_error& e) {
        std::cerr << "Error parsing json file: " << std::endl;
        inFile.close();
        return;
    }

    inFile.close();

    if (data.find("Owner") == data.end())
        throw std::runtime_error("Error: 'Owner' key not found in JSON data.");

    acc.setOwner(data["Owner"]);

    switch (choice) {
    case 1:
        acc.display();
        break;
    case 2: {
        banking bank;
        bank.deposit(filepath);
        break;
    }
    case 3: {
        banking bank;
        bank.withdraw(filepath);
        break;
    }
    case 0:
        std::cout << "Exiting program!" << std::endl;
        break;
    default:
        throw std::runtime_error("Failed to register input");
        break;
    }
}

void io::login() {
    // get email
    std::string email = getEmail();

    // get password
    std::string password;
    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    size_t len = password.length();
    if (len > 0 && password[len - 1] == '\n')
        password[len - 1] = '\0';
    
    bool found = false, correct = false;

    for (const auto &entry : std::filesystem::directory_iterator(Account_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == File_ext) {
            // copy file data to memory
            json data;

            std::ifstream inFile(entry.path(), std::ios::in);

            if (!inFile.is_open())
                throw std::runtime_error("Error opening file");

            try {   
                inFile >> data;
            }
            catch(const json::parse_error& e) {
                std::cerr << "Error parsing json file: " << entry.path() <<std::endl;
                inFile.close();
                return;
            }

            inFile.close();
            
            // compare email input
            if (email == data["Email"]) {
                found = true;
                if (validPassword(entry.path(), password)) {
                    correct = true;
                    accessAccount(entry.path());
                }
            } else
                continue;
        }
    }
}
