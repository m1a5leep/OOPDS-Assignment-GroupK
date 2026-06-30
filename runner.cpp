#include <fstream>
#include <iostream>
#include <string>
#include <cctype>
#include <cstdlib>

using namespace std;

class Register {
private:
    signed char value; // 1 byte signed char (-128 to 127)

public:
    Register() {
        value = 0;
    }

    virtual ~Register() {} 

    virtual void setValue(signed char val) {
        value = val;
    }

    virtual signed char getValue() const {
        return value;
    }
};

// FONG WEI HONG 253UC25627 - Member 1
class GeneralRegister : public Register {
public:
    GeneralRegister() : Register() {}

    // Explicitly overriding the base class methods to match your specific design
    void setValue(signed char val) override {
        // Must call the parent method because 'value' is private in Register
        Register::setValue(val); 
    }

    signed char getValue() const override {
        return Register::getValue(); 
    }
};

class FlagRegister {
private:
    bool OF; // Overflow Flag
    bool UF; // Underflow Flag
    bool CF; // Carry Flag
    bool ZF; // Zero Flag

public:
    FlagRegister() {
        resetAllFlags();
    }

    void setOF(bool val) { OF = val; }
    bool getOF() const { return OF; }

    void setUF(bool val) { UF = val; }
    bool getUF() const { return UF; }

    void setCF(bool val) { CF = val; }
    bool getCF() const { return CF; }

    void setZF(bool val) { ZF = val; }
    bool getZF() const { return ZF; }

    void resetAllFlags() {
        OF = false;
        UF = false;
        CF = false;
        ZF = false;
    }

    // Helper to evaluate a math result and set flags accordingly
    void updateFlagsFromMath(int rawResult) {
        resetAllFlags();
        
        if (rawResult == 0) {
            setZF(true);
        }
        if (rawResult > 127) {
            setOF(true);
        }
        if (rawResult < -128) {
            setUF(true);
        }
        // Carry flag logic: if the result requires more than 8 bits
        if (rawResult > 255 || rawResult < -256) {
            setCF(true);
        }
    }
};

class CPU {
private:
    GeneralRegister registers[8]; // R0 to R7
    unsigned char PC;             // Program Counter (starts at 0)
    unsigned char SI;             // Stack Index (starts at 0)
    
    // CPU aggregates FlagRegister (passed in via pointer)
    FlagRegister* flags;          
    
    // CPU composes Memory (owns its lifecycle)
    Memory* memory;               

public:
    // Constructor handles composition and aggregation
    CPU(FlagRegister* externalFlags) {
        flags = externalFlags;
        PC = 0;
        SI = 0;
        // memory = new Memory(); // Uncomment when Member 2 finishes Memory class
    }

    ~CPU() {
        // delete memory; // Uncomment when Member 2 finishes Memory class
    }

    // --- PC & SI Management ---
    unsigned char getPC() const { return PC; }
    void incrementPC() { PC++; }

    unsigned char getSI() const { return SI; }
    void incrementSI() { SI++; }
    void decrementSI() { SI--; }

    // --- Register & Flag Access ---
    GeneralRegister& getRegister(int index) {
        return registers[index];
    }

    FlagRegister* getFlags() {
        return flags;
    }
};

static string trim(const string& text)
{
    size_t start = 0;
    size_t end = text.size();

    while (start < end && isspace((unsigned char)text[start]))
    {
        start++;
    }

    while (end > start && isspace((unsigned char)text[end - 1]))
    {
        end--;
    }

    return text.substr(start, end - start);
}

static bool parseRegisterIndex(const string& token, int& index)
{
    if (token.size() != 2)
    {
        return false;
    }

    char letter = token[0];
    char digit = token[1];

    if ((letter != 'R' && letter != 'r') || digit < '0' || digit > '7')
    {
        return false;
    }

    index = digit - '0';
    return true;
}

static bool parseImmediate(const string& token, int& value)
{
    if (token.empty())
    {
        return false;
    }

    int sign = 1;
    size_t i = 0;

    if (token[0] == '+' || token[0] == '-')
    {
        if (token[0] == '-')
        {
            sign = -1;
        }
        i = 1;
    }

    if (i >= token.size())
    {
        return false;
    }

    int result = 0;
    for (; i < token.size(); ++i)
    {
        char c = token[i];
        if (c < '0' || c > '9')
        {
            return false;
        }
        result = result * 10 + (c - '0');
    }

    value = result * sign;
    return true;
}

static bool splitOperands(const string& text, string& left, string& right)
{
    size_t commaPos = text.find(',');
    if (commaPos == string::npos)
    {
        return false;
    }

    left = trim(text.substr(0, commaPos));
    right = trim(text.substr(commaPos + 1));
    return true;
}

class DynamicArray;

class Runner
{
private:
    CPU* cpu;
    DynamicArray* program;

public:
    Runner(CPU* c);

    void loadProgram(const string& filename);

    void run();

    void executeInstruction(const string& line);
};

void Runner::loadProgram(const string& filename) // Load assembly instructions from file
{
    ifstream file(filename);

    string line;

    while(getline(file, line))
    {
        if(line != "")
        {
            program->add(line);
        }
    }

    file.close();
}

void Runner::run() // Execute the loaded program
{
    for(int i = 0; i < program->size(); i++)
    {
        string line = program->get(i);
        executeInstruction(line);
    }
}

void Runner::executeInstruction(const string& line) // Execute a single instruction
{
    string trimmed = trim(line);
    if (trimmed.empty())
    {
        return;
    }

    size_t separator = trimmed.find_first_of(" \t");
    string opcode;
    string operands;

    if (separator == string::npos)
    {
        opcode = trimmed;
    }
    else
    {
        opcode = trim(trimmed.substr(0, separator));
        operands = trim(trimmed.substr(separator));
    }

    for (char& c : opcode)
    {
        c = (char)toupper((unsigned char)c);
    }

    if (opcode == "MOV")
    {
        string srcText;
        string dstText;
        if (!splitOperands(operands, dstText, srcText))
        {
            cerr << "Error: MOV requires two operands" << endl;
            exit(1);
        }

        int dstIndex;
        if (!parseRegisterIndex(dstText, dstIndex))
        {
            cerr << "Error: invalid destination register in MOV: " << dstText << endl;
            exit(1);
        }

        int srcIndex;
        int srcValue;
        if (parseRegisterIndex(srcText, srcIndex))
        {
            cpu->getRegister(dstIndex).setValue(cpu->getRegister(srcIndex).getValue());
        }
        else if (parseImmediate(srcText, srcValue))
        {
            cpu->getRegister(dstIndex).setValue((signed char)srcValue);
        }
        else
        {
            cerr << "Error: invalid source operand in MOV: " << srcText << endl;
            exit(1);
        }
    }
    else if (opcode == "INPUT")
    {
        int regIndex;
        if (!parseRegisterIndex(operands, regIndex))
        {
            cerr << "Error: INPUT requires a register operand" << endl;
            exit(1);
        }

        int value;
        cout << "INPUT " << "R" << regIndex << ": ";
        if (!(cin >> value))
        {
            cerr << "Error: failed to read input" << endl;
            exit(1);
        }

        cpu->getRegister(regIndex).setValue((signed char)value);
    }
    else if (opcode == "DISPLAY")
    {
        int regIndex;
        if (!parseRegisterIndex(operands, regIndex))
        {
            cerr << "Error: DISPLAY requires a register operand" << endl;
            exit(1);
        }

        signed char value = cpu->getRegister(regIndex).getValue();
        cout << "DISPLAY R" << regIndex << " = " << (int)value << endl;
    }
    else if (opcode == "ROL" || opcode == "ROR" || opcode == "SHL" || opcode == "SHR")
    {
        string regText;
        string countText;
        if (!splitOperands(operands, regText, countText))
        {
            cerr << "Error: " << opcode << " requires a register and count" << endl;
            exit(1);
        }

        int regIndex;
        if (!parseRegisterIndex(regText, regIndex))
        {
            cerr << "Error: invalid register for " << opcode << ": " << regText << endl;
            exit(1);
        }

        int count;
        if (!parseImmediate(countText, count))
        {
            cerr << "Error: invalid shift count for " << opcode << ": " << countText << endl;
            exit(1);
        }

        unsigned char value = (unsigned char)cpu->getRegister(regIndex).getValue();
        unsigned char result = value;
        int shift = count & 7;

        if (shift == 0)
        {
            result = value;
        }
        else if (opcode == "SHL")
        {
            result = (unsigned char)((value << shift) & 0xFF);
        }
        else if (opcode == "SHR")
        {
            result = (unsigned char)(value >> shift);
        }
        else if (opcode == "ROL")
        {
            result = (unsigned char)(((value << shift) | (value >> (8 - shift))) & 0xFF);
        }
        else if (opcode == "ROR")
        {
            result = (unsigned char)(((value >> shift) | (value << (8 - shift))) & 0xFF);
        }

        cpu->getRegister(regIndex).setValue((signed char)result);
    }
    else
    {
        cerr << "Error: unknown instruction '" << opcode << "'" << endl;
        exit(1);
    }

    cpu->incrementPC();
}

Runner::Runner(CPU* c)
{
    cpu = c;
    program = new DynamicArray();
}

Runner::~Runner()
{
    delete program;
}
