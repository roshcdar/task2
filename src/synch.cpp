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
    if (!(std::cin >> source) || !fs::is_directory(source))
        return false;
    std::cout << "Enter replica folder path:" << std::endl;
    if (!(std::cin >> replica))
        return false;
    std::cout << "Enter synchronization interval in seconds:" << std::endl;
    if (!(std::cin >> interval || interval < 0))
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
            os << path + " is created." << std::endl;
            break;
        }
        case copied: {
            os << path + " is copied." << std::endl;
            break;
        }
        case removed: {
            os << path + " is removed." << std::endl;
            break;
        }
    }
}

void synchronize(const std::string & source, const std::string & replica, int interval, const std::string & log, bool & quiet) {
    std::set<std::string> lastLog, currentLog;
    std::set<std::string>::iterator it;
    std::string path;
    std::ofstream logFile;
    logFile.open(log);
    if (!logFile.is_open()) {
        std::cout << "Error opening file. Enter quiet to end." << std::endl;
        return;
    }
    while(!quiet) {
        std::this_thread::sleep_for(std::chrono::seconds(interval));
        removeFolderContent(replica);
        fs::copy(source, replica, fs::copy_options::overwrite_existing | fs::copy_options::directories_only | fs::copy_options::recursive);
        for (const auto& entry : fs::recursive_directory_iterator(source)) {
            if (!is_directory(entry.path())) {
                fs::copy(source, replica, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
                if (is_regular_file(entry.path())) {
                    path = fs::relative(entry.path(), source).generic_string();
                    it = lastLog.find(path);
                    if (it == lastLog.end()) {
                        logOperation(path, logFile, created);
                        logOperation(path, std::cout, created);
                    }
                    else
                        lastLog.erase(it);
                    logOperation(path, logFile, copied);
                    logOperation(path, std::cout, copied);
                    currentLog.insert(path);
                }
            }
        }
        for (const auto & str: lastLog) {
            logOperation(str, logFile, removed);
            logOperation(str, std::cout, removed);
        }
        logFile << std::endl;
        std::cout << std::endl;
        lastLog = currentLog;
        currentLog.clear();
    }
    logFile.close();
}

void waitForEnd(bool & quiet) {
    std::string line;
    while (getline(std::cin, line)) {
        if (line == "quiet") {
            quiet = true;
            break;
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
    if (!fs::is_directory(replica))
        fs::create_directories(replica);
    bool quiet = false;
    std::thread t1(waitForEnd, std::ref(quiet));
    synchronize(source, replica, interval, log, quiet);
    t1.join();
}
