#include <iostream>
#include <fstream>
#include <cstdio>

using namespace std;

// ==========================================
// FONG WEI HONG 253UC25627 - Member 1
// ZAVIER LAY JIUN HAO - Member 2
// JOY YU WEN 2521UC243S6 - Member 3
// NUR DAMIA ADLINA BINTI KAMARULAZIZI 242UC244TE - Member 4
// ==========================================

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

// FONG WEI HONG 253UC25627 - Member 1
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

// Forward declarations for classes built by other members
class Memory; 
class Instruction;

// ==========================================
// Zavier Lay Jiun Hao 253UC254FR - Member 2
// ==========================================

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
            cerr << "[CRASH] Stack overflow — stack is full.\n";
            exit(1);
        }
        top++;
        data[top] = val;
    }

    // pop(): remove and return top value
    signed char pop() {
        if (isEmpty()) {
            cerr << "[CRASH] Stack underflow — cannot pop from empty stack.\n";
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
            cerr << "[ERROR] Stack is empty — cannot peek.\n";
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
        delete data[i]; // free the Instruction object
        for (int j = i; j < size - 1; j++) {
            data[j] = data[j + 1]; // shift left
        }
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
            head = n; // first item — head and tail both point here
        }
        tail = n;
        sz++;
    }

    // dequeue(): remove and return the front string
    string dequeue() {
        if (isEmpty()) {
            cerr << "[ERROR] StringQueue is empty — cannot dequeue.\n";
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
            cerr << "[ERROR] StringQueue is empty — cannot peek.\n";
            exit(1);
        }
        return head->val;
    }

    // isEmpty(): true when no items in queue
    bool isEmpty() const { return head == nullptr; }

    // getSize(): how many items in queue
    int  getSize() const { return sz; }
};

// ==========================================
// Member 1
// CPU class — extended to wire in Member 2's classes
// ==========================================
class CPU {
private:
    GeneralRegister registers[8]; // R0 to R7
    unsigned char PC;             // Program Counter (starts at 0)
    unsigned char SI;             // Stack Index (starts at 0)
    
    // CPU aggregates FlagRegister (passed in via pointer)
    FlagRegister* flags;          
    
    // CPU composes Memory (owns its lifecycle)  — Member 2 activated this
    Memory* memory;

    // CPU composes StackDS (owns its lifecycle) — Member 2 added this
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

