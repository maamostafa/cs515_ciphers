#include <iostream>       // Provides input/output streams such as cout
#include <string>         // Provides the string class
#include <unordered_map>  // Provides unordered_map for storing command-line options
#include <algorithm>      // Provides algorithms such as find, transform, remove
#include <fstream>        // Provides file input/output streams
#include <sstream>        // Provides stringstream for reading whole files into strings
#include <vector>

using namespace std;

// Parses command-line arguments into a key-value map.
// Flags such as --action encrypt are stored as:
//   "--action" -> "encrypt"
// Flags without values such as --remove-spaces are stored as:
//   "--remove-spaces" -> ""
// The first non-flag argument is treated as direct input text.
unordered_map<string, string> parseArgs(int argc, char **argv);

// Decrypts text using the Rail Fence Cipher.
// The key represents the number of rails used to reconstruct
// the original zigzag pattern and recover the plaintext.
string decrypt(string input, int key);

// Encrypts text using the Rail Fence Cipher.
// The key represents the number of rails used to arrange
// characters in a zigzag pattern before reading row by row.
string encrypt(string input, int key);

// Saves the final output text to a file.
// Returns true if the file was written successfully, false otherwise.
bool saveOutput(const string& output, const string& filePath);

// Reads the entire contents of a text file into the content string.
// Returns true if the file was read successfully, false otherwise.
bool readTextFile(const string& filePath, string& content);

// Displays usage instructions and available command-line options.
void showHelp();

// Displays the program version.
void showVersion();

// Displays detailed information about the program, course, and authors.
void showInfo();

// The Rail Fence Cipher does not use an alphabet lookup table.
// Characters are not shifted or replaced; they are only rearranged
// by placing them across multiple rails in a zigzag pattern.

int main(int argc, char **argv) {
    // Convert command-line arguments into an easy-to-use map.
    unordered_map<string, string> argsMap = parseArgs(argc, argv);

    // Handle help request early.
    // If the user only wants help, no encryption/decryption options are required.
    if (argsMap.find("--help") != argsMap.end() || argsMap.find("-h") != argsMap.end()) {
        showHelp();
        return 0;
    }

    // Handle version request early.
    if (argsMap.find("--version") != argsMap.end() || argsMap.find("-v") != argsMap.end()) {
        showVersion();
        return 0;
    }

    // Handle detailed information request early.
    if (argsMap.find("--info") != argsMap.end() || argsMap.find("-i") != argsMap.end()) {
        showInfo();
        return 0;
    }

    // This variable stores the text that will be encrypted or decrypted.
    // It can come either from a direct command-line argument or from a file.
    string input;

    // If the first command-line argument was plain text, use it as input.
    if (argsMap.find("input") != argsMap.end()) {
        input = argsMap["input"];
    }

    // If the user supplied --file or -f, read the input text from that file.
    // File input overrides direct text input if both are supplied.
    if (argsMap.find("--file") != argsMap.end() || argsMap.find("-f") != argsMap.end()) {
        string inputFile;

        // Support both long and short forms of the file option.
        if (argsMap.find("--file") != argsMap.end()) {
            inputFile = argsMap["--file"];
        } else {
            inputFile = argsMap["-f"];
        }

        // The file option requires a path after it.
        if (inputFile.empty()) {
            cout << "No input file specified\n";
            return 1;
        }

        // Attempt to read the input file.
        // If reading fails, stop the program with an error code.
        if (!readTextFile(inputFile, input)) {
            cout << "Failed to read input file\n";
            return 1;
        }
    }

    // The program cannot continue without input text.
    if (input.empty()) {
        cout << "No input text or file specified\n";
        return 1;
    }

    // If requested, remove all regular space characters from the output.
    // Note: this removes only ' ', not tabs or newlines.
    if (argsMap.find("--remove-spaces") != argsMap.end() || argsMap.find("-r") != argsMap.end()) {
        input.erase(remove(input.begin(), input.end(), ' '), input.end());
    }

    // The user must specify whether they want encryption or decryption.
    if (argsMap.find("--action") == argsMap.end() && argsMap.find("-a") == argsMap.end()) {
        cout << "No action specified\n";
        return 1;
    }

    // Store the requested action.
    // Valid values are:
    //   encrypt, en
    //   decrypt, de
    string action;

    // Prefer the long option when it has a value; otherwise use the short option.
    if (!argsMap["--action"].empty()) {
        action = argsMap["--action"];
    } else {
        action = argsMap["-a"];
    }

    // Convert the action to lowercase so input such as "Encrypt",
    // "ENCRYPT", or "encrypt" works the same way.
    transform(action.begin(), action.end(), action.begin(), ::tolower);

    // A Rail Fence Cipher requires a numeric key that controls
    // how many rails are used in the zigzag pattern.
    if (argsMap.find("--key") == argsMap.end() && argsMap.find("-k") == argsMap.end()) {
        cout << "No key specified\n";
        return 1;
    }

    // Convert the key argument from string to integer.
    int key;
    if (argsMap.find("--key") != argsMap.end()) {
        key = stoi(argsMap["--key"]);
    } else {
        key = stoi(argsMap["-k"]);
    }

    // This variable stores the encrypted or decrypted result.
    string output;

    // Perform the requested operation.
    if (action == "encrypt" || action == "en") {
        output = encrypt(input, key);
    } else if (action == "decrypt" || action == "de") {
        output = decrypt(input, key);
    } else {
        cout << "Invalid action specified\n";
        return 1;
    }

    // Optional output mode.
    // Supported values:
    //   c, capital  -> convert final output to uppercase
    //   s, small    -> convert final output to lowercase
    string mode;

    // Read mode from either the long or short option.
    if (!argsMap["--mode"].empty()) {
        mode = argsMap["--mode"];
    } else {
        mode = argsMap["-m"];
    }

    // Normalize mode to lowercase before validation.
    if (!mode.empty()) {
        transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
    }

    // Reject unsupported mode values.
    if (!mode.empty() && mode != "c" && mode != "s" && mode != "capital" && mode != "small") {
        cout << "Invalid mode specified\n";
        return 1;
    }

    // Apply case conversion to the final output if requested.
    if (mode == "c" || mode == "capital") {
        transform(output.begin(), output.end(), output.begin(), ::toupper);
    } else if (mode == "s" || mode == "small") {
        transform(output.begin(), output.end(), output.begin(), ::tolower);
    }

    // If an output file is specified, save the result to that file.
    // Otherwise, print the result to the console.
    if (argsMap.find("--output") != argsMap.end() || argsMap.find("-o") != argsMap.end()) {
        string outputFile;

        // Support both long and short output-file options.
        if (argsMap.find("--output") != argsMap.end()) {
            outputFile = argsMap["--output"];
        } else {
            outputFile = argsMap["-o"];
        }

        // The output option requires a file path.
        if (outputFile.empty()) {
            cout << "No output file specified\n";
            return 1;
        }

        // Attempt to save the output.
        if (!saveOutput(output, outputFile)) {
            cout << "Failed to save output to file\n";
            return 1;
        }

        cout << "Output saved to " << outputFile << endl;
    } else {
        // No output file was requested, so display the result in the terminal.
        cout << output << endl;
    }

    // Return 0 to indicate successful execution.
    return 0;
}

string decrypt(string input, int key) {
    // Rail Fence Cipher decryption:
    // During encryption, characters are written diagonally across several rows
    // and then read row by row. To decrypt, we reconstruct the same zigzag row
    // pattern, split the ciphertext back into rail rows, then read characters
    // following the zigzag pattern again.
    //
    // Example:
    //   Plaintext:  "HELLOWORLD"
    //   Key:        3
    //
    //   Encryption writes characters like this:
    //
    //     Rail 0: H   O   L
    //     Rail 1:  E L W R D
    //     Rail 2:   L   O
    //
    //   Reading row by row gives ciphertext:
    //     "HOL" + "ELWRD" + "LO" = "HOLELWRDLO"
    //
    //   Decryption receives:
    //     input = "HOLELWRDLO"
    //
    //   Step 1: Recreate the zigzag rail pattern for 10 characters:
    //     position: 0 1 2 3 4 5 6 7 8 9
    //     rail:     0 1 2 1 0 1 2 1 0 1
    //
    //   Step 2: Count how many characters belong to each rail:
    //     Rail 0 has 3 characters
    //     Rail 1 has 5 characters
    //     Rail 2 has 2 characters
    //
    //   Step 3: Split ciphertext using those counts:
    //     Rail 0 gets "HOL"
    //     Rail 1 gets "ELWRD"
    //     Rail 2 gets "LO"
    //
    //   Step 4: Read from these rails following the pattern:
    //     rail 0 -> H
    //     rail 1 -> E
    //     rail 2 -> L
    //     rail 1 -> L
    //     rail 0 -> O
    //     rail 1 -> W
    //     rail 2 -> O
    //     rail 1 -> R
    //     rail 0 -> L
    //     rail 1 -> D
    //
    //   Final plaintext:
    //     "HELLOWORLD"

    // If the key is 1 or less, there is only one rail.
    // No zigzag movement is possible, so the text remains unchanged.
    if (key <= 1) return input;

    // Store the total number of characters in the encrypted input.
    const int n = input.size();

    // pattern[i] stores which rail/row the original character at position i
    // would have belonged to during encryption.
    vector<int> pattern(n);

    // index represents the current rail while building the zigzag pattern.
    int index = 0;

    // down controls the direction of movement through the rails:
    // true  -> moving downward from rail 0 toward rail key - 1
    // false -> moving upward from rail key - 1 toward rail 0
    bool down = true;

    // Recreate the rail pattern used during encryption.
    for (int i = 0; i < n; i++) {
        // Save the rail number for this character position.
        pattern[i] = index;

        // If we reach the top rail, start moving downward.
        if (index == 0) down = true;

        // If we reach the bottom rail, start moving upward.
        else if (index == key - 1) down = false;

        // Move to the next rail according to the current direction.
        index += down ? 1 : -1;
    }

    // count[r] stores how many characters belong to rail r.
    // This is needed because the encrypted text is arranged row by row.
    vector<int> count(key, 0);

    // Count how many characters should be placed in each rail.
    for (const int r : pattern) count[r]++;

    // rows[r] will store the substring of encrypted text that belongs to rail r.
    vector<string> rows(key);

    // idx tracks the current position in the encrypted input while splitting it.
    int idx = 0;

    // Split the encrypted input into rail strings.
    // Since encryption reads rows from top to bottom, the ciphertext is already
    // grouped by rails. The count array tells us how many characters each rail has.
    for (int r = 0; r < key; r++) {
        rows[r] = input.substr(idx, count[r]);
        idx += count[r];
    }

    // pos[r] stores the next unread character index inside rail r.
    vector<int> pos(key, 0);

    // result will hold the decrypted original text.
    string result;

    // Rebuild the original text by following the zigzag pattern.
    // For each original character position, take the next character from the
    // corresponding rail.
    for (int i = 0; i < n; i++) {
        int r = pattern[i];
        result += rows[r][pos[r]++];
    }

    // Return the reconstructed plaintext.
    return result;
}

string encrypt(string input, int key) {
    // Rail Fence Cipher encryption:
    // Characters are written diagonally across a number of rows called rails.
    // After all characters are placed, the encrypted text is produced by reading
    // the rails from top to bottom.
    //
    // Example:
    //   Input: "HELLOWORLD"
    //   Key:   3
    //
    //   Step 1: Place characters in a zigzag pattern:
    //
    //     Rail 0: H   O   L
    //     Rail 1:  E L W R D
    //     Rail 2:   L   O
    //
    //   The character placement is:
    //     H -> rail 0
    //     E -> rail 1
    //     L -> rail 2
    //     L -> rail 1
    //     O -> rail 0
    //     W -> rail 1
    //     O -> rail 2
    //     R -> rail 1
    //     L -> rail 0
    //     D -> rail 1
    //
    //   Step 2: Read each rail from top to bottom:
    //     Rail 0: "HOL"
    //     Rail 1: "ELWRD"
    //     Rail 2: "LO"
    //
    //   Step 3: Join the rails:
    //     "HOL" + "ELWRD" + "LO" = "HOLELWRDLO"
    //
    //   Final ciphertext:
    //     "HOLELWRDLO"

    // If the key is 1 or less, there is only one rail.
    // The input cannot be rearranged, so return it unchanged.
    if (key <= 1) return input;

    // Create one string for each rail.
    // Each character from the input will be appended to one of these rows.
    vector<string> rows(key);

    // row stores the current rail where the next character will be placed.
    int row = 0;

    // down controls the direction of movement through the rails:
    // true  -> moving downward from the top rail to the bottom rail
    // false -> moving upward from the bottom rail to the top rail
    bool down = true;

    // Place every character into the correct rail following a zigzag pattern.
    for (char c : input) {
        // Add the current character to the current rail.
        rows[row] += c;

        // If we are at the top rail, the next movement must go downward.
        if (row == 0) down = true;

        // If we are at the bottom rail, the next movement must go upward.
        else if (row == key - 1) down = false;

        // Move to the next rail based on the current direction.
        row += down ? 1 : -1;
    }

    // result will contain the final encrypted text.
    string result;

    // Read the rails from top to bottom.
    // This produces the Rail Fence encrypted output.
    for (auto& r : rows) result += r;

    // Return the ciphertext.
    return result;
}

bool saveOutput(const string& output, const string& filePath) {
    // Open the output file for writing.
    // If the file already exists, it will be overwritten.
    ofstream file(filePath);

    // If the file could not be opened, report failure.
    if (!file.is_open()) {
        return false;
    }

    // Write the output text to the file.
    file << output;

    // Close the file explicitly.
    file.close();

    return true;
}

bool readTextFile(const string& filePath, string& content) {
    // Open the input file for reading.
    ifstream file(filePath);

    // If the file could not be opened, report failure.
    if (!file.is_open()) {
        return false;
    }

    // Read the entire file into a stringstream.
    stringstream buffer;
    buffer << file.rdbuf();

    // Store the file contents in the output reference parameter.
    content = buffer.str();

    // Close the file explicitly.
    file.close();

    return true;
}

unordered_map<string, string> parseArgs(int argc, char **argv) {
    // Stores parsed arguments as key-value pairs.
    unordered_map<string, string> argsMap;

    // Start at index 1 because argv[0] is the program name.
    for (int i = 1; i < argc; i++) {
        string key = argv[i];

        // Command-line options start with '-'.
        // Examples:
        //   -a
        //   --action
        //   --remove-spaces
        if (key.rfind('-', 0) == 0) {
            // Check if there is another argument after this option.
            if (i + 1 < argc) {
                string value = argv[i + 1];

                // If the next token is not another option,
                // treat it as the value for the current option.
                if (value.rfind('-', 0) != 0) {
                    argsMap[key] = value;

                    // Skip the value because it has already been consumed.
                    i++;
                } else {
                    // The option exists but has no value.
                    // This is useful for boolean flags such as --remove-spaces.
                    argsMap[key] = "";
                }
            } else {
                // The option is the last argument and has no value.
                argsMap[key] = "";
            }
        }

        // If the first user argument does not start with '-',
        // treat it as direct input text.
        if (i == 1 && key.rfind('-', 0) != 0) {
            argsMap["input"] = argv[i];
        }
    }

    return argsMap;
}

void showHelp() {
    // Print command-line usage information.
    cout << "Usage: rf [options]\n\n";
    cout << "Options:\n";
    cout << "  <input>                  Text to encrypt or decrypt\n";
    cout << "  -a, --action             Encrypt|en or decrypt|de <input> / <file>\n";
    cout << "  -k, --key                Number of rails used by the Rail Fence Cipher\n";
    cout << "  -m, --mode               capital|c or small|s, changes output letter case\n";
    cout << "  -r  --remove-spaces      Remove white spaces from output\n";
    cout << "  -f  --file <file>        Read input from file\n";
    cout << "  -o  --output <file>      Save output to file\n";
    cout << "  -h, --help               Show this help message\n";
    cout << "  -v, --version            Show version\n";
    cout << "  -i, --info               Show detailed program information\n";
}

void showVersion() {
    // Print the current program version.
    cout << "rf v.0.0.1\n";
}

void showInfo() {
    // Print detailed information about the program.
    cout << "==================================================\n";
    cout << "           Rail Fence Cipher CLI Tool (v0.0.1)\n";
    cout << "==================================================\n\n";

    cout << "Description:\n";
    cout << "  A command-line utility for encrypting and decrypting\n";
    cout << "  text using the classical Rail Fence Cipher algorithm.\n";
    cout << "  The Rail Fence Cipher writes text in a zigzag pattern\n";
    cout << "  across multiple rows, then reads the rows from top to\n";
    cout << "  bottom to produce the encrypted message.\n\n";

    cout << "Supported Features:\n";
    cout << "  - Encrypt text using the Rail Fence Cipher\n";
    cout << "  - Decrypt Rail Fence encrypted text\n";
    cout << "  - Read input from command-line text or from a file\n";
    cout << "  - Save output to a file\n";
    cout << "  - Convert output to uppercase or lowercase\n";
    cout << "  - Remove spaces from the final output\n\n";

    cout << "Course:\n";
    cout << "  CS515 (2025-2026)\n\n";

    cout << "Supervisor:\n";
    cout << "  Dr. Nermin Hamza\n\n";

    cout << "Authors:\n";
    cout << "  Ahmed Gameil\n";
    cout << "  Mohammed Mostafa\n\n";

    cout << "Acknowledgment:\n";
    cout << "  Special thanks to our supervisor for guidance and support.\n\n";

    cout << "Build Date:\n";
    cout << "  May 02, 2026\n";

    cout << "==================================================\n";
}
