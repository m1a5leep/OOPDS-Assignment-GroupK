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

class Memory; // forward-declare Memory so CPU can hold a pointer

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

// Parse line into opcode and operands. Returns false for empty/blank lines.
static bool parseOpcodeAndOperands(const string& line, string& opcode, string& operands)
{
    string trimmed = trim(line);
    if (trimmed.empty()) return false;

    size_t sep = trimmed.find_first_of(" \t");
    if (sep == string::npos) {
        opcode = trimmed;
        operands.clear();
    } else {
        opcode = trim(trimmed.substr(0, sep));
        operands = trim(trimmed.substr(sep + 1));
    }

    for (char &c : opcode) c = (char)toupper((unsigned char)c);
    return true;
}

// MOV dst, src
static void Mov(CPU* cpu, const string& operands)
{
    string dstText, srcText;
    if (!splitOperands(operands, dstText, srcText)) {
        cerr << "Error: MOV requires two operands" << endl;
        exit(1);
    }

    int dstIndex;
    if (!parseRegisterIndex(dstText, dstIndex)) {
        cerr << "Error: invalid destination register in MOV: " << dstText << endl;
        exit(1);
    }

    int srcIndex, srcValue;
    if (parseRegisterIndex(srcText, srcIndex)) {
        cpu->getRegister(dstIndex).setValue(cpu->getRegister(srcIndex).getValue());
    } else if (parseImmediate(srcText, srcValue)) {
        cpu->getRegister(dstIndex).setValue((signed char)srcValue);
    } else {
        cerr << "Error: invalid source operand in MOV: " << srcText << endl;
        exit(1);
    }
}

// INPUT Rn
static void execInput(CPU* cpu, const string& operands)
{
    int regIndex;
    if (!parseRegisterIndex(operands, regIndex)) {
        cerr << "Error: INPUT requires a register operand" << endl;
        exit(1);
    }
    int value;
    cout << "INPUT R" << regIndex << ": ";
    if (!(cin >> value)) {
        cerr << "Error: failed to read input" << endl;
        exit(1);
    }
    cpu->getRegister(regIndex).setValue((signed char)value);
}

// DISPLAY Rn
static void execDisplay(CPU* cpu, const string& operands)
{
    int regIndex;
    if (!parseRegisterIndex(operands, regIndex)) {
        cerr << "Error: DISPLAY requires a register operand" << endl;
        exit(1);
    }
    signed char value = cpu->getRegister(regIndex).getValue();
    cout << "DISPLAY R" << regIndex << " = " << (int)value << endl;
}

// SHL/SHR/ROL/ROR Rn, count
static void execShiftRotate(CPU* cpu, const string& opcode, const string& operands)
{
    string regText, countText;
    if (!splitOperands(operands, regText, countText)) {
        cerr << "Error: " << opcode << " requires a register and count" << endl;
        exit(1);
    }
    int regIndex;
    if (!parseRegisterIndex(regText, regIndex)) {
        cerr << "Error: invalid register for " << opcode << ": " << regText << endl;
        exit(1);
    }
    int count;
    if (!parseImmediate(countText, count)) {
        cerr << "Error: invalid shift count for " << opcode << ": " << countText << endl;
        exit(1);
    }

    unsigned char value = (unsigned char)cpu->getRegister(regIndex).getValue();
    unsigned char result = value;
    int shift = count & 7;
    if (shift == 0) result = value;
    else if (opcode == "SHL") result = (unsigned char)((value << shift) & 0xFF);
    else if (opcode == "SHR") result = (unsigned char)(value >> shift);
    else if (opcode == "ROL") result = (unsigned char)(((value << shift) | (value >> (8 - shift))) & 0xFF);
    else if (opcode == "ROR") result = (unsigned char)(((value >> shift) | (value << (8 - shift))) & 0xFF);

    cpu->getRegister(regIndex).setValue((signed char)result);
}

class DynamicArray {
private:
    string* items;
    int capacity;
    int length;

    void grow() {
        int newCap = capacity == 0 ? 8 : capacity * 2;
        string* newItems = new string[newCap];
        for (int i = 0; i < length; ++i) newItems[i] = items[i];
        delete[] items;
        items = newItems;
        capacity = newCap;
    }

public:
    // Construct an empty dynamic array
    DynamicArray(): items(nullptr), capacity(0), length(0) {}

    // Destructor releases internal buffer
    ~DynamicArray() { delete[] items; }

    // Append a string to the array (grows automatically)
    void add(const string& s) {
        if (length >= capacity) grow();
        items[length++] = s;
    }

    // Retrieve an element by index (no bounds checks here)
    string get(int idx) const { return items[idx]; }

    // Number of elements stored
    int size() const { return length; }
};

class Runner

{
private:
    CPU* cpu;               // associated CPU instance
    DynamicArray* program;  // instruction storage (one line per element)
    
public:
    Runner(CPU* c);

    // Load assembly instructions from file into `program`.
    // Empty lines are ignored. Caller should ensure PC is reset if needed.
    void loadProgram(const string& filename);

    // Execute the loaded program, instruction by instruction.
    void run();

    // Parse and execute a single instruction line. Supported opcodes:
    // MOV, INPUT, DISPLAY, ROL, ROR, SHL, SHR.
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
    string opcode, operands;
    if (!parseOpcodeAndOperands(line, opcode, operands)) return;

    if (opcode == "MOV") Mov(cpu, operands);
    else if (opcode == "INPUT") execInput(cpu, operands);
    else if (opcode == "DISPLAY") execDisplay(cpu, operands);
    else if (opcode == "SHL" || opcode == "SHR" || opcode == "ROL" || opcode == "ROR") execShiftRotate(cpu, opcode, operands);
    else {
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
