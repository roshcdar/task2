# Program that synchronizes two folders

Description:

Program, that synchronizes two folders: source and replica. Synchronization is one-way. 
After the synchronization content of the replica folder is exactly matched the content of the source folder.
Synchronization is performed periodically. File creation/copying/removal operations in source file are logged to file and to the console output.

Source folder path, replica folder path, synchronization interval and log file path are provided by command line arguments. 
Synchronization is quited only if user enters quit. 

Contents:

It contains src folder with source code, CMakeLists.txt and compiled code runnable on Linux in task2 file.
