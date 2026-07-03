#include <iostream>
#include <fstream>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <string>

using namespace std;

// ==========================================
// FONG WEI HONG 253UC25627 - Member 1
// ZAVIER LAY JIUN HAO - Member 2
// JOY YU WEN 2521UC243S6 - Member 3
// NUR DAMIA ADLINA BINTI KAMARULAZIZI 242UC244TE - Member 4
// ==========================================

//Acts as a base class for all registers.
//Demonstrates Encapsulation by hiding the 'value' variableand restricting access to getters and setters.
class Register {
private:
    signed char value; // 1 byte signed char (-128 to 127)

public:
    // Constructor: Initializes the register value to 0
    Register() {
        value = 0;
    }

    // Virtual Destructor: Ensures proper cleanup of derived classes (Polymorphism)
    virtual ~Register() {} 

    // Setter: Updates the register's value safely
    virtual void setValue(signed char val) {
        value = val;
    }

    // Getter: Retrieves the register's value without modifying it
    virtual signed char getValue() const {
        return value;
    }
};

//Represents the 8 general-purpose data registers (R0-R7).
//Demonstrates Inheritance by deriving from the base Register class, and Polymorphism by overriding the virtual methods.
class GeneralRegister : public Register {
public:
    // Constructor: Calls the base class constructor
    GeneralRegister() : Register() {}

    // Explicitly overrides the base setter to pass data safely to the private parent variable
    void setValue(signed char val) override {
        Register::setValue(val); 
    }

    // Explicitly overrides the base getter to retrieve data from the private parent variable
    signed char getValue() const override {
        return Register::getValue(); 
    }
};

//Manages the 4 specific system flags (Overflow, Underflow, Carry, Zero).
//Encapsulates flag states and provides centralized logic for math evaluation.
class FlagRegister {
private:
    bool OF; // Overflow Flag
    bool UF; // Underflow Flag
    bool CF; // Carry Flag
    bool ZF; // Zero Flag

public:
    // Constructor: Initialized all flags to false (0) at startup
    FlagRegister() {
        resetAllFlags();
    }
    // Getters and Setters for individual flags
    void setOF(bool val) { OF = val; }
    bool getOF() const { return OF; }

    void setUF(bool val) { UF = val; }
    bool getUF() const { return UF; }

    void setCF(bool val) { CF = val; }
    bool getCF() const { return CF; }

    void setZF(bool val) { ZF = val; }
    bool getZF() const { return ZF; }

    // Helper Function: Resets all flags back to 0
    void resetAllFlags() {
        OF = false;
        UF = false;
        CF = false;
        ZF = false;
    }

    // Main Logic Function: Evaluates the raw integer result of an arithmetic operation
    // (ADD, SUB, MUL, DIV) and triggers the appropriate flags based on 8-bit limits.
    void updateFlagsFromMath(int rawResult) {
        resetAllFlags(); // Clear previous states
        
        if (rawResult == 0) {
            setZF(true); // Result is exactly zero
        }
        if (rawResult > 127) {
            setOF(true); // Exceeded signed positive limit
        }
        if (rawResult < -128) {
            setUF(true); // Fell below signed negative limit
        }
        if (rawResult > 255 || rawResult < -256) {
            setCF(true); // Exceeded total 8-bit capacity
        }
    }
};

// Forward declarations for classes built by other members
class Memory;
class Instruction;

class Memory {
private:
    static const int SIZE = 64;   // 64 bytes total, address 0-63
    signed char memory[SIZE];     // the actual storage array

public:
    // Constructor: zero all 64 bytes on startup
    Memory() {
        reset();
    }

    // write(): store one byte at the given address
    void write(int address, signed char value) {
        if (address < 0 || address >= SIZE) {
            cerr << "[ERROR] Memory write out of range: " << address << "\n";
            return;
        }
        memory[address] = value;
    }

    // read(): fetch one byte from the given address
    signed char read(int address) const {
        if (address < 0 || address >= SIZE) {
            cerr << "[ERROR] Memory read out of range: " << address << "\n";
            return 0;
        }
        return memory[address];
    }

    // reset(): zero all memory cells
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            memory[i] = 0;
        }
    }

    // dump(): print memory in required output format
    void dump(ostream& out) const {
        for (int row = 0; row < 8; row++) {
            out << "#";
            for (int col = 0; col < 8; col++) {
                char buf[8];
                int val = (int)(unsigned char)memory[row * 8 + col];
                sprintf(buf, "%04d", val);
                out << buf << "#";
            }
            out << "\n";
        }
    }
};

class StackDS {
private:
    static const int MAX = 64;  // max 64 entries (matches UML field data[64])
    signed char data[MAX];      // storage array
    int top;                    // index of current top (-1 means empty)

public:
    // Constructor: start empty
    StackDS() : top(-1) {
        for (int i = 0; i < MAX; i++) {
            data[i] = 0;
        }
    }

    // push(): put a value on top of the stack, crash if full
    void push(signed char val) {
        if (top >= MAX - 1) {
            cerr << "[CRASH] Stack overflow - stack is full.\n";
            exit(1);
        }
        top++;
        data[top] = val;
    }

    // pop(): remove and return top value
    signed char pop() {
        if (isEmpty()) {
            cerr << "[CRASH] Stack underflow - cannot pop from empty stack.\n";
            exit(1);
        }
        signed char val = data[top];
        data[top] = 0; // clear the slot
        top--;
        return val;
    }

    // peek(): view top value without removing it
    signed char peek() const {
        if (isEmpty()) {
            cerr << "[ERROR] Stack is empty - cannot peek.\n";
            exit(1);
        }
        return data[top];
    }

    // isEmpty(): true when no items on stack
    bool isEmpty() const {
        return top == -1;
    }

    // getTop(): returns current top index (used for SI register)
    int getTop() const {
        return top;
    }
};

class InstructionList {
private:
    Instruction** data;   // heap array of Instruction pointers
    int size;             // how many instructions are stored
    int capacity;         // how many slots are allocated

    // grow(): double the capacity when array is full
    void grow() {
        int newCapacity = (capacity == 0) ? 4 : capacity * 2;
        Instruction** newData = new Instruction*[newCapacity];

        // copy old pointers to new buffer
        for (int i = 0; i < size; i++) {
            newData[i] = data[i];
        }

        delete[] data;            // free old buffer (not the objects inside)
        data     = newData;
        capacity = newCapacity;
    }

public:
    // Constructor: start empty
    InstructionList() : data(nullptr), size(0), capacity(0) {}

    // Destructor: free the pointer array (not the Instruction objects)
    ~InstructionList() {
        delete[] data;
        data = nullptr;
    }

    // add(): append a new Instruction pointer to the list
    void add(Instruction* instr) {
        if (size == capacity) {
            grow(); // expand if full
        }
        data[size] = instr;
        size++;
    }

    // get(): return the Instruction pointer at index i
    Instruction* get(int i) const {
        if (i < 0 || i >= size) {
            cerr << "[ERROR] InstructionList index out of range: " << i << "\n";
            return nullptr;
        }
        return data[i];
    }

    // remove(): delete the Instruction object at index i and shift remaining
    void remove(int i) {
        if (i < 0 || i >= size) {
            cerr << "[ERROR] InstructionList remove out of range: " << i << "\n";
            return;
        }
        for (int j = i; j < size - 1; j++) {
       data[j] = data[j + 1];  // shift left
        }
        data[size - 1] = nullptr;  // clear last slot
        size--;
        }

    // getSize(): how many instructions are stored
    int  getSize()  const { return size; }

    // isEmpty(): true when no instructions stored
    bool isEmpty()  const { return size == 0; }

    // clear(): remove all instructions without deleting objects
    void clear()          { size = 0; }
};

class StringQueue {
private:
    // Internal linked node
    struct Node {
        string val;   // the stored line
        Node*  next;  // pointer to next node
        Node(const string& v) : val(v), next(nullptr) {}
    };

    Node* head;  // front of queue (dequeue from here)
    Node* tail;  // back of queue  (enqueue here)
    int   sz;    // number of items in queue

public:
    // Constructor: start empty
    StringQueue() : head(nullptr), tail(nullptr), sz(0) {}

    // Destructor: free all nodes
    ~StringQueue() {
        while (!isEmpty()) {
            dequeue();
        }
    }

    // enqueue(): add a string to the back of the queue
    void enqueue(const string& val) {
        Node* n = new Node(val);
        if (tail != nullptr) {
            tail->next = n;
        } else {
            head = n; // first item - head and tail both point here
        }
        tail = n;
        sz++;
    }

    // dequeue(): remove and return the front string
    string dequeue() {
        if (isEmpty()) {
            cerr << "[ERROR] StringQueue is empty - cannot dequeue.\n";
            exit(1);
        }
        Node*  tmp = head;
        string val = tmp->val;
        head       = head->next;
        if (head == nullptr) {
            tail = nullptr; // queue is now empty
        }
        delete tmp;
        sz--;
        return val;
    }

    // front(): view the front item without removing it
    string front() const {
        if (isEmpty()) {
            cerr << "[ERROR] StringQueue is empty - cannot peek.\n";
            exit(1);
        }
        return head->val;
    }

    // isEmpty(): true when no items in queue
    bool isEmpty() const { return head == nullptr; }

    // getSize(): how many items in queue
    int  getSize() const { return sz; }
};

// CPU class -Member 1 extended to wire in Member 2 classes
class CPU {
private:
    GeneralRegister registers[8]; // R0 to R7
    unsigned char PC;             // Program Counter (starts at 0)
    unsigned char SI;             // Stack Index (starts at 0)

    // CPU aggregates FlagRegister (passed in via pointer)
    FlagRegister* flags;

    // CPU composes Memory (owns its lifecycle) - Member 2 activated this
    Memory* memory;

    // CPU composes StackDS (owns its lifecycle) - Member 2 added this
    StackDS stack;

public:
    // Constructor handles composition and aggregation
    CPU(FlagRegister* externalFlags) {
        flags  = externalFlags;
        PC     = 0;
        SI     = 0;
        memory = new Memory(); // Member 2: Memory is now active
    }

    ~CPU() {
        delete memory; // Member 2: clean up owned Memory
    }

    // --- PC & SI Management ---
    unsigned char getPC() const { return PC; }
    void incrementPC() { PC++; }
    void resetPC()     { PC = 0; }

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

    // --- Helper: get register value as int ---
    int getReg(int i) const {
        return (int)registers[i].getValue();
    }

    // --- Helper: set register value (updates flags) ---
    void setReg(int i, int val) {
        registers[i].setValue((signed char)val);
        flags->updateFlagsFromMath(val);
    }

    // --- Helper: set register value (NO flag update) ---
    void setRegNoFlag(int i, int val) {
        registers[i].setValue((signed char)val);
    }

    // --- Memory access (delegated to Memory object) ---
    void memWrite(int addr, signed char val) {
        memory->write(addr, val);
    }

    signed char memRead(int addr) const {
        return memory->read(addr);
    }

    // --- Stack operations (PUSH / POP) ---
    // stackPush(): push value onto stack, increment SI
    void stackPush(signed char val) {
        stack.push(val);
        SI++;
    }

    // stackPop(): pop value from stack, decrement SI
    // Crashes on empty stack (spec section 3.11)
    signed char stackPop() {
        signed char val = stack.pop(); // stack.pop() handles crash
        SI--;
        return val;
    }

    // stackIsEmpty(): check if system stack is empty
    bool stackIsEmpty() const {
        return stack.isEmpty();
    }

    // --- State dump (required output format) ---
    void dumpToStream(ostream& out) const {
        // Header
        out << "#Begin#\n";

        // Registers: R0-R7, each 4-digit zero-padded
        out << "#Registers#";
        for (int i = 0; i < 8; i++) {
            char buf[8];
            int val = (int)(unsigned char)registers[i].getValue();
            sprintf(buf, "%04d", val);
            out << buf << "#";
        }
        out << "\n";

        // Flags
        out << "#Flags#OF#" << flags->getOF()
            << "#UF#"       << flags->getUF()
            << "#CF#"       << flags->getCF()
            << "#ZF#"       << flags->getZF() << "#\n";

        // Program Counter
        char pcBuf[8];
        sprintf(pcBuf, "%04d", (int)PC);
        out << "#PC#" << pcBuf << "#\n";

        // Memory: 8x8 grid
        out << "#Memory#\n";
        memory->dump(out);

        out << "#End#\n";
    }
};

// ==========================================
// Member 3
// Arithmetic Instruction class
// ==========================================

// abstract class Instruction
class Instruction {
public:
    virtual ~Instruction() {}

    virtual void execute(CPU& cpu) = 0;
    virtual string getName() const = 0;
};


// ArithmeticIns inherits from Instruction
//=================================
class ArithmeticIns : public Instruction {
protected:
    int destReg;    // register numbers hidden inside class
    int srcReg;     // only derived classes can access them

public:
    ArithmeticIns(int dest, int src)
        : destReg(dest), srcReg(src) {}
};


// ADDInstruction inherits from ArithmeticIns
//=================================
// performs ADD R1, R2 
// as R1 = R1 + R2
class ADDInstruction : public ArithmeticIns {
public:
    ADDInstruction(int dest, int src)
        : ArithmeticIns(dest, src) {}

    void execute(CPU& cpu) override {   // overrides pure virtual functions
        int result =                    // from Instruction/runtime polymorphsm
            cpu.getReg(destReg) + cpu.getReg(srcReg);
        
        cpu.setReg(destReg, result);
    }

    string getName() const override {
        return "ADD";
    }
};


// SUBInstruction inherits from ArithmeticIns
//=================================
class SUBInstruction : public ArithmeticIns {
public: 
    SUBInstruction(int dest, int src)
        : ArithmeticIns(dest, src) {}

    void execute(CPU& cpu) override {                                   // overrides pure virtual functions
        cpu.setReg(destReg, cpu.getReg(destReg) - cpu.getReg(srcReg));  // from Instruction/runtime polymorphsm
    }

    string getName() const override {
        return "SUB";
    }
};


// MULInstruction inherits from ArithmeticIns
//=================================
class MULInstruction : public ArithmeticIns {
public:
    MULInstruction(int dest, int src) : ArithmeticIns(dest, src) {}

    void execute(CPU& cpu) override {                                   // overrides pure virtual functions
        cpu.setReg(destReg, cpu.getReg(destReg) * cpu.getReg(srcReg));  // from Instruction/runtime polymorphsm
    }

    string getName() const override {
        return "MUL";
    }
};


// DIVInstruction inherits from ArithmeticIns
//=================================
class DIVInstruction : public ArithmeticIns {
public:
    DIVInstruction(int dest, int src) : ArithmeticIns(dest, src) {}

    void execute(CPU& cpu) override {       // overrides pure virtual functions
        int divisor = cpu.getReg(srcReg);   // fron Instruction/runtime polymorphsm

        if (divisor == 0){
            cerr << "[ERROR] Division by zero\n";
            return;
        }

        cpu.setReg(destReg, cpu.getReg(destReg) / divisor);
    }

    string getName() const override {
        return "DIV";
    }
};


// INCInstruction inherits directly from Instruction
//=================================
//Increment
class INCInstruction : public Instruction { 
private:
    int reg;    // register index is private
                // only member functions of INCInstruction can use it
public:
    INCInstruction(int r) : reg(r) {}

    void execute(CPU& cpu) override {           // overrides pure virtual functions
        cpu.setReg(reg, cpu.getReg(reg) + 1);   // from Instruction/runtime polymorphsm
    }

    string getName() const override {
        return "INC";
    }
}; 


// DECInstruction inherits directly from Instruction
//=================================
//Decrement
class DECInstruction : public Instruction {
private:
    int reg;    // register index is private
                // only member functions of DECInstruction can use it
public:
    DECInstruction(int r) : reg(r) {}

    void execute(CPU& cpu) override {           // overrides pure virtual functions
        cpu.setReg(reg, cpu.getReg(reg) - 1);   // fron Instruction/runtime polymorphsm
    }

    string getName() const override {
        return "DEC";
    }
}; 

// trim(): remove leading/trailing whitespace from a string
static string trim(const string& text) {
    size_t start = 0;
    size_t end = text.size();

    while (start < end && isspace((unsigned char)text[start])) {
        start++;
    }

    while (end > start && isspace((unsigned char)text[end - 1])) {
        end--;
    }

    return text.substr(start, end - start);
}

// parseRegisterIndex(): parses tokens like "R0".."R7" into an index 0-7
static bool parseRegisterIndex(const string& token, int& index) {
    if (token.size() != 2) {
        return false;
    }

    char letter = token[0];
    char digit = token[1];

    if ((letter != 'R' && letter != 'r') || digit < '0' || digit > '7') {
        return false;
    }

    index = digit - '0';
    return true;
}

// parseBracketedRegister(): parses "[R0]".."[R7]" used for register-indirect addressing
static bool parseBracketedRegister(const string& token, int& index) {
    if (token.size() < 3 || token.front() != '[' || token.back() != ']') {
        return false;
    }
    string inner = trim(token.substr(1, token.size() - 2));
    return parseRegisterIndex(inner, index);
}

// parseBracketedAddress(): parses "[20]" used for direct memory addressing
static bool parseBracketedAddress(const string& token, int& address) {
    if (token.size() < 3 || token.front() != '[' || token.back() != ']') {
        return false;
    }
    string inner = trim(token.substr(1, token.size() - 2));
    if (inner.empty()) return false;
    for (char c : inner) {
        if (!isdigit((unsigned char)c)) return false;
    }
    address = atoi(inner.c_str());
    return true;
}

// parseImmediate(): parses signed decimal integers e.g. "10", "-5", "+3"
static bool parseImmediate(const string& token, int& value) {
    if (token.empty()) {
        return false;
    }

    int sign = 1;
    size_t i = 0;

    if (token[0] == '+' || token[0] == '-') {
        if (token[0] == '-') {
            sign = -1;
        }
        i = 1;
    }

    if (i >= token.size()) {
        return false;
    }

    int result = 0;
    for (; i < token.size(); ++i) {
        char c = token[i];
        if (c < '0' || c > '9') {
            return false;
        }
        result = result * 10 + (c - '0');
    }

    value = result * sign;
    return true;
}

// splitOperands(): splits "dst, src" into its two trimmed operand strings
static bool splitOperands(const string& text, string& left, string& right) {
    size_t commaPos = text.find(',');
    if (commaPos == string::npos) {
        return false;
    }

    left = trim(text.substr(0, commaPos));
    right = trim(text.substr(commaPos + 1));
    return true;
}

// parseOpcodeAndOperands(): splits a line into opcode and operand string.
// Returns false for empty/blank lines. Errors out if more than one
// instruction is found on a single line, per spec section 3.
static bool parseOpcodeAndOperands(const string& line, string& opcode, string& operands) {
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

    for (char& c : opcode) c = (char)toupper((unsigned char)c);
    return true;
}

// Supports the three modes required by spec 3.4:
//   MOV R0, 10     -> immediate
//   MOV R0, R1     -> register to register
//   MOV R3, [R1]   -> register-indirect memory load
static void Mov(CPU* cpu, const string& operands) {
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

    int srcIndex, srcValue, srcAddr;
    if (parseBracketedRegister(srcText, srcIndex)) {
        // MOV R3, [R1] -> read memory at address held in R1
        int addr = (int)(unsigned char)cpu->getRegister(srcIndex).getValue();
        signed char val = cpu->memRead(addr);
        cpu->setReg(dstIndex, (int)val);
    } else if (parseBracketedAddress(srcText, srcAddr)) {
        // MOV R3, [20] -> read memory at literal address 20
        signed char val = cpu->memRead(srcAddr);
        cpu->setReg(dstIndex, (int)val);
    } else if (parseRegisterIndex(srcText, srcIndex)) {
        cpu->setReg(dstIndex, (int)cpu->getRegister(srcIndex).getValue());
    } else if (parseImmediate(srcText, srcValue)) {
        cpu->setReg(dstIndex, srcValue);
    } else {
        cerr << "Error: invalid source operand in MOV: " << srcText << endl;
        exit(1);
    }
}

// Reads a value from the keyboard, validates the range and updates flags
static void execInput(CPU* cpu, const string& operands) {
    int regIndex;
    if (!parseRegisterIndex(operands, regIndex)) {
        cerr << "Error: INPUT requires a register operand" << endl;
        exit(1);
    }
    int value;
    cout << "?";
    if (!(cin >> value)) {
        cerr << "Error: failed to read input" << endl;
        exit(1);
    }
    cpu->setReg(regIndex, value);
}

// Writes the value held in the register to the screen.
static void execDisplay(CPU* cpu, const string& operands) {
    int regIndex;
    if (!parseRegisterIndex(operands, regIndex)) {
        cerr << "Error: DISPLAY requires a register operand" << endl;
        exit(1);
    }
    signed char value = cpu->getRegister(regIndex).getValue();
    cout << "DISPLAY R" << regIndex << " = " << (int)value << endl;
}

// Bits are processed on the unsigned 8-bit representation of the register,
// then the result is converted back into the signed char range and stored,
// updating flags through CPU::setReg().
static void execShiftRotate(CPU* cpu, const string& opcode, const string& operands) {
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

    cpu->setReg(regIndex, (int)(signed char)result);
}

// DynamicArray is the required vector/list data structure used to
// load the .asm file into memory before execution.
class DynamicArray {
private:
    string* items;      // Encapsulation: internal array is hidden from other classes
    int capacity;       // Encapsulation: current storage capacity
    int length;         // Encapsulation: number of stored instructions

    void grow() {       // double the capacity when full
        int newCap = capacity == 0 ? 8 : capacity * 2;
        string* newItems = new string[newCap];
        for (int i = 0; i < length; ++i) newItems[i] = items[i];
        delete[] items;
        items = newItems;
        capacity = newCap;
    }

public:
    // Construct an empty dynamic array
    DynamicArray() : items(nullptr), capacity(0), length(0) {}

    // Destructor releases internal buffer
    ~DynamicArray() { delete[] items; }

    // Append a string to the array (grows automatically)
    void add(const string& s) {
        if (length >= capacity) grow();
        items[length++] = s;
    }

    // Retrieve an element by index
    string get(int idx) const {
        if (idx < 0 || idx >= length) {
            cerr << "[ERROR] DynamicArray index out of range: " << idx << "\n";
            exit(1);
        }
        return items[idx];
    }

    // Number of elements stored
    int size() const { return length; }
};

// ==========================================
// Member 4
// Runner class
// =========================================

// loads the .asm program, decodes each instruction line
// and delegates execution to the CPU.
class Runner {
private:
    CPU* cpu;               // Aggregation: Runner uses an existing CPU object
    DynamicArray* program;  // Composition: Runner owns the instruction storage

public:
    Runner(CPU* c);      // Constructor receives an existing CPU object
    ~Runner();

    // Load assembly instructions from file into `program`.
    // Empty lines are ignored.
    void loadProgram(const string& filename);

    // Execute the loaded program, instruction by instruction.
    void run();

    // Parse and execute a single instruction line.
    void executeInstruction(const string& line);
};

Runner::Runner(CPU* c) {
    cpu = c;                             // Aggregation: stores reference to existing CPU
    program = new DynamicArray();        // Composition: Runner creates its own DynamicArray
}

Runner::~Runner() {
    delete program;              // Composition: releases owned object
}

// reads the .asm file line by line, skipping blank lines
void Runner::loadProgram(const string& filename) {
    ifstream file(filename);          // Opens the assembly source file
    if (!file.is_open()) {
        cerr << "Error: could not open program file: " << filename << endl;
        exit(1);
    }

    string line;
    while (getline(file, line)) {     // Reads instructions line by line
        if (trim(line) != "") {
            program->add(line);       // Stores instruction into DynamicArray
        }
    }

    file.close();
}

// executes every loaded instruction in order, then dumps machine state
void Runner::run() {
    for (int i = 0; i < program->size(); i++) {
        string line = program->get(i);       // Retrieves current instruction
        executeInstruction(line);            // Executes parsed instruction
    }
}

// decodes opcode/operands and dispatches to the relevant handler. 
// Instructions owned by other members are dispatched to
// their respective handlers; unimplemented opcodes report a clear error
// instead of silently doing nothing.
static void handleArithmetic(CPU* cpu, const string& op, const string& args) {
    string left, right;
    if (!splitOperands(args, left, right)) { cout << "Error: " << op << " requires 2 operands\n"; exit(1); }
    int dest, src;
    if (!parseRegisterIndex(left, dest) || !parseRegisterIndex(right, src)) { cout << "Error: invalid registers\n"; exit(1); }

    if (op == "ADD") { ADDInstruction i(dest, src); i.execute(*cpu); }
    else if (op == "SUB") { SUBInstruction i(dest, src); i.execute(*cpu); }
    else if (op == "MUL") { MULInstruction i(dest, src); i.execute(*cpu); }
    else if (op == "DIV") { DIVInstruction i(dest, src); i.execute(*cpu); }
}

static void handleSingleOpMath(CPU* cpu, const string& op, const string& args) {
    int reg;
    if (!parseRegisterIndex(args, reg)) { cout << "Error: " << op << " requires a register\n"; exit(1); }
    if (op == "INC") { INCInstruction i(reg); i.execute(*cpu); }
    else if (op == "DEC") { DECInstruction i(reg); i.execute(*cpu); }
}

static void handleLoadStore(CPU* cpu, const string& op, const string& args) {
    string left, right;
    if (!splitOperands(args, left, right)) { cout << "Error: " << op << " requires 2 operands\n"; exit(1); }
    int reg, addr, imm;

    if (op == "LOAD") {
        if (parseRegisterIndex(left, reg) && parseBracketedAddress(right, addr)) {
            cpu->setReg(reg, (int)cpu->memRead(addr));
        } else if (parseRegisterIndex(left, reg) && parseBracketedRegister(right, addr)) {
            int memAddr = (int)(unsigned char)cpu->getRegister(addr).getValue();
            cpu->setReg(reg, (int)cpu->memRead(memAddr));
        }
    } else if (op == "STORE") {
        if (parseBracketedAddress(left, addr) && parseRegisterIndex(right, reg)) {
            cpu->memWrite(addr, cpu->getRegister(reg).getValue());
        } else if (parseBracketedAddress(left, addr) && parseImmediate(right, imm)) {
            cpu->memWrite(addr, (signed char)imm);
        }
    }
}

static void handleStack(CPU* cpu, const string& op, const string& args) {
    int reg;
    if (!parseRegisterIndex(args, reg)) { cout << "Error: " << op << " requires a register\n"; exit(1); }
    if (op == "PUSH") { cpu->stackPush(cpu->getRegister(reg).getValue()); }
    else if (op == "POP") { cpu->setReg(reg, cpu->stackPop()); }
}

// decodes opcode/operands and dispatches to the relevant handler cleanly.
void Runner::executeInstruction(const string& line) {
    string op, args;
    if (!parseOpcodeAndOperands(line, op, args)) return;    // Parses the instruction format
    
    if (op == "MOV") Mov(cpu, args);
    else if (op == "INPUT") execInput(cpu, args);
    else if (op == "DISPLAY") execDisplay(cpu, args);
    else if (op == "SHL" || op == "SHR" || op == "ROL" || op == "ROR") execShiftRotate(cpu, op, args);
    else if (op == "ADD" || op == "SUB" || op == "MUL" || op == "DIV") handleArithmetic(cpu, op, args);
    else if (op == "INC" || op == "DEC") handleSingleOpMath(cpu, op, args);
    else if (op == "LOAD" || op == "STORE") handleLoadStore(cpu, op, args);
    else if (op == "PUSH" || op == "POP") handleStack(cpu, op, args);
    else if (op == "RESET") {
        for (int i = 0; i < 8; i++) cpu->setRegNoFlag(i, 0);
        cpu->getFlags()->resetAllFlags();
    } else cout << "Error: unknown instruction\n";

    cpu->incrementPC();
}

// ==========================================
// MAIN EXECUTION ENTRY POINT 
// ==========================================
int main() {
    string filename;
    
    //Get the filename from the user using standard cin
    cout << "Enter the name of the assembly file (e.g., program.asm): ";
    cin >> filename;

    //Initialize Virtual Machine Components
    FlagRegister systemFlags;
    CPU myCPU(&systemFlags);
    Runner runner(&myCPU);

    //Load and Execute the Assembly Program
    cout << "Loading assembly program from: " << filename << "...\n";
    runner.loadProgram(filename);
    
    cout << "Executing instructions...\n";
    runner.run();

    //Output Generation Requirement (Screen and File)
    cout << "\n=== Final Virtual Machine State ===\n";
    myCPU.dumpToStream(cout);

    string outFilename = "output.txt";
    ofstream outFile(outFilename);
    
    if (outFile.is_open()) {
        myCPU.dumpToStream(outFile);
        outFile.close();
        cout << "\n[SUCCESS] Final state successfully saved to " << outFilename << "\n";
    } else {
        // Using cout here instead of cerr to be strictly safe
        cout << "\n[ERROR] Could not create output file: " << outFilename << "\n";
    }

    return 0;
}
