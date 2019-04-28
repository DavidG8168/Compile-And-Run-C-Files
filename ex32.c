#define _GNU_SOURCE

#include <stdio.h>  // All needed libraries.
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include "string.h"
#include <fcntl.h>

#define SIZE 1024;  // Define the buffer size.
int resultsFD;  // Global file descriptor.

// ************************************ Function Declarations *********************************************************
// *****************

void startProgram(const char *name, char *input, char *output);  // Begins reading and writing all files in given path.
char** getProgramArgs(char *path);  // Get the arguments for the execvp funcs.
int checkIfCFile(char* name);  // Checks if string is a C file or not.
int checkForCFiles(const char* name);  // Checks directory for a C file.
char** splitString(const char* s, const char* delim); // Split a string by the delim character.
int CallEX31(char *argv[]);  // ex31 in a function.
int getLength(const char *string);  // Gets the length of the string.
char* fixString(char *input);  // Removes spaces and newlines from a string.
int compareEqualOrSimilar(const char *str1, const char *str2);  // Compares if strings are equal or similar.
int compareEqual(const char *str1, const char *str2);  // Compares if strings are equal.

// *****************
// ************************************ Main Function *****************************************************************
// *****************

int main(int argc, char *argv[]) {  // Main accepts path to configuration file as command line argument.
    int size = SIZE;  // Will be used to remove newline at the end.
    char buff[size];
    int i = 0;
    for (i = 0 ; i < size; i ++) {
        buff[i] = '\0';
    }
    if ((resultsFD = open("results.csv", O_WRONLY | O_CREAT | O_APPEND|O_TRUNC,0777)) < 0) {  // Open a results file.
        fprintf(stderr, "Error in system call.\n");
        exit(EXIT_FAILURE);  // Exit.
    }
    char** args = getProgramArgs(argv[1]);  // Get the arguments from the file in the command line path.
    startProgram(args[0], args[1], args[2]);  // Iterate the directory, and run all C files in the sub-directories.
    int fdir = open("results.csv", O_RDONLY);  // Open the result file for reading.
    if (fdir < 0) {   // Handle opening error.
        fprintf(stderr, "Error in system call.\n");
        exit(EXIT_FAILURE);
    }
    i = 0;  // Index.
    while (read(fdir, &buff[i], 1) == 1) {  // Read first file into buffer.
        i++;
    }
    i = 0; // Removing newlinea t the end.
    for (i = 0 ; i < size - 1; i ++) {
        if (buff[i] == '\n' && buff[i+1] == '\0') {
            buff[i] = '\0';
            break;
        }
    }
    if ((resultsFD = open("results.csv", O_WRONLY | O_CREAT | O_APPEND|O_TRUNC,0777)) < 0) {  // Open a results file.
        fprintf(stderr, "Error in system call.\n");
        exit(EXIT_FAILURE);  // Exit.
    }
    write(resultsFD,buff,strlen(buff));  // Write using the global descriptor.
    return 0;  // Finish.
}

// *****************
// ************************************ Implementation ****************************************************************
// *****************

char** getProgramArgs(char* path) {  // Get the program arguments from the input file in the given path.
    int size = SIZE;  // Get the buffer size from the macro,
    char** args;
    char buff1[size];  // Create the buffers to read from the files.
    int k = 0;  // Buffer initialization index.
    for (k = 0; k < size; k++) {  // Initialize the buffers and copy char arrays.
        buff1[k] = '\0';
    }
    int fdin1; // Create the input file descriptors.
    fdin1 = open(path, O_RDONLY);  // Open the first file for reading.
    if (fdin1 < 0) {   // Handle opening error.
        fprintf(stderr, "Error in system call.\n");
        exit(EXIT_FAILURE);  // Exit.
    }
    int i = 0;  // Index for first buffer reading.
    while (read(fdin1, &buff1[i], 1) == 1) {  // Read first file into buffer.
        i++;
    }
    args = splitString(buff1, "\n");  // Split the configuration file by newlines to get each of the three rows.
    return args;  // Return the arguments.
}

// *****************

void GetCommands(char* path, char inputArgs[1024][1024]) {  // Get arguments for the C files.
    int size = SIZE;  // Get the buffer size from the macro,
    char buff1[size];  // Create the buffers to read from the files.
    int k = 0;  // Buffer initialization index.
    for (k = 0; k < size; k++) {  // Initialize the buffers and copy char arrays.
        buff1[k] = '\0';
    }
    int fdin1; // Create the input file descriptors.
    fdin1 = open(path, O_RDONLY);  // Open the first file for reading.
    if (fdin1 < 0) {   // Handle opening error.
        fprintf(stderr, "Error in system call.\n");
        exit(EXIT_FAILURE);
    }
    int i = 0;  // Index.
    while (read(fdin1, &buff1[i], 1) == 1) {  // Read first file into buffer.
        i++;
    }
    char* string[1024];
    char delimit[] = " \n"; // The characters we will split the parameters from the input file by.
    i = 0;
    string[i] = strtok(buff1, delimit);
    while (string[i] != NULL) {
        i++;
        string[i] = strtok(NULL, delimit);
    }
    for (i = 0; i < 1024; i++) {  // Copy the parameters to the char array we will use from the splitted string.
        if (string[i] == NULL) {
            break;
        }
        strcpy(inputArgs[i], string[i]);
    }
}

// *****************

int checkForCFiles(const char* name) {  // Checks if the folder contains a C file.
    DIR *dir;  // The directory.
    struct dirent *entry;  // The entry in the directory.
    if (!(dir = opendir(name)))  // If we can't open the directory stop.
        return 0;
    while ((entry = readdir(dir)) != NULL) {  // Check all entries in the directory.
        if (checkIfCFile(entry->d_name)) {
            return 1;
        }
    }
    return 0;
}

// *****************

void startProgram(const char* name, char* input, char* output) {  // Lists files in the directory and executes C files.
    DIR* dir;  // The directory.
    struct dirent* entry;  // The entry in the directory.
    if (!(dir = opendir(name)))  // If we can't open the directory stop.
        return;
    while ((entry = readdir(dir)) != NULL) {  // Check all entries in the directory.
        if (entry->d_type == DT_DIR) {  // If the entry is a sub-directory.
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            if (!checkForCFiles(path)) {  // Writes and skips folders with no C files.
                char  fileWriter[1024];  // Initialize the writing to file buffer.
                int j = 0;
                for (j = 0; j < 1024 ; ++j) {
                    fileWriter[j] = 0;
                }
                printf("No C files found in this directory.\n");
                strcat(fileWriter, entry->d_name);  // Build the results string.
                strcat(fileWriter,",");
                strcat(fileWriter, "0,NO_C_FILE\n");
                write(resultsFD,fileWriter,strlen(fileWriter));  // Write using the global descriptor.
                continue;
            }
            startProgram(path, input, output);  // Go in recursively to iterate all entries.
        } else {  // Otherwise the entry is a file, and not a directory.
            if (checkIfCFile(entry->d_name)) {  // Check if the file is a C file.
                char inputArgs[1024][1024];  // Get commands for C file from the input file given in the config file.
                char* aoutArgs[1024];
                char* gccArgs[1024];
                GetCommands(input, inputArgs);  // Getting the commands and placing them in the char** array.
                int i = 0;  // Build the parameters for execvp command.
                gccArgs[i] = "gcc";  // The command for gcc compilation.
                gccArgs[i + 1] = entry->d_name;
                gccArgs[i + 2] = NULL;
                char cwd[1024];  // Get the current work directory.
                getcwd(cwd, sizeof(cwd));
                char* prev = cwd;  // Set the previous as the current, since we will enter the given path to compile.
                chdir(name);  // Enter the path with the C file.
                char pathBuffer[1024];  // Get the name of the folder.
                strcpy(pathBuffer, name);
                char** shortName;
                shortName = splitString(pathBuffer, "/");  // Split by "/" characters.
                int l = 0;  // Get the index that is the actual name.
                while(1) {
                    if (shortName[l] == 0 ) {
                        break;
                    }
                    l++;
                }
                pid_t pid = fork();  // Fork to create a child process to run the command.
                if (pid == -1) {  // Check if fork() has filed.
                    fprintf(stderr, "Error in system call.\n");
                    exit(EXIT_FAILURE); // Exit.
                }
                else if (pid == 0) {  // If the fork() works, the child will execute the gcc command.
                    if (execvp("gcc", gccArgs) == -1) {  // Check if gcc failed.
                        fprintf(stderr, "Error in system call.\n");
                        char  fileWriter[1024];  // Write name and result error in the results.csv file.
                        int j = 0;
                        for (j = 0; j < 1024 ; ++j) {
                            fileWriter[j] = 0;
                        }
                        strcat(fileWriter,shortName[l - 1]);  // Create the string that needs to be written.
                        strcat(fileWriter,",");
                        strcat(fileWriter,"20,COMPILATION_ERROR\n");
                        write(resultsFD,fileWriter,strlen(fileWriter));  // Write using the global descriptor.
                        exit(EXIT_FAILURE);  // Exit child due to failure.
                    }
                } else {
                    wait(NULL); // Otherwise, the original process waits for the child to finish.
                }
                aoutArgs[0] = "./a.out";  // Create the arguments for the ./a.out command.
                for (i = 1; i < 1024; ++i) {  // Move from char** array into char* array.
                    if (inputArgs[i - 1][0] == '\0') {
                        break;
                    }
                    aoutArgs[i] = inputArgs[i - 1];
                }
                aoutArgs[i + 1] = NULL;  // Set the last arguments as NULL for execvp.
                pid_t pid2 = fork();  // Fork to create a child process to run the ./a.out command.
                if (pid2 == -1) {  // Handle fork() failure.
                    fprintf(stderr, "Error in system call.\n");
                    exit(EXIT_FAILURE);  // Exit.
                } else if (pid2 == 0) {  // If child process, we will run the command.
                    int newfd,infd; // Create a new file descriptor.
                    if ((newfd = open("ProgOutput", O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) { // Create output file.
                        fprintf(stderr, "Error in system call.\n");
                        exit(EXIT_FAILURE);  // Close child process.
                    }
                    if ((infd = open(input, O_RDONLY)) < 0) {
                        fprintf(stderr, "Error in system call.\n");
                        exit(EXIT_FAILURE);
                    }
                    dup2(newfd, 1);  // Used to write results to the file instead of the user screen.
                    dup2(infd, 0);  // Fixes problem with input to scanf.
                    if (execvp("./a.out", aoutArgs) == -1) {  // Run the ./a.out command and handle any errors.
                        char  fileWriter[1024];  // Write a compilation error if failed to run the ./a.out command.
                        int j = 0;
                        for (j = 0; j < 1024 ; ++j) {
                            fileWriter[j] = 0;
                        }
                        strcat(fileWriter,shortName[l - 1]);
                        strcat(fileWriter,",");
                        strcat(fileWriter,"20,COMPILATION_ERROR\n");
                        write(resultsFD,fileWriter,strlen(fileWriter));
                        printf("$*$");
                        fprintf(stderr, "Error in system call.\n");
                        exit(EXIT_FAILURE);  // Close child process if execvp failed.
                    }
                } else {
                    printf("Waiting 5 seconds for C program to execute.\n");
                    int timeout = 0;
                    sleep(7);  // Wait 5 seconds for child to finish, if didn't then it's a timeout.
                    int status;  // Status int.
                    if (waitpid(pid2, &status, WNOHANG) == 0) {  // If child is still running.
                        timeout = 1;
                        printf("Execution did not finish, TIMEOUT.\n");
                        char fileWriter[1024];  // Initialize the writing to file buffer.
                        int j = 0;
                        for (j = 0; j < 1024; ++j) {
                            fileWriter[j] = 0;
                        }
                        strcat(fileWriter, shortName[l - 1]);
                        strcat(fileWriter, ",");
                        strcat(fileWriter, "40,TIMEOUT\n");
                        write(resultsFD, fileWriter, strlen(fileWriter));
                    }
                    printf("Finished executing C file.\n");
                    int m = 0;
                    char buff1[1024];  // Create the buffers to read from the progout file.
                    int fdin1 = open("ProgOutput", O_RDONLY);  // Open the progout file for reading.
                    if (fdin1 < 0) {   // Handle opening error.
                        fprintf(stderr, "Error in system call.\n");
                        exit(EXIT_FAILURE);
                    }
                    while (read(fdin1, &buff1[m], 1) == 1) {  // Read file into buffer.
                        m++;
                    }
                    int compError = 0; // Check if compilation error identifier is present.
                    if (buff1[0] == '$' && buff1[1] == '*' && buff1[2] == '$') {
                        compError = 1;
                    }
                    if (timeout == 0  && compError == 0) {  // If time out and no comp error to the results.csv file.
                        printf("Finished executing C file.\n");
                        char* fileComparator[3];  // This will be used to pass arguments to the file comparison.
                        fileComparator[1] = "ProgOutput";  // Set the first argument to the output file of each C file.
                        fileComparator[2] = output;  // Set the second argument to the correct output file.
                        int result = CallEX31(fileComparator);  // Get the result from the file comparison.
                        char  fileWriter[1024];  // Initialize the writing to file buffer.
                        int j = 0;
                        for (j = 0; j < 1024 ; ++j) {
                            fileWriter[j] = 0;
                        }
                        strcat(fileWriter,shortName[l - 1]);  // Build the results string.
                        strcat(fileWriter,",");
                        if (result == 1) {  // Good output gets a 100 grade.
                            strcat(fileWriter, "100,GREAT_JOB\n");
                            write(resultsFD, fileWriter, strlen(fileWriter));  // Write to result file.
                        }
                        if (result == 2) {  // Bad output gets a 60 grade.
                            strcat(fileWriter, "60,BAD_OUTPUT\n");
                            write(resultsFD, fileWriter, strlen(fileWriter));
                        }
                        if (result == 3) {  // Similar output gets a 80 grade.
                            strcat(fileWriter, "80,SIMILAR_OUTPUT\n");
                            write(resultsFD, fileWriter, strlen(fileWriter));
                        }
                    }
                    // Delete files we created.
                    unlink("a.out");
                    unlink("ProgOutput");
                    chdir(prev);  // Once we're done checking for C files, return the previous directory.
                }
            }
        }
    }
    closedir(dir);  // Close directory.
}

// *****************

int checkIfCFile(char* name) {  // Checks if a given file is a C type file.
    int size = getLength(name);
    if (size < 3) {
        return 0;
    }
    if (name[size - 2] == '.' && name[size - 1] == 'c') {
        return 1;
    } else {
        return 0;
    }

}

// *****************

char** splitString(const char* s, const char* delim) {  // Split string by delim character.
    void *data;
    char* _s = (char*) s;
    const char** ptrs;
    unsigned int
            ptrsSize,
            nbWords = 1,
            sLen = strlen(s),
            delimLen = strlen(delim);

    while ((_s = strstr(_s, delim))) {
        _s += delimLen;
        ++nbWords;
    }
    ptrsSize = (nbWords + 1) * sizeof(char*);
    ptrs =
    data = malloc(ptrsSize + sLen + 1);
    if (data) {
        *ptrs =
        _s = strcpy(((char*) data) + ptrsSize, s);
        if (nbWords > 1) {
            while ((_s = strstr(_s, delim))) {
                *_s = '\0';
                _s += delimLen;
                *++ptrs = _s;
            }
        }
        *++ptrs = NULL;
    }
    return data;
}

// *****************
// ************************************************* EX31 *************************************************************
// *****************

int CallEX31(char *argv[]) {
    // Ex31 in a function.
    int size = SIZE;  // Get the buffer size from the macro,
    char buff1[size];  // Create the buffers to read from the files.
    char buff2[size];
    char copy1[size];  // Used to copy the buffers for removing the spaces and newlines.
    char copy2[size];
    int k = 0;  // Buffer initialization index.
    for (k = 0; k < size; k++) {  // Initialize the buffers and copy char arrays.
        buff1[k] = '\0';
        buff2[k] = '\0';
        copy1[k] = '\0';
        copy2[k] = '\0';
    }
    int fdin1; // Create the input file descriptors.
    int fdin2;
    char *path1 = argv[1];  // Move the file paths from the command line to local variables.
    char *path2 = argv[2];
    fdin1 = open(path1, O_RDONLY);  // Open the first file for reading.
    if (fdin1 < 0) {   // Handle opening error.
        fprintf(stderr, "Error in system call.\n");
        exit(EXIT_FAILURE);
    }
    fdin2 = open(path2, O_RDONLY);  // Open second file.
    if (fdin2 < 0) {  // Handle opening error.
        fprintf(stderr, "Error in system call.\n");
        exit(EXIT_FAILURE);
    }
    int i = 0;  // Index for first buffer reading.
    while (read(fdin1, &buff1[i], 1) == 1) {  // Read first file into buffer.
        i++;
    }
    int j = 0;  // Index for second buffer reading.
    while (read(fdin2, &buff2[j], 1) == 1) {  // Read second file into buffer.
        j++;
    }
    int result = 1;  // Will be the result value.
    snprintf(copy1, 1024, "%s", buff1);  // Copy the buffers for further use.
    snprintf(copy2, 1024, "%s", buff2);
    char *fixed1 = fixString(copy1);  // Keep the fixed strings after removing the spaces and newlines.
    char *fixed2 = fixString(copy2);
    if (getLength(fixed1) != getLength(fixed2)) { // If the fixed lengths are different they are different
        result = 2;
    } else {
        if (compareEqual(fixed1, fixed2) == 1) {  // If they have no different characters we suspect they are similar.
            result = compareEqualOrSimilar(buff1, buff2);  // Otherwise compare to see if similar or equal.
        } else {
            result = 2;  // Otherwise they are different.
        }
    }
    if (close(fdin1) < 0) {  // Close first file and handle error.
        fprintf(stderr, "Error in system call.\n");
        exit(EXIT_FAILURE);
    }
    if (close(fdin2) < 0) {  // Close second file and handle error.
        fprintf(stderr, "Error in system call.\n");
        exit(EXIT_FAILURE);
    }
    return result;  // Return the final result.
}

// *****************

int getLength(const char *string) {  // Get the length of the string.
    int i = 0;
    int count = 0;
    while (string[i] != '\0') {  // Count the characters until null terminator is reached.
        count++;
        i++;
    }
    return count;
}

// *****************

char *fixString(char *input) {  // Remove spaces and newlines.
    int i, j;
    char *output = input;
    for (i = 0, j = 0; i < getLength(input); i++, j++) {  // Don't insert the spaces and newlines into the output.
        if (input[i] != ' ' && input[i] != '\n')
            output[j] = input[i];
        else
            j--;
    }
    output[j] = 0;
    return output;
}

// *****************

int compareEqualOrSimilar(const char *str1, const char *str2) { // Return 3 if strings are mismatched otherwise 1.
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] + 32 == str2[i] || str1[i] - 32 == str2[i]) {  // If they are uppercased or lowercased.
            return 3;
        }
        if (str1[i] == ' ' && str2[i] != ' ') {  // If one has a space and the other does not.
            return 3;
        }
        if (str1[i] != ' ' && str2[i] == ' ') {
            return 3;
        }
        if (str1[i] == '\n' && str2[i] != '\n') {  // If one has a newline and the other does not.
            return 3;
        }
        if (str1[i] != '\n' && str2[i] == '\n') {
            return 3;
        }
        if (str1[i + 1] == '\0' && str2[i + 1] != '\0') {  // If one has ended and the other has not.
            return 3;
        }
        if (str1[i + 1] != '\0' && str2[i + 1] == '\0') {
            return 3;
        }
        i++;
    }
    return 1;  // If no mismatches occurred, return 1 as the strings are equal.
}

// *****************

int compareEqual(const char *str1, const char *str2) {  // Compare strings to see if they are exactly equal.
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {  // Iterate the characters until null terminator is reached.
        if (str1[i] != str2[i]) {
            if ((str1[i] + 32) != str2[i] && (str1[i] - 32) != str2[i]) {  // If not different by lower or upper case
                return 2;  // The strings are different and we return 2.
            }
        }
        i++;
    }
    return 1;
}

// *****************
// ********************************************************************************************************************
