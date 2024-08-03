#include <iostream>
#include "acc.hpp"
#include "io.hpp"
#include "bank.hpp"

int main(void) {
    in_out input;
    std::cout << "+-------------+" << '\n';
    std::cout << "| Bank System |"  << '\n'; 
    std::cout << "+-------------+" << '\n';
    std::cout << "1. Sign up" << '\n';
    std::cout << "2. Log in" << '\n';
    std::cout << "3. Show account list" << '\n';
    std::cout << "4. Search account" << '\n';
    std::cout << "0. Exit" << '\n';
    std::cout << "---------------" << '\n';

    unsigned int choice;
    std::cout << "Enter choice: ";
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    while (choice < 0 || choice > 5) {
        std::cout << "Unavailable choice, try again: ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    switch (choice) {
        case 1:
            input.create_acc();
            break;
        case 2:
            input.login();
            break;
        case 3:
            input.display_acc_list();
            break;
        case 4:
            input.search_acc();
            break;
        case 0:
            std::cout << "Program exited!" << std::endl;
            break;
        default:
            throw std::runtime_error("Error processing input");
            break;
    }

    return 0;
}
