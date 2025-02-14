#include <iostream>
#include <string>
#include <cctype>  
#include <sstream>
#include <random>
#include <algorithm>
#include <chrono>
#include <thread>

std::string removeWhitespace(const std::string& input) {
    std::string cleaned;
    for (char c : input) {
        if (!isspace(c)) {  // Keep only non-space characters
            cleaned += c;
        }
    }
    return cleaned;
}

bool isNonDecimalDigit(const std::string& input) {
    std::string cleaned = removeWhitespace(input); // Step 1: Clean input

    // Check if it's a normal integer
    if (cleaned.find('.') == std::string::npos) { 
        for (char c : cleaned) {
            if (!isdigit(c)) return false;
        }
        return true;  // It's a valid integer
    }

    // Check if it's a float ending in ".0"
    std::istringstream iss(cleaned);
    float f;
    char extra;
    if ((iss >> f) && !(iss >> extra)) { // Check valid float
        return (f == static_cast<int>(f));  // Only valid if it's a whole number
    }

    return false; // If none of the above checks passed, it's invalid
}


int getRandomNumber(int min, int max) {
    std::random_device rd;  // Seed for the random number engine
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<int> dist(min, max); // Uniform distribution

    return dist(gen); // Generate and return a random number
}

bool checkAnswer(int n1, int n2, int answer) {
    int correct = n1 * n2;
    if (correct == answer) {
        return true;
    }
    return false;
}

int main() {
    std::vector<int> digits1;
    std::vector<int> digits2;
    std::string userInput;

    while (digits1.size() < 2) {
        std::cout << "Enter min or max value for interval 1(or type 'exit' to quit): ";
        std::getline(std::cin, userInput);

        if (userInput == "exit") {
            std::cout << "Goodbye!\n";
            break;
        }

        if (isNonDecimalDigit(userInput)) {
            int number = std::stoi(removeWhitespace(userInput));
            digits1.push_back(number);
        }  else {
            std::cout << "Only non-decimal values are allowed.";
        }
    }

        std::cout << "\n";
    
    while (digits2.size() < 2) {
        std::cout << "Enter min or max value for interval 2(or type 'exit' to quit): ";
        std::getline(std::cin, userInput);
    
        if (userInput == "exit") {
            std::cout << "Goodbye!\n";
            break;
        }
    
        if (isNonDecimalDigit(userInput)) {
            int number = std::stoi(removeWhitespace(userInput));
            digits2.push_back(number);
        }  else {
            std::cout << "Only non-decimal values are allowed.";
        }
    }

        std::cout << "\n";

    int min1 = *std::min_element(digits1.begin(), digits1.end());
    int max1 = *std::max_element(digits1.begin(), digits1.end());
    int min2 = *std::min_element(digits2.begin(), digits2.end());
    int max2 = *std::max_element(digits2.begin(), digits2.end());

    int seconds = 120;
    int corrects = 0;

    auto start = std::chrono::steady_clock::now();
    auto end_time = start + std::chrono::seconds(seconds);

    while (std::chrono::steady_clock::now() < end_time) {
    
        auto now = std::chrono::steady_clock::now();
        int elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
        int timeLeft = seconds - elapsedSeconds;

        int n1 = getRandomNumber(min1, max1);
        int n2 = getRandomNumber(min2, max2);
        
        std::cout << "\nTime left: " << timeLeft << " seconds" << std::endl;
        std::cout << "\n" << n1 << " * " << n2 << " = ";
        std::getline(std::cin, userInput);
        
        if (userInput == "exit") {
            std::cout << "Goodbye!\n";
            break;
        }
        
        int answer = std::stoi(removeWhitespace(userInput));
        if (checkAnswer(n1, n2, answer)) {
            corrects += 1;
            std::cout << "Correct!\n";
        } else {
            std::cout << "Wrong, correct answer is " << n1*n2 << "\n";
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));  // Wait 1 sec between iterations
    }
    std::cout << "\nYou answered " << corrects << " questions correctly in " << seconds << " seconds!\n";
    
    return 0;
}
