#include <iostream>

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

// FONG WEI HONG 253UC25627 - Member 1
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