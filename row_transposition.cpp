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

// Encrypts text using the Row Transposition Cipher.
// The plaintext is written row by row into a matrix, then columns are read
// according to the sorted key order to produce the ciphertext.
string decrypt(string input, vector<int> key);

// Encrypts text using the Row Transposition Cipher.
// The plaintext is written row by row into a matrix, then columns are read
// according to the sorted key order to produce the ciphertext.
string encrypt(string input, vector<int> key);

vector<int> getOrder(vector<int> key);

vector<int> stringToVector(const string &str);

// Saves the final output text to a file.
// Returns true if the file was written successfully, false otherwise.
bool saveOutput(const string &output, const string &filePath);

// Reads the entire contents of a text file into the content string.
// Returns true if the file was read successfully, false otherwise.
bool readTextFile(const string &filePath, string &content);

// Displays usage instructions and available command-line options.
void showHelp();

// Displays the program version.
void showVersion();

// Displays detailed information about the program, course, and authors.
void showInfo();

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

    // If requested, remove all regular space characters from the input.
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

    // A Row Transposition Cipher requires a numeric key.
    if (argsMap.find("--key") == argsMap.end() && argsMap.find("-k") == argsMap.end()) {
        cout << "No key specified\n";
        return 1;
    }

    // Convert the key argument from string to integer.

    vector<int> vKey;

    if (argsMap.find("--key") != argsMap.end()) {
        vKey = stringToVector(argsMap["--key"]);
    }

    // This variable stores the encrypted or decrypted result.
    string output;

    // Perform the requested operation.
    if (action == "encrypt" || action == "en") {
        output = encrypt(input, vKey);
    } else if (action == "decrypt" || action == "de") {
        output = decrypt(input, vKey);
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


// Encrypts text using the Row Transposition Cipher.
//
// Process:
//   1. Create a matrix with:
//        rows    = enough rows to hold the full input
//        columns = key length
//
//   2. Fill the matrix row by row using the plaintext.
//      If the last row is incomplete, the remaining cells stay as 'X' padding.
//
//   3. Calculate the column reading order from the key.
//
//   4. Read columns from top to bottom using that order.
//
// Example:
//   input: "HELLOWORLD"
//   key:   {3, 1, 4, 2}
//
//   Matrix size:
//     columns = 4
//     rows    = 3
//
//   Matrix after row-wise filling:
//
//       column index:  0   1   2   3
//                    -----------------
//                     H   E   L   L
//                     O   W   O   R
//                     L   D   X   X
//
//   Key order:
//     key value 1 is at column 1
//     key value 2 is at column 3
//     key value 3 is at column 0
//     key value 4 is at column 2
//
//   So the column reading order is:
//     {1, 3, 0, 2}
//
//   Ciphertext is produced by reading columns in that order:
//
//     column 1 -> E W D
//     column 3 -> L R X
//     column 0 -> H O L
//     column 2 -> L O X
//
//   result:
//     "EWDLRXHOLLOX"
//
// Notes:
//   - The padding character is 'X'.
//   - Padding is included in the encrypted output.
//   - The same key must be used for decryption.
//
// Parameters:
//   input - Plaintext to encrypt.
//   key   - Numeric transposition key. Each value controls one column.
//
// Returns:
//   Encrypted ciphertext.
string encrypt(string input, vector<int> key) {
    int cols = key.size();
    int rows = (input.size() + cols - 1) / cols;

    vector<string> matrix(rows, string(cols, 'X'));


    // Fill the matrix row by row.
    //
    // Example with input "HELLOWORLD" and 4 columns:
    //   row 0: H E L L
    //   row 1: O W O R
    //   row 2: L D X X
    //
    // idx tracks the next character to copy from input.
    // If input ends before the matrix is full, the remaining cells keep 'X'.

    int idx = 0;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (idx < input.size())
                matrix[r][c] = input[idx++];
        }
    }

    // Get column reading order from the key.
    //
    // Example:
    //   key   = {3, 1, 4, 2}
    //   order = {1, 3, 0, 2}
    //
    // This means:
    //   read column 1 first,
    //   read column 3 second,
    //   read column 0 third,
    //   read column 2 fourth.

    vector<int> order = getOrder(key);
    string result;


    // Read the selected columns from top to bottom.
    //
    // Using the example matrix:
    //   column 1 gives E W D
    //   column 3 gives L R X
    //   column 0 gives H O L
    //   column 2 gives L O X
    for (int c: order) {
        for (int r = 0; r < rows; r++) {
            result += matrix[r][c];
        }
    }

    return result;
}


// Decrypts text encrypted with the Row Transposition Cipher.
//
// Process:
//   1. Recreate the same matrix dimensions used during encryption.
//      The number of columns is the key length.
//      The number of rows is calculated from the ciphertext length.
//
//   2. Calculate the same column order from the key.
//
//   3. Fill the matrix column by column using the sorted key order.
//      This reverses the encryption step where columns were read in that order.
//
//   4. Read the completed matrix row by row to recover the padded plaintext.
//
// Example:
//   ciphertext: "EWDLRXHOLLOX"
//   key:        {3, 1, 4, 2}
//
//   Column order from the key:
//     {1, 3, 0, 2}
//
//   Fill the matrix columns in this order:
//
//     column 1 receives: E W D
//     column 3 receives: L R X
//     column 0 receives: H O L
//     column 2 receives: L O X
//
//   Reconstructed matrix:
//
//       column index:  0   1   2   3
//                    -----------------
//                     H   E   L   L
//                     O   W   O   R
//                     L   D   X   X
//
//   Reading row by row gives:
//
//     "HELLOWORLDXX"
//
// Notes:
//   - The decrypted output may contain padding characters.
//   - This function does not remove trailing 'X' automatically because
//     the original plaintext may legitimately end with 'X'.
//   - Use the same key that was used for encryption.
//
// Parameters:
//   input - Ciphertext to decrypt.
//   key   - Numeric transposition key used during encryption.
//
// Returns:
//   Decrypted plaintext, including any padding.
string decrypt(string input, vector<int> key) {
    const int cols = key.size();
    const int rows = (input.size() + cols - 1) / cols;

    vector<string> matrix(rows, string(cols, 'X'));

    const vector<int> order = getOrder(key);


    // Fill the matrix column by column using the same order used in encryption.
    //
    // Example:
    //   ciphertext = "EWDLRXHOLLOX"
    //   order      = {1, 3, 0, 2}
    //
    // Filling order:
    //   column 1 <- E W D
    //   column 3 <- L R X
    //   column 0 <- H O L
    //   column 2 <- L O X
    //
    // idx tracks the next ciphertext character to place into the matrix.
    int idx = 0;
    for (int k = 0; k < cols; k++) {
        int c = order[k];
        for (int r = 0; r < rows; r++) {
            if (idx < input.size())
                matrix[r][c] = input[idx++];
        }
    }

    // Read the reconstructed matrix row by row.
    //
    // Using the example matrix:
    //   row 0 -> H E L L
    //   row 1 -> O W O R
    //   row 2 -> L D X X
    //
    // Result:
    //   "HELLOWORLDXX"
    string result;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            result += matrix[r][c];
        }
    }

    return result;
}


// Calculates the column reading order from the numeric key.
//
// Purpose:
//   In a row transposition cipher, columns are not read left to right.
//   They are read according to the ascending order of the key values.
//
// Example:
//   key:          {3, 1, 4, 2}
//   column index:  0  1  2  3
//
//   Sort the key values:
//
//     key value 1 is in column 1
//     key value 2 is in column 3
//     key value 3 is in column 0
//     key value 4 is in column 2
//
//   Therefore, the returned order is:
//
//     {1, 3, 0, 2}
//
// Meaning:
//   column 1 is read first,
//   column 3 is read second,
//   column 0 is read third,
//   column 2 is read fourth.
//
// Example with repeated values:
//   key:          {2, 1, 2}
//   column index:  0  1  2
//
//   The function stores pairs of:
//
//     {key value, original column index}
//
//   So the temporary pairs are:
//
//     {(2, 0), (1, 1), (2, 2)}
//
//   After sorting:
//
//     {(1, 1), (2, 0), (2, 2)}
//
//   Returned order:
//
//     {1, 0, 2}
//
// Notes:
//   - pair sorting makes repeated key values deterministic.
//   - The first element of each pair is the key value.
//   - The second element of each pair is the original column index.
//
// Parameters:
//   key - Numeric transposition key.
//
// Returns:
//   Column indexes sorted by their corresponding key values.
vector<int> getOrder(vector<int> key) {
    const int n = key.size();
    vector<pair<int, int> > temp;

    // Pair each key value with its original column index.
    //
    // Example:
    //   key = {3, 1, 4, 2}
    //
    // Stored pairs:
    //   {(3, 0), (1, 1), (4, 2), (2, 3)}
    for (int i = 0; i < n; i++)
        temp.push_back({key[i], i});

    // Sort pairs by key value.
    //
    // Before:
    //   {(3, 0), (1, 1), (4, 2), (2, 3)}
    //
    // After:
    //   {(1, 1), (2, 3), (3, 0), (4, 2)}
    //
    // If two key values are equal, the column index is used as a tie-breaker.
    sort(temp.begin(), temp.end());

    // Extract the original column indexes from the sorted pairs.
    //
    // From:
    //   {(1, 1), (2, 3), (3, 0), (4, 2)}
    //
    // Extract:
    //   {1, 3, 0, 2}

    vector<int> order(n);
    for (int i = 0; i < n; i++) order[i] = temp[i].second;

    return order;
}


// Converts a string key into a vector of integer digits.
//
// Purpose:
//   Command-line arguments are received as strings.
//   For this cipher, the key must be handled as separate numeric values,
//   where each digit represents the priority of one column.
//
// Example:
//   Input string:
//     "3142"
//
//   Output vector:
//     {3, 1, 4, 2}
//
// Explanation:
//   Each character is converted by subtracting '0':
//
//     '3' - '0' = 3
//     '1' - '0' = 1
//     '4' - '0' = 4
//     '2' - '0' = 2
//
// Example usage:
//   vector<int> key = stringToVector("3142");
//
//   Then:
//     key[0] == 3
//     key[1] == 1
//     key[2] == 4
//     key[3] == 2
//
// Important:
//   This function assumes the string contains only digit characters.
//   Valid example:
//     "3142"
//
//   Invalid example:
//     "31A2"
//
//   If non-digit characters are possible, validate the string before calling
//   this function.
//
// Parameters:
//   str - Numeric key as a string.
//
// Returns:
//   A vector containing the integer value of each digit.
vector<int> stringToVector(const string &str) {
    vector<int> result;

    // Convert each character digit into its integer value.
    //
    // Example:
    //   c = '3'
    //   c - '0' = 3
    for (const char c: str) {
        result.push_back(c - '0'); // convert char to int
    }
    return result;
}


bool saveOutput(const string &output, const string &filePath) {
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

bool readTextFile(const string &filePath, string &content) {
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
    cout << "Usage: rt [options]\n\n";
    cout << "Options:\n";
    cout << "  <input>                  Text to encrypt or decrypt\n";
    cout << "  -a, --action             encrypt|en or decrypt|de\n";
    cout << "  -k, --key                Numeric transposition key, example: 3142\n";
    cout << "  -m, --mode               capital|c or small|s, changes output letter case\n";
    cout << "  -r, --remove-spaces      Remove regular spaces from output\n";
    cout << "  -f, --file <file>        Read input from file\n";
    cout << "  -o, --output <file>      Save output to file\n";
    cout << "  -h, --help               Show this help message\n";
    cout << "  -v, --version            Show version\n";
    cout << "  -i, --info               Show detailed program information\n";
}

void showVersion() {
    // Print the current program version.
    cout << "rt v0.0.1\n";
}

void showInfo() {
    // Print detailed information about the program.
    cout << "==================================================\n";
    cout << "        Row Transposition Cipher Tool (v0.0.1)\n";
    cout << "==================================================\n\n";

    cout << "Description:\n";
    cout << "  A command-line utility for encrypting and decrypting\n";
    cout << "  text using the classical row transposition cipher.\n";
    cout << "  The message is written into rows, then columns are\n";
    cout << "  read according to the sorted numeric key order.\n\n";

    cout << "Example:\n";
    cout << "  Input: HELLOWORLD\n";
    cout << "  Key:   3142\n";
    cout << "  The key controls the order in which columns are read.\n\n";

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
