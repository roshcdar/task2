#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <set>

namespace fs = std::filesystem;

bool readInput(std::string & source, std::string & replica, int & interval, std::string & log) {
    std::cout << "Enter source folder path:" << std::endl;
    if (!(std::cin >> source))
        return false;
    std::cout << "Enter replica folder path:" << std::endl;
    if (!(std::cin >> replica))
        return false;
    std::cout << "Enter synchronization interval in seconds:" << std::endl;
    if (!(std::cin >> interval || interval <= 0))
        return false;
    std::cout << "Enter log file path:" << std::endl;
    if (!(std::cin >> log))
        return false;
    return true;
}

void removeFolderContent(const std::string& folder)
{
    for (const auto& entry : fs::directory_iterator(folder))
        fs::remove_all(entry.path());
}

enum OperationType{created, removed, copied};

void logOperation(const std::string & path,  std::ostream & os, OperationType operationType) {
    switch(operationType) {
        case created: {
            os << path + " is created.\n";
            break;
        }
        case copied: {
            os << path + " is copied.\n";
            break;
        }
        case removed: {
            os << path + " is removed.\n";
            break;
        }
    }
}

void logInfo(const std::string & source, const std::string & log, std::set<std::string> & lastLog) {
    std::set<std::string> currentLog;
    std::set<std::string>::iterator it;
    std::string path;
    std::ofstream myFile;
    myFile.open(log);
    for (const auto& entry : fs::recursive_directory_iterator(source)) {
        if (is_regular_file(entry.path())) {
            path = entry.path().generic_string();
            it =  lastLog.find(path);
            if (it == lastLog.end()) {
                logOperation(path, myFile, created);
                logOperation(path, std::cout, created);
            }
            else
                lastLog.erase(it);
            logOperation(path, myFile, copied);
            logOperation(path, std::cout, copied);
            currentLog.insert(path);
        }
    }
    for (const auto & path: lastLog) {
        logOperation(path, myFile, removed);
        logOperation(path, std::cout, removed);
    }
    myFile << "\n";
    myFile.close();
    std::cout << "\n";
    lastLog = currentLog;
}

void synchronize(const std::string & source, const std::string & replica, int interval, const std::string & log) {
    std::set<std::string> lastLog;
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(interval));
        removeFolderContent(replica);
        try
        {
            fs::copy(source, replica, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
            logInfo(source, log, lastLog);
        }
        catch (std::exception& e)
        {
            std::cout << e.what();
        }
    }
}

int main() {
    std::string source, replica, log;
    int interval;
    if (!readInput(source, replica, interval, log)) {
        std::cout << "Bad input." << std::endl;
        return 0;
    }
    synchronize(source, replica, interval, log);
}
