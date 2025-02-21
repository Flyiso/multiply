#include <iostream>
#include <string>
#include <cctype>  
#include <sstream>
#include <random>
#include <algorithm>
#include <chrono>
#include <thread>
#include <sys/stat.h>
#include <sqlite3.h>

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

bool ensureScoreTable(sqlite3* db) {
    const char* sql = "CREATE TABLE IF NOT EXISTS scores ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "playerName TEXT NOT NULL, "
                      "minuteScore REAL NOT NULL, "
                      "scopePairsSTR TEXT NOT NULL, "
                      "percentageCorrect REAL NOT NULL, "
                      "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";

    char* errorMessage = nullptr;
    int exitCode = sqlite3_exec(db, sql, nullptr, nullptr, &errorMessage);
    if (exitCode != SQLITE_OK) {
        std::cerr << "Error creating table: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        return false;
    }
    return true;
}

bool dbAddResults(
    sqlite3* db,
    const std::string& playerName,
    const std::string& scopePairsSTR,
    double minuteScore,
    double percentageCorrect
) {
    const char* sql = "INSERT INTO scores (playerName, scopePairsSTR, minuteScore, percentageCorrect) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Preparation failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    if (sqlite3_bind_text(stmt, 1, playerName.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        std::cerr << "Binding playerName failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    if (sqlite3_bind_text(stmt, 2, scopePairsSTR.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        std::cerr << "Binding description failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    if (sqlite3_bind_double(stmt, 3, minuteScore) != SQLITE_OK) {
        std::cerr << "Binding score failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    if (sqlite3_bind_double(stmt, 4, percentageCorrect) != SQLITE_OK) {
        std::cerr << "Binding Percentage correct failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

int main() {
    std::vector<int> digits1;
    std::vector<int> digits2;
    std::string userInput;

    // default/placeholder user name (to be removed/replaced with input)
    std::string playerName = "Anonomous";


    // Ensure DB scores setup
    std::string dbName = "game_scores.db";
    sqlite3* db;
    int exitCode = sqlite3_open(dbName.c_str(), &db);
    if (exitCode != SQLITE_OK) {
        std::cerr << "Error opening database:" << sqlite3_errmsg(db) << std::endl;
    }
    ensureScoreTable(db);
    sqlite3_close(db);
    std::cout << "Database setup complete!\n\n" << std::endl;
    // DB scores setup done


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

    int seconds = 120; // 120, but adjustable in future.
    int corrects = 0;
    int wrongs = 0;
    int totalAnswers = 0;
    double percentageCorrect = 0.0;

    auto start = std::chrono::steady_clock::now();
    auto end_time = start + std::chrono::seconds(seconds);

    while (std::chrono::steady_clock::now() < end_time) {

        auto now = std::chrono::steady_clock::now();
        int elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
        int timeLeft = seconds - elapsedSeconds;

        int n1 = getRandomNumber(min1, max1);
        int n2 = getRandomNumber(min2, max2);

        bool answerApproved = false;
        
        while (! answerApproved) {
            std::cout << "\nTime left: " << timeLeft << " seconds" << " ------------ ";
            std::cout << "Current accurracy: "<< percentageCorrect << "%" << std::endl;
            std::cout << "\n" << n1 << " * " << n2 << " = ";
            std::getline(std::cin, userInput);
            userInput = removeWhitespace(userInput);
            if (userInput == "exit" ){
                answerApproved = true;
                break;
            } else if (isNonDecimalDigit(userInput)) {
                answerApproved = true;
                break;
            } else {
                std::cout << "Guess not in valid format. Enter a non-decimal number to guess or 'exit' to exit" << std::endl;
            }
        }
            
        if (userInput == "exit") {
            std::cout << "Goodbye!\n";
            break;
        }

        int answer = std::stoi(userInput);
        if (checkAnswer(n1, n2, answer)) {
            corrects += 1;
            totalAnswers += 1;
            std::cout << "Correct!\n";
        } else {
            std::cout << "Wrong, correct answer is " << n1*n2 << "\n";
            wrongs += 1;
            totalAnswers += 1;
        }

        if (totalAnswers > 0) {
            percentageCorrect = (((1.0 * corrects) / (1.0 * totalAnswers)) * (100.0));
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));  // Wait 1 sec between iterations
        }

    std::cout << "\nYou answered " << corrects << " / " << totalAnswers << " questions correctly in " << seconds << " seconds!\n";
    std::cout << "Your accurracy was: " << percentageCorrect << "% !\n\n";

    
    // Prepare and add to db.
    double minuteScore = (1.0 * corrects) / ((1.0 * seconds) / 60.0);
    std::ostringstream scopePairs;
    if (min1 > min2) {
        scopePairs << min1 << "," << max1 << "-" << min2 << ',' << max2;
    } else if (min2 > min1) {
        scopePairs << min2 << "," << max2 << "-" << min1 << ',' << max1;
    } else if (max1 - min1 < max2 - min2) {
        scopePairs << min1 << "," << max1 << "-" << min2 << ',' << max2;
    } else if (max2 - min2 < max1 - min1) {
        scopePairs << min2 << "," << max2 << "-" << min1 << ',' << max1;
    } else {
        scopePairs << min1 << "," << max1 << "-" << min2 << ',' << max2;
    }
    std::string scopePairsSTR = scopePairs.str();

    // open db and write.
    if (sqlite3_open("game_scores.db", &db)) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }
    if (dbAddResults(db, playerName, scopePairsSTR, minuteScore, percentageCorrect)) {
        std::cout << "New scores successfully saved" << std::endl;
    } else {
        std::cerr << "Failed to store new scores" << std::endl;
    }
    sqlite3_close(db);
    // DONE saving scores

    return 0;
}
