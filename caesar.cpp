#include <iostream>       // Console input/output
#include <string>         // std::string
#include <unordered_map>  // Stores parsed command-line options
#include <algorithm>      // find, transform, remove
#include <fstream>        // File input/output
#include <sstream>        // Reads full file content into a string

using namespace std;

// Parses command-line arguments into option/value pairs.
// Example: --action encrypt becomes {"--action", "encrypt"}.
// Boolean flags such as --remove-spaces are stored with an empty value.
unordered_map<string, string> parseArgs(int argc, char **argv);

// Decrypts text with a Caesar Cipher by shifting letters backward.
// Example: decrypt("Khoor, Zruog!", 3) returns "Hello, World!".
string decrypt(string input, int key);

// Encrypts text with a Caesar Cipher by shifting letters forward.
// Example: encrypt("Hello, World!", 3) returns "Khoor, Zruog!".
string encrypt(string input, int key);

// Saves output text to a file. Returns true on success.
bool saveOutput(const string& output, const string& filePath);

// Reads a full text file into content. Returns true on success.
bool readTextFile(const string& filePath, string& content);

// Prints command-line usage.
void showHelp();

// Prints the program version.
void showVersion();

// Prints program, course, and author information.
void showInfo();

// Caesar Cipher alphabet table.
// Letters are matched in uppercase, then converted back to lowercase if needed.
constexpr const char *ALPHABETS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int main(int argc, char **argv) {
    // Parse command-line options once for easy lookup.
    unordered_map<string, string> argsMap = parseArgs(argc, argv);

    // Help, version, and info do not require input, action, or key.
    if (argsMap.find("--help") != argsMap.end() || argsMap.find("-h") != argsMap.end()) {
        showHelp();
        return 0;
    }

    if (argsMap.find("--version") != argsMap.end() || argsMap.find("-v") != argsMap.end()) {
        showVersion();
        return 0;
    }

    if (argsMap.find("--info") != argsMap.end() || argsMap.find("-i") != argsMap.end()) {
        showInfo();
        return 0;
    }

    // Text to encrypt/decrypt, either from direct input or a file.
    string input;

    // Use the first plain argument as direct input text.
    if (argsMap.find("input") != argsMap.end()) {
        input = argsMap["input"];
    }

    // File input overrides direct input when --file or -f is used.
    if (argsMap.find("--file") != argsMap.end() || argsMap.find("-f") != argsMap.end()) {
        string inputFile;

        // Accept both long and short file options.
        if (argsMap.find("--file") != argsMap.end()) {
            inputFile = argsMap["--file"];
        } else {
            inputFile = argsMap["-f"];
        }

        // A file path is required after --file or -f.
        if (inputFile.empty()) {
            cout << "No input file specified\n";
            return 1;
        }

        // Stop if the input file cannot be read.
        if (!readTextFile(inputFile, input)) {
            cout << "Failed to read input file\n";
            return 1;
        }
    }

    // Encryption/decryption requires input text.
    if (input.empty()) {
        cout << "No input text or file specified\n";
        return 1;
    }

    // Action decides whether the Caesar Cipher encrypts or decrypts.
    if (argsMap.find("--action") == argsMap.end() && argsMap.find("-a") == argsMap.end()) {
        cout << "No action specified\n";
        return 1;
    }

    // Supported actions: encrypt/en and decrypt/de.
    string action;

    // Prefer --action when it has a value; otherwise use -a.
    if (!argsMap["--action"].empty()) {
        action = argsMap["--action"];
    } else {
        action = argsMap["-a"];
    }

    // Make action matching case-insensitive.
    transform(action.begin(), action.end(), action.begin(), ::tolower);

    // Caesar Cipher requires a numeric shift key.
    if (argsMap.find("--key") == argsMap.end() && argsMap.find("-k") == argsMap.end()) {
        cout << "No key specified\n";
        return 1;
    }

    // Convert the key from text to integer.
    int key;
    if (argsMap.find("--key") != argsMap.end()) {
        key = stoi(argsMap["--key"]);
    } else {
        key = stoi(argsMap["-k"]);
    }

    // Result after encryption or decryption.
    string output;

    // Run the requested Caesar Cipher operation.
    if (action == "encrypt" || action == "en") {
        output = encrypt(input, key);
    } else if (action == "decrypt" || action == "de") {
        output = decrypt(input, key);
    } else {
        cout << "Invalid action specified\n";
        return 1;
    }

    // Optional output case mode:
    // c/capital = uppercase, s/small = lowercase.
    string mode;

    // Read mode from --mode or -m.
    if (!argsMap["--mode"].empty()) {
        mode = argsMap["--mode"];
    } else {
        mode = argsMap["-m"];
    }

    // Make mode matching case-insensitive.
    if (!mode.empty()) {
        transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
    }

    // Reject unknown mode values.
    if (!mode.empty() && mode != "c" && mode != "s" && mode != "capital" && mode != "small") {
        cout << "Invalid mode specified\n";
        return 1;
    }

    // Apply optional output case conversion.
    if (mode == "c" || mode == "capital") {
        transform(output.begin(), output.end(), output.begin(), ::toupper);
    } else if (mode == "s" || mode == "small") {
        transform(output.begin(), output.end(), output.begin(), ::tolower);
    }

    // Remove regular spaces only; tabs and newlines are kept.
    if (argsMap.find("--remove-spaces") != argsMap.end() || argsMap.find("-r") != argsMap.end()) {
        output.erase(remove(output.begin(), output.end(), ' '), output.end());
    }

    // Save to a file when requested; otherwise print to the console.
    if (argsMap.find("--output") != argsMap.end() || argsMap.find("-o") != argsMap.end()) {
        string outputFile;

        // Accept both long and short output options.
        if (argsMap.find("--output") != argsMap.end()) {
            outputFile = argsMap["--output"];
        } else {
            outputFile = argsMap["-o"];
        }

        // A file path is required after --output or -o.
        if (outputFile.empty()) {
            cout << "No output file specified\n";
            return 1;
        }

        // Stop if the result cannot be saved.
        if (!saveOutput(output, outputFile)) {
            cout << "Failed to save output to file\n";
            return 1;
        }

        cout << "Output saved to " << outputFile << endl;
    } else {
        // Print result directly when no output file is selected.
        cout << output << endl;
    }

    // Program completed successfully.
    return 0;
}

string decrypt(string input, int key) {
    // Final decrypted result.
    // Example: decrypt("Khoor", 3) -> "Hello".
    string decrypted;

    // Process each character separately.
    // Letters shift backward; non-letters stay unchanged.
    // Example with key 3: "Khoor 123!" -> "Hello 123!".
    for (int i = 0; i < input.length(); i++) {
        // Keep the original character to restore lowercase when needed.
        // Example: 'K' remains uppercase, 'h' returns as lowercase.
        char original = input[i];

        // Use uppercase only for alphabet lookup.
        // Example: 'h' becomes 'H' before searching ALPHABETS.
        char c = toupper(original);

        // Find the character position in A-Z.
        auto it = find(ALPHABETS, ALPHABETS + 26, c);

        // Decrypt only alphabetic characters.
        if (it != ALPHABETS + 26) {
            // Convert the letter to a zero-based index.
            // Example: A = 0, B = 1, ..., K = 10.
            int index = distance(ALPHABETS, it);

            // Shift backward by key and wrap around the alphabet.
            // Example: K index 10, key 3 -> index 7 -> H.
            // Example: A index 0, key 3 -> index 23 -> X.
            char shifted = ALPHABETS[(index - key + 26) % 26];

            // Restore lowercase if the input letter was lowercase.
            // Example: 'k' with key 3 becomes 'h', not 'H'.
            if (islower(original)) {
                shifted = tolower(shifted);
            }

            // Add decrypted letter to the result.
            decrypted += shifted;
        } else {
            // Keep spaces, digits, punctuation, and newlines unchanged.
            // Examples: ' ', '5', '!', '\n'.
            decrypted += original;
        }
    }

    return decrypted;
}

string encrypt(string input, int key) {
    // Final encrypted result.
    // Example: encrypt("Hello", 3) -> "Khoor".
    string encrypted;

    // Process each character separately.
    // Letters shift forward; non-letters stay unchanged.
    // Example with key 3: "Hello 123!" -> "Khoor 123!".
    for (int i = 0; i < input.length(); i++) {
        // Keep the original character to restore lowercase when needed.
        // Example: 'H' remains uppercase, 'e' returns as lowercase.
        char original = input[i];

        // Use uppercase only for alphabet lookup.
        // Example: 'e' becomes 'E' before searching ALPHABETS.
        char c = toupper(original);

        // Find the character position in A-Z.
        auto it = find(ALPHABETS, ALPHABETS + 26, c);

        // Encrypt only alphabetic characters.
        if (it != ALPHABETS + 26) {
            // Convert the letter to a zero-based index.
            // Example: A = 0, B = 1, ..., H = 7.
            int index = distance(ALPHABETS, it);

            // Shift forward by key and wrap around the alphabet.
            // Example: H index 7, key 3 -> index 10 -> K.
            // Example: Z index 25, key 3 -> index 2 -> C.
            char shifted = ALPHABETS[(index + key + 26) % 26];

            // Restore lowercase if the input letter was lowercase.
            // Example: 'e' with key 3 becomes 'h', not 'H'.
            if (islower(original)) {
                shifted = tolower(shifted);
            }

            // Add encrypted letter to the result.
            encrypted += shifted;
        } else {
            // Keep spaces, digits, punctuation, and newlines unchanged.
            // Examples: ' ', '5', '!', '\n'.
            encrypted += original;
        }
    }

    return encrypted;
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
    // Print command-line usage and supported options.
    cout << "Usage: csr [options]\n\n";
    cout << "Options:\n";
    cout << "  <input>                  Text to encrypt or decrypt\n";
    cout << "  -a, --action             Encrypt|en or decrypt|de <input> / <file>\n";
    cout << "  -k, --key                How many letters to shift?\n";
    cout << "  -m, --mode               capital|c or small|s, changes output letter case\n";
    cout << "  -r  --remove-spaces      Remove white spaces from output\n";
    cout << "  -f  --file <file>        Read input from file\n";
    cout << "  -o  --output <file>      Save output to file\n";
    cout << "  -h, --help               Show this help message\n";
    cout << "  -v, --version            Show version\n";
    cout << "  -i, --info               Show detailed program information\n";
}

void showVersion() {
    // Keep this version synchronized with showInfo().
    cout << "csr v.0.0.2\n";
}

void showInfo() {
    // Print project details.
    cout << "==================================================\n";
    cout << "           Caesar Cipher CLI Tool (v0.0.2)\n";
    cout << "==================================================\n\n";

    cout << "Description:\n";
    cout << "  A command-line utility for encrypting and decrypting\n";
    cout << "  text with the classical Caesar Cipher algorithm.\n";
    cout << "  The tool supports direct input, file input, file output,\n";
    cout << "  output case conversion, and optional space removal.\n\n";

    cout << "Cipher Behavior:\n";
    cout << "  Alphabetic characters are shifted by the selected key.\n";
    cout << "  Letter case is preserved unless --mode is used.\n";
    cout << "  Non-alphabetic characters are kept unchanged.\n\n";

    cout << "Examples:\n";
    cout << "  Encrypt: csr \"Hello\" --action encrypt --key 3\n";
    cout << "  Output : Khoor\n";
    cout << "  Decrypt: csr \"Khoor\" --action decrypt --key 3\n";
    cout << "  Output : Hello\n\n";

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
