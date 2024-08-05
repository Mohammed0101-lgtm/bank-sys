
#include <nlohmann/json.hpp>
#include <regex>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "bank.hpp"
#include "acc.hpp"
#include "config.hpp"
#include "io.hpp"


const std::string saving_account   = "saving";
const std::string checking_account = "check";

std::string get_Name() {
    std::string fullname = get_string("Enter full name");
    
    while (fullname.empty()) 
        fullname = get_string("name cannot be empty, try again");

    for (int i = 0; fullname[i] != '\0'; i++) 
        if (!isalpha(fullname[i]) && !isspace(fullname[i]) && fullname[i] != '\n') {
            std::cerr << "Invalid character detected: " << fullname[i] << '\n';
            return "";
        }
    
    return fullname;
}

bool isValidEmail(const std::string& email) {
    const std::regex email_regex(
        "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");

    return std::regex_match(email, email_regex);
}

std::string getEmail() {
    std::string email = get_string("Enter email adress");

    while (email.empty()) 
        email = get_string("Email cannot be empty, try again");

    while (!isValidEmail(email)) {
        std::cout << "Invalid Email format!"                          << '\n';
        std::cout << "Email format should be: example@exaple.example" << '\n';
        
        email = get_string("Enter email address again");
        
        while (email.empty()) 
            email = get_string("Email cannot be empty, try again");
    }
    
    email.erase(std::remove_if(email.begin(), email.end(), ::isspace), email.end());
    transform(email.begin(), email.end(), email.begin(), ::tolower);

    return email;
}

bool strongPassword(const std::string& password) {
    int lowerCase = 0;
    int digit     = 0;
    int upperCase = 0;

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
    std::string password = get_string("Enter password");
    
    while (password.empty()) 
        password = get_string("password cannot be empty, try again");

    while (!strongPassword(password)) {
        std::cout << "Password is weak, you're password should meet the following requirements:" << '\n';
        std::cout << "- Have six characters or more"                      << '\n';
        std::cout << "- At least one upper case letter"                   << '\n';
        std::cout << "- At least one lower case letter"                   << '\n';
        std::cout << "- At least one digit"                               << '\n';
        std::cout << "- Shouldn't have three same consecutive characters" << '\n';
        
        password = get_string("try again");
        while (password.empty()) 
            password = get_string("password cannot be empty, try again");
    }

    return password;
}

bool match(const std::string &name, const std::string &accountowner) {
    std::string strippedAccountowner = accountowner;
    strippedAccountowner.erase(std::remove(strippedAccountowner.begin(),
                                           strippedAccountowner.end(), '\"'),
                                           strippedAccountowner.end());

    std::vector<std::string> enteredTokens;
    std::istringstream       enteredStream(name);
    std::string              enteredToken;
    
    while (enteredStream >> enteredToken) 
        enteredTokens.push_back(enteredToken);

    std::vector<std::string> accountownerTokens;
    std::istringstream       ownerStream(strippedAccountowner);
    std::string              ownerToken;
    
    while (ownerStream >> ownerToken) 
        accountownerTokens.push_back(ownerToken);

    for (const std::string &enteredToken : enteredTokens) 
        if (std::find(accountownerTokens.begin(), accountownerTokens.end(), 
                      enteredToken) == accountownerTokens.end())
            return false;

    return true;
}

std::vector<std::string> get_accList() {
    std::vector<std::string> owners;

    for (const auto &entry : std::filesystem::directory_iterator(Account_dir)) 
        if (entry.is_regular_file() && entry.path().extension() == File_ext) {
            nlohmann::json data = get_jsonData(entry.path());
            owners.push_back(data.at("owner"));
        }
        
    return owners;
}

bool validPassword(const std::string &stored_password, const std::string& encoded_salt, std::string &password) {
    std::string salt                    = base64_decode(encoded_salt);
    std::string decoded_stored_password = base64_decode(stored_password);
    std::string hashed_input_password   = bcryptHash(password, salt);

    return decoded_stored_password == hashed_input_password;
}

void in_out::create_acc() {
    account acc;

    std::string account_type = get_string("Enter account type [ckeck / saving] : ");
    
    if (account_type != checking_account && account_type != saving_account) 
        account_type = get_string("option unavailable , Enter account type [ckeck / saving] :");
    
    acc.setType(account_type);

    std::string ownerName = get_Name();
    std::string email     = getEmail();
    std::string password  = getPassword();

    acc.setEmail(email);
    acc.setowner(ownerName);
    acc.setPassword(password);

    unsigned int counter = 1;
    if (!std::filesystem::is_directory(Account_dir)) 
        std::filesystem::create_directory(Account_dir);
        
    if (!std::filesystem::exists(Account_dir)) 
        throw std::runtime_error("Failed to create account");

    for (const auto& entry : std::filesystem::directory_iterator(Account_dir)) 
        if (entry.is_regular_file() && entry.path().extension() == File_ext) {
            counter++;

    acc.setNumber(counter);
    acc.setBalance(0.0);
    acc.setInterestRate(0.0);
    acc.store();

    std::cout << "Account created successfully!" << std::endl;
}

void in_out::search_acc() {
    account acc;
    std::string owner = get_Name();

    try {
        for (const auto &entry : std::filesystem::directory_iterator(Account_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == File_ext) {
                const std::string filePath = entry.path().string();

                if (std::filesystem::exists(filePath)) {

                    nlohmann::json data = get_jsonData(filePath);

                    if (match(owner, data["owner"])) {
                        std::cout << "Account found!" << '\n';
                        
                        std::cout << "------------------------------" << '\n';
                        std::cout << "1. Display account information" << '\n';
                        std::cout << "2. Return"                      << '\n';
                        std::cout << "------------------------------" << '\n';
                        
                        int choice;
                        std::cout << '\n' << "Enter choice: ";
                        std::cin  >> choice;
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        
                        if (choice == 1) {
                            acc.setowner(owner);
                            acc.display();
                            return;
                        } else if (choice == 2) {
                            return;
                        }
                    }
                } else  {
                    throw std::runtime_error("File not found");
                }
            }
        }
        
        throw std::runtime_error("Account not found");
    } catch (const std::exception &e) 
        std::cerr << "Error: " << e.what() << std::endl;
}

void in_out::remove_acc() {
    account acc;
    std::string owner = get_Name();

    for (const auto &entry : std::filesystem::directory_iterator(Account_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == File_ext) {
            nlohmann::json data = get_jsonData(entry.path());

            if (match(owner, data.at("owner"))) {
                if (!remove(entry.path())) 
                    throw std::runtime_error("Failed to remove file!");
                
                std::cout << "Account removed successfully!" << std::endl;
                return;
            }
        }
    }

    std::cerr << "Account not found!" << std::endl;
}

void in_out::display_acc_list() {
    std::vector<std::string> acc_list = get_accList();

    std::cout << "+---------------+" << '\n';
    std::cout << "| Accounts list |" << '\n';
    std::cout << "+---------------+" << '\n';
    
    for (std::string owner : acc_list)
        std::cout << "* " << owner << '\n';
    
    std::cout << "-----------------" << '\n';

    char choice;
    std::cout << "Display an account information[Y / N]: ";
    std::cin  >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice == 'y' || choice == 'Y') {
        std::string owner    = get_Name();
        std::string filename = owner;
        
        filename.erase(std::remove_if(filename.begin(), filename.end(), ::isspace), filename.end());
        transform(filename.begin(), filename.end(), filename.end(), ::tolower);

        std::string filepath   = Account_dir + filename + File_ext;
        nlohmann::json js_data = get_jsonData(filepath);

        std::cout << "+-------------------------------+" << '\n';
        std::cout << "| " << js_data.at("owner") << " |" << '\n';
        std::cout << "+-------------------------------+" << '\n';
        std::cout << "* Account number : " << js_data.at("number") << '\n';
        std::cout << "* Email adress : " << js_data.at("email")    << '\n';
        std::cout << "* Account type : " << js_data.at("type")     << '\n';
        std::cout << "--------------------------------"  << std::endl;
    }
    
    else if (choice == 'n' || choice == 'N')
        return;
    else
        throw std::runtime_error("Entered undefined character.");
}

void accessAccount(const std::string& filepath) {
    std::cout <<  "------------------------------"  << '\n';
    std::cout <<  "1. Display account information"  << '\n';
    std::cout <<  "2. Deposit"                      << '\n';
    std::cout <<  "3. Withdraw"                     << '\n';
    std::cout <<  "4. Make a transaction"           << '\n';
    std::cout <<  "5. Remove account"               << '\n';
    std::cout <<  "0. Log out"                      << '\n';
    std::cout <<  "------------------------------"  << '\n';

    int choice;
    std::cout << "Enter choice: ";
    std::cin  >> choice;

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    while (choice < 0 || choice > 5) {
        std::cerr << "Error: incorrect choice, try again: ";
        std::cin  >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    account acc;
    nlohmann::json data = get_jsonData(filepath);

    if (data.find("owner") == data.end())
        throw std::runtime_error("Error: 'owner' key not found in JSON data.");

    acc.setowner(data.at("owner"));

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
        case 4: {
            banking bank;
            bank.make_trans(filepath);
            break;
        }
        case 5: {
            std::string confirme = get_string("Do you really want to remove this account?[Y/N]");
            while (confirme != "Y" && confirme != "N" && confirme != "y" && confirme != "n") 
                confirme = get_string("Do you really want to remove this account?[Y/N]");

            if (confirme == "Y" || confirme == "y") {
                remove(filepath.c_str());
                break;
            } else 
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

void in_out::login() {
    std::string email    = getEmail();
    std::string password = get_string("Enter password");
    bool found           = false; 
    bool correct         = false;

    for (const auto &entry : std::filesystem::directory_iterator(Account_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == File_ext) {
            nlohmann::json data = get_jsonData(entry.path());
            
            if (email == data.at("email")) {
                found = true;
                
                if (validPassword(data.at("password"), data.at("salt"), password)) {
                    correct = true;
                    accessAccount(entry.path());
                }
            } else
                continue;
        }
    }
 
    if (!found) {
        std::cerr << "Email adress not found!" << std::endl;
        exit(EXIT_FAILURE);
    }
 
    if (!correct) {
        std::cerr << "Password is incorrect!" << std::endl;
        exit(EXIT_FAILURE);
    }
} 
