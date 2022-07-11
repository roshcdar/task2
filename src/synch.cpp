#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <set>

namespace fs = std::filesystem;

/**
 * Check if string represents positive integer or not.
 *
 * @param[in] str   string to check
 * @returns true if str represents integer, false otherwise
 */
bool isPositiveInteger(const std::string & str) {
    if (str.empty())
        return false;
    std::string::const_iterator it = str.begin();
    while (it != str.end()) {
        if (!std::isdigit(*it))
            return false;
        ++it;
    }
    return true;
}

/**
 * Read source folder path, replica folder path, synchronization interval, log file path from command line arguments.
 *
 * Source folder path, replica folder path, synchronization interval and log file path are read from command line arguments.
 * Reading is not successful:
 * - if command line arguments are less than 5 ( folder paths + synchronization interval + log file path and name of the program),
 * - if source folder path is not a folder path,
 * - if synchronization interval is not a valid integer or is less than 0
 *
 * @param[out] source   source folder path
 * @param[out] replica  replica folder path
 * @param[out] interval synchronization interval
 * @param[out] log      log file path
 * @returns true if reading is successful, false otherwise
 */
bool readArguments(int argc, char ** argv, std::string & source, std::string & replica, int & interval, std::string & log) {
    if (argc < 5)
        return false;
    source = argv[1];
    if (!fs::is_directory(source))
        return false;
    replica = argv[2];
    if (!isPositiveInteger(argv[3]))
        return false;
    std::stringstream intervalString(argv[3]);
    intervalString >> interval;
    log = argv[4];
    return true;
}

/**
 * Remove all content of a folder.
 *
 * @param[in] folder folder path
 */
void removeFolderContent(const std::string& folder)
{
    for (const auto& entry : fs::directory_iterator(folder))
        fs::remove_all(entry.path());
}

/**
 * Enum class representing operations, which are performed on a file.
 */
enum operationType{created, removed, copied};

/**
 * Log a operation, which is performed on a file.
 *
 * Operation, which is performed on a file, is logged to output stream.
 *
 * @param[in] path          file path
 * @param[out] os           output stream
 * @param[in] operationType operation, which is performed on a file
 */
void logOperation(const std::string & path,  std::ostream & os, operationType operationType) {
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

/**
 * Synchronize two folders: source and replica and log operations performed on files in source folder.
 *
 * Source and replica folders are periodically synchronized. The synchronization is one-way.
 * After the synchronization content of the replica folder is  exactly matched content of the source folder.
 * The information about what files were copied from source folder to replica folder during synchronization is logged to
 * a log file and standard output. The information about what files were created and removed during synchronization interval
 * is logged to a log file and standard output too.
 * Synchronization is quited, when signal to quit is true.
 *
 * @param[in] source    source file path
 * @param[in] replica   replica file path
 * @param[in] interval  synchronization interval
 * @param[in] log       log file path
 * @param[in] quit      signal to quit
 * @returns true if synchronization is successfully quited, false otherwise
 */
bool synchronize(const std::string & source, const std::string & replica, int interval, const std::string & log, const bool & quit) {
    std::ofstream logFile;
    logFile.open(log);
    if (!logFile.is_open())
        return false;
    /// Paths to files, which were copied during last synchronization.
    std::set<std::string> lastLog;
    /// Paths to files, which are copied during current synchronization.
    std::set<std::string> currentLog;
    std::set<std::string>::iterator it;
    std::string path;
    while(!quit) {
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
        if (!lastLog.empty() || !currentLog.empty()) {
            logFile << std::endl;
            std::cout << std::endl;
            lastLog = currentLog;
            currentLog.clear();
        }
    }
    logFile.close();
    return true;
}

/**
 * Wait until user enters quit. After quit is entered, signal to quit is set on true.
 *
 * @param[out] quit   signal to quit
 */
void waitForQuit(bool & quit) {
    std::string line;
    while (getline(std::cin, line)) {
        if (line == "quit") {
            quit = true;
            break;
        }
    }
}

int main(int argc, char * argv[]) {
    std::string source, replica, log;
    int interval;
    if (!readArguments(argc, argv, source, replica, interval, log)) {
        std::cout << "Bad input." << std::endl;
        return 0;
    }
    if (!fs::is_directory(replica))
        fs::create_directories(replica);
    bool quit = false;
    std::thread t1(waitForQuit, std::ref(quit));
    if (!synchronize(source, replica, interval, log, quit))
        std::cout << "Error opening file. Enter quit to end." << std::endl;
    t1.join();
}
