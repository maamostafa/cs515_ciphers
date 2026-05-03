// ============================================================
//  ciphershell.cpp  Interactive wizard & top-level dispatcher
//
//  Guides the user through 8 questions, builds the exact
//  sub-command line, prints a summary table, then executes
//  the chosen cipher binary (csr / rf / rt).
//
//  On Windows  : uses CreateProcess() -- bypasses cmd.exe entirely,
//                so quoted arguments with spaces work correctly.
//  On POSIX    : uses fork() + execvp() -- same benefit.
//
//  Subcommands expected in the same directory (or on PATH):
//    csr   rf   rt
//
//  Project : CS515  Cryptography & Network Security
//  Under   : Dr. Nermin Hamza
//  Tool    : CipherShell v1.0.0
// ============================================================
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cstdlib>

#ifdef _WIN32
  #include <windows.h>
  #define CLEAR_CMD "cls"
  #define PATH_SEP  "\\"
  #define EXE_EXT   ".exe"
#else
  #include <unistd.h>
  #include <sys/wait.h>
  #define CLEAR_CMD "clear"
  #define PATH_SEP  "/"
  #define EXE_EXT   ""
#endif

// -- Colour helpers --------------------------------------------------
namespace C {
    const char* rst  = "\033[0m";
    const char* bold = "\033[1m";
    const char* grn  = "\033[32m";
    const char* cyn  = "\033[36m";
    const char* yel  = "\033[33m";
    const char* red  = "\033[31m";
    const char* mag  = "\033[35m";
    const char* gray = "\033[90m";
    const char* blu  = "\033[34m";
}

static void clrscr() { std::system(CLEAR_CMD); }

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// -- Box-drawing helpers (pure ASCII) --------------------------------
static void hline(char fill = '-', int w = 64) {
    for (int i = 0; i < w; ++i) std::cout << fill;
    std::cout << '\n';
}

static void boxRow(const std::string& label, const std::string& val,
                   int colW1 = 22, int colW2 = 34) {
    std::cout << C::gray << "  | " << C::rst
              << C::cyn  << std::left << std::setw(colW1) << label << C::rst
              << C::gray << " | " << C::rst
              << C::bold << std::left << std::setw(colW2) << val   << C::rst
              << C::gray << " |\n" << C::rst;
}

// -- Intro banner ----------------------------------------------------
static void printIntro() {
    std::cout << "\n";
    std::cout << C::grn << C::bold;
    std::cout << "  +----------------------------------------------------------------+\n";
    std::cout << "  |          CipherShell - Classical Cipher Toolkit                |\n";
    std::cout << "  |                        Version 1.0.0                           |\n";
    std::cout << "  +----------------------------------------------------------------+\n";
    std::cout << C::rst << C::grn;
    std::cout << "  |  Ciphers  : Caesar (csr) * Rail Fence (rf) * Row Trans (rt)   |\n";
    std::cout << "  |  Project  : CS515 - Cryptography & Network Security            |\n";
    std::cout << "  |  Supervisor: Dr. Nermin Hamza                                  |\n";
    std::cout << "  +----------------------------------------------------------------+\n";
    std::cout << C::rst;
    std::cout << "\n  " << C::gray << "Type 'r' at any prompt to restart * 'q' to quit\n" << C::rst;
    std::cout << "\n";
}

// -- State -----------------------------------------------------------
struct Session {
    std::string algo;         // csr | rf | rt
    std::string action;       // encrypt | decrypt
    std::string inputType;    // direct | file
    std::string text;         // raw text or file path
    std::string key;
    std::string mode;         // capital | small | preserve
    std::string removeSpaces; // yes | no
    std::string outputFile;   // empty = stdout
};

// -- Prompt helpers --------------------------------------------------
enum class PollResult { Ok, Restart, Quit };

static void stepHeader(int n, int total, const std::string& title) {
    std::cout << "\n  " << C::yel << C::bold
              << "[" << n << "/" << total << "] " << title
              << C::rst << "\n";
}

static PollResult readLine(std::string& out) {
    std::getline(std::cin, out);
    out = trim(out);
    std::string lo = toLower(out);
    if (lo == "r" || lo == "restart") return PollResult::Restart;
    if (lo == "q" || lo == "quit" || lo == "exit") return PollResult::Quit;
    return PollResult::Ok;
}

static PollResult choicePrompt(const std::vector<std::pair<std::string,std::string>>& opts,
                                std::string& chosen) {
    for (size_t i = 0; i < opts.size(); ++i)
        std::cout << "    " << C::cyn << "[" << (i+1) << "] "
                  << C::bold << opts[i].first
                  << C::rst  << "  " << C::gray << opts[i].second << C::rst << "\n";

    for (;;) {
        std::cout << "\n  " << C::grn << "> " << C::rst;
        std::string raw;
        PollResult pr = readLine(raw);
        if (pr != PollResult::Ok) return pr;

        std::string lo = toLower(raw);
        if (raw.size() == 1 && raw[0] >= '1' &&
            raw[0] <= static_cast<char>('0' + opts.size())) {
            chosen = opts[raw[0]-'1'].first;
            return PollResult::Ok;
        }
        for (auto& o : opts) {
            if (lo == toLower(o.first)) { chosen = o.first; return PollResult::Ok; }
        }
        std::cout << "  " << C::red << "Invalid choice. Try again." << C::rst << "\n";
    }
}

static PollResult textPrompt(const std::string& hint, std::string& out,
                              bool allowEmpty = false) {
    if (!hint.empty())
        std::cout << "  " << C::gray << hint << C::rst << "\n";
    for (;;) {
        std::cout << "\n  " << C::grn << "> " << C::rst;
        PollResult pr = readLine(out);
        if (pr != PollResult::Ok) return pr;
        if (!out.empty() || allowEmpty) return PollResult::Ok;
        std::cout << "  " << C::red << "Cannot be empty. Try again." << C::rst << "\n";
    }
}

// -- Summary table ---------------------------------------------------
static void printSummary(const Session& s, const std::string& displayCmd) {
    std::map<std::string,std::string> algoNames =
        {{"csr","Caesar"},{"rf","Rail Fence"},{"rt","Row Transposition"}};

    std::cout << "\n";
    std::cout << C::cyn << C::bold;
    std::cout << "  +--------------------------------------------------------------+\n";
    std::cout << "  |                    Session Summary                           |\n";
    std::cout << "  +--------------------------------------------------------------+\n";
    std::cout << C::rst;

    boxRow("Cipher",        algoNames[s.algo] + " (" + s.algo + ")");
    boxRow("Action",        s.action);
    boxRow("Input type",    s.inputType);
    boxRow("Input",         s.text.size() > 30 ? s.text.substr(0,27)+"..." : s.text);
    boxRow("Key",           s.key);
    boxRow("Case mode",     s.mode);
    boxRow("Remove spaces", s.removeSpaces);
    boxRow("Output",        s.outputFile.empty() ? "stdout (console)" : s.outputFile);

    std::cout << C::cyn;
    std::cout << "  +--------------------------------------------------------------+\n";
    std::cout << "  |  Generated command                                           |\n";
    std::cout << "  |  " << C::rst << C::yel << C::bold
              << std::left << std::setw(59) << displayCmd.substr(0, 59)
              << C::cyn << " |\n";
    if (displayCmd.size() > 59) {
        std::cout << "  |  " << C::rst << C::yel
                  << std::left << std::setw(59) << displayCmd.substr(59, 59)
                  << C::cyn << " |\n";
    }
    std::cout << "  +--------------------------------------------------------------+\n";
    std::cout << C::rst << "\n";
}

// -- Argument list builder -------------------------------------------
// Returns a clean argv-style token list with no shell quoting.
static std::vector<std::string> buildArgs(const Session& s,
                                          const std::string& selfDir) {
    std::string bin = selfDir + s.algo + EXE_EXT;
    {
        std::ifstream test(bin);
        if (!test.good()) bin = s.algo + std::string(EXE_EXT);
    }

    std::vector<std::string> args;
    args.push_back(bin);

    if (s.inputType == "file") {
        args.push_back("--file");
        args.push_back(s.text);
    } else {
        args.push_back(s.text);     // positional plain-text argument
    }

    args.push_back("--action");  args.push_back(s.action);
    args.push_back("--key");     args.push_back(s.key);
    if (s.mode != "preserve") {
        args.push_back("--mode");
        args.push_back(s.mode);
    }
    if (s.removeSpaces == "yes") args.push_back("--remove-spaces");
    if (!s.outputFile.empty()) {
        args.push_back("--output");
        args.push_back(s.outputFile);
    }

    return args;
}

// Human-readable command string for the summary box only
static std::string argsToDisplay(const std::vector<std::string>& args) {
    std::ostringstream ss;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i) ss << ' ';
        bool needQ = args[i].find(' ') != std::string::npos;
        if (needQ) ss << '"';
        ss << args[i];
        if (needQ) ss << '"';
    }
    return ss.str();
}

// -- Cross-platform process launcher ---------------------------------
//
//  Windows : CreateProcess() -- does NOT invoke cmd.exe, so there is
//            no shell-quoting ambiguity regardless of spaces in args.
//  POSIX   : fork() + execvp() -- same benefit.
//
static int runProcess(const std::vector<std::string>& args) {
    if (args.empty()) return 1;

#ifdef _WIN32
    // Build a single command-line string where every token is
    // individually double-quoted (internal " escaped as \").
    // CreateProcess passes this directly to the Windows process loader,
    // completely bypassing cmd.exe.
    std::string cmdline;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i) cmdline += ' ';
        cmdline += '"';
        for (char c : args[i]) {
            if (c == '"') cmdline += "\\\"";
            else          cmdline += c;
        }
        cmdline += '"';
    }

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    std::vector<char> buf(cmdline.begin(), cmdline.end());
    buf.push_back('\0');

    if (!CreateProcessA(
            nullptr,    // exe found from first quoted token in cmdline
            buf.data(), // full command line (must be mutable)
            nullptr,    // process security attributes
            nullptr,    // thread security attributes
            FALSE,      // do not inherit handles
            0,          // creation flags
            nullptr,    // inherit parent environment
            nullptr,    // inherit parent current directory
            &si,
            &pi)) {
        std::cerr << "  Failed to launch " << args[0]
                  << " (Windows error " << GetLastError() << ")\n";
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return static_cast<int>(exitCode);

#else
    // POSIX: fork + exec directly -- no shell involved.
    std::vector<char*> argv;
    for (const auto& a : args)
        argv.push_back(const_cast<char*>(a.c_str()));
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "  fork() failed\n";
        return 1;
    }
    if (pid == 0) {
        execvp(argv[0], argv.data());
        std::cerr << "  execvp failed: " << argv[0] << "\n";
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
#endif
}

// -- Main wizard loop ------------------------------------------------
int main(int argc, char* argv[]) {

    std::string selfDir;
    if (argc > 0) {
        std::string self(argv[0]);
        size_t sep = self.find_last_of("/\\");
        if (sep != std::string::npos)
            selfDir = self.substr(0, sep + 1);
    }

    const int TOTAL = 8;

restart:
    clrscr();
    printIntro();

    Session s;

// -- Q1: Algorithm ---------------------------------------------------
    stepHeader(1, TOTAL, "Choose a cipher algorithm");
    {
        std::vector<std::pair<std::string,std::string>> opts = {
            {"csr", "Caesar cipher - shift each letter by a fixed amount"},
            {"rf",  "Rail Fence cipher - zigzag transposition"},
            {"rt",  "Row Transposition cipher - column reordering"},
        };
        PollResult r = choicePrompt(opts, s.algo);
        if (r == PollResult::Restart) goto restart;
        if (r == PollResult::Quit)    goto quit;
    }

// -- Q2: Action ------------------------------------------------------
    stepHeader(2, TOTAL, "Choose an action");
    {
        std::vector<std::pair<std::string,std::string>> opts = {
            {"encrypt", "Transform plaintext -> ciphertext"},
            {"decrypt", "Recover plaintext from ciphertext"},
        };
        PollResult r = choicePrompt(opts, s.action);
        if (r == PollResult::Restart) goto restart;
        if (r == PollResult::Quit)    goto quit;
    }

// -- Q3: Input source ------------------------------------------------
    stepHeader(3, TOTAL, "Input source");
    {
        std::vector<std::pair<std::string,std::string>> opts = {
            {"direct", "Type or paste text here"},
            {"file",   "Read from a .txt file (provide path next)"},
        };
        PollResult r = choicePrompt(opts, s.inputType);
        if (r == PollResult::Restart) goto restart;
        if (r == PollResult::Quit)    goto quit;
    }

// -- Q4: Text / file path --------------------------------------------
    if (s.inputType == "direct") {
        stepHeader(4, TOTAL, "Enter your text");
        PollResult r = textPrompt("Paste or type the message:", s.text);
        if (r == PollResult::Restart) goto restart;
        if (r == PollResult::Quit)    goto quit;
    } else {
        stepHeader(4, TOTAL, "Enter the path to your .txt file");
        for (;;) {
            PollResult r = textPrompt("Example: C:\\data\\message.txt or ./message.txt", s.text);
            if (r == PollResult::Restart) goto restart;
            if (r == PollResult::Quit)    goto quit;
            std::ifstream test(s.text);
            if (test.good()) break;
            std::cout << "  " << C::red << "File not found: " << s.text
                      << ". Try again." << C::rst << "\n";
            s.text.clear();
        }
    }

// -- Q5: Key ---------------------------------------------------------
    stepHeader(5, TOTAL, "Enter the cipher key");
    {
        std::string hint;
        if      (s.algo == "csr") hint = "Integer shift value, e.g. 3  (negative shifts left)";
        else if (s.algo == "rf")  hint = "Number of rails (integer >= 2), e.g. 3";
        else                      hint = "Column order as digit string, e.g. 3124";

        PollResult r = textPrompt(hint, s.key);
        if (r == PollResult::Restart) goto restart;
        if (r == PollResult::Quit)    goto quit;
    }

// -- Q6: Case mode ---------------------------------------------------
    stepHeader(6, TOTAL, "Character case mode");
    {
        std::vector<std::pair<std::string,std::string>> opts = {
            {"capital",  "Convert all output to UPPERCASE"},
            {"small",    "Convert all output to lowercase"},
            {"preserve", "Keep original character case unchanged"},
        };
        PollResult r = choicePrompt(opts, s.mode);
        if (r == PollResult::Restart) goto restart;
        if (r == PollResult::Quit)    goto quit;
    }

// -- Q7: Remove spaces -----------------------------------------------
    stepHeader(7, TOTAL, "Remove spaces?");
    {
        std::string note = (s.algo == "csr")
            ? "  (Caesar: spaces removed from output)"
            : "  (Rail Fence / Row Trans: spaces removed from input before processing)";
        std::cout << C::gray << note << C::rst << "\n";

        std::vector<std::pair<std::string,std::string>> opts = {
            {"yes", "Strip all space characters"},
            {"no",  "Keep spaces as-is"},
        };
        PollResult r = choicePrompt(opts, s.removeSpaces);
        if (r == PollResult::Restart) goto restart;
        if (r == PollResult::Quit)    goto quit;
    }

// -- Q8: Output destination ------------------------------------------
    stepHeader(8, TOTAL, "Output destination");
    {
        std::vector<std::pair<std::string,std::string>> opts = {
            {"stdout", "Print result to the console"},
            {"file",   "Write result to a file"},
        };
        std::string outChoice;
        PollResult r = choicePrompt(opts, outChoice);
        if (r == PollResult::Restart) goto restart;
        if (r == PollResult::Quit)    goto quit;

        if (outChoice == "file") {
            stepHeader(8, TOTAL, "Enter output file path");
            r = textPrompt("Example: result.txt or C:\\output\\cipher.txt", s.outputFile);
            if (r == PollResult::Restart) goto restart;
            if (r == PollResult::Quit)    goto quit;
        }
    }

// -- Build args, show summary, run -----------------------------------
    {
        std::vector<std::string> args = buildArgs(s, selfDir);
        std::string displayCmd = argsToDisplay(args);
        printSummary(s, displayCmd);

        if (s.removeSpaces == "yes") {
            std::string where = (s.algo == "csr") ? "output" : "input";
            std::cout << "  " << C::gray << "*  Spaces will be stripped from "
                      << where << " before processing.\n" << C::rst;
        }

        std::cout << "\n  " << C::yel << C::bold << "Running cipher..." << C::rst << "\n\n";
        std::cout << C::gray;
        hline('-', 66);
        std::cout << C::rst;

        int ret = runProcess(args);

        std::cout << C::gray;
        hline('-', 66);
        std::cout << C::rst;

        if (ret != 0) {
            std::cout << "\n  " << C::red << "Cipher exited with code " << ret
                      << ". Check the error above." << C::rst << "\n";
        }
    }

// -- Start again? ----------------------------------------------------
    {
        std::cout << "\n  " << C::cyn << C::bold << "Start again? " << C::rst
                  << C::gray << "[y / n]" << C::rst << "\n";
        std::cout << "  " << C::grn << "> " << C::rst;
        std::string ans;
        std::getline(std::cin, ans);
        ans = toLower(trim(ans));
        if (ans == "y" || ans == "yes" || ans == "r" || ans == "restart")
            goto restart;
    }

quit:
    std::cout << "\n  " << C::grn << C::bold
              << "Goodbye! CipherShell terminated." << C::rst << "\n\n";
    return 0;
}