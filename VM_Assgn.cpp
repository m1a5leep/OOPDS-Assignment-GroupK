#include <iostream>
#include "Instruction.h"
#include "CPU.h"
using namespace std;

// ==========================================
// FONG WEI HONG 253UC25627 - Member 1
// ZAVIER LAY JIUN HAO - Member 2
// JOY YU WEN 2521UC243S6 - Member 3
// NUR DAMIA ADLINA BINTI KAMARULAZIZI 242UC244TE - Member 4
// ==========================================


//==================================================
// Instruction
// │
// ├── ArithmeticInstruction
// │   ├── ADDInstruction
// │   ├── SUBInstruction
// │   ├── MULInstruction
// │   └── DIVInstruction
// │
// ├── INCInstruction
// ├── DECInstruction
// └── RESETInstruction

// ArithmeticIns (abstract)
//  ├── ADD
//  ├── SUB
//  ├── MUL
//  ├── DIV
//  ├── INC
//  ├── DEC
//  └── RESET

//# Member 3 — Instruction Hierarchy & Arithmetic
// ### Coding
// * Instruction base class
// * ArithmeticInstruction class
// * ADD
// * SUB
// * MUL
// * DIV
// * INC
// * DEC
// * RESET
//
// flags are by Member 1
//==================================================

//handle specific LIFO (Last-In, First-Out) stack or register-based fetching protocols.
class ArithmeticIns : public Instruction {
protected:
    virtual signed char compute(signed char a, signed char b) = 0;

public:
    //Stack architecture, Right-to-Left (First out is operand B)
    void execute(CPU& cpu) override {
        signed char b = cpu.stackPop(); 
        signed char a = cpu.stackPop();

        signed char result = compute(a, b);

        // Push the calculated value back to the stack
        cpu.stackPush(result);
    }
};

// my own
class ADDInstruction : public ArithmeticIns {
    protected:
        signed char compute(signed char a, signed char b) override {
            return a + b;
        }

    public:
        string getName() const override {
            return "ADD";
        }
};

class SUBInstruction : public ArithmeticIns {
    protected:
        signed char compute(signed char a, signed char b) override {
            return a - b;
        }
    
    public: 
        string getName() const override {
            return "SUB";
        }
};

class MULInstruction : public ArithmeticIns {
    protected:
        signed char compute(signed char a, signed char b) override {
            return a * b;
        }

    public:
        string getName() const override {
            return "MUL";
        }
};

class DIVInstruction : public ArithmeticIns {
    protected:
        signed char compute(signed char a, signed char b) override {
            //error handling
            if (b == 0){
                cerr << "[ERROR] DIV by zero\n";
                return 0;
            }
            //other outputs
                //print error
                //halt VM
                //throw exception

            return a / b;
        }

    public:
        string getName() const override {
            return "DIV";
        }
};

//Increment
class INCInstruction : public Instruction { 
    public:
        void execute(CPU& cpu) override {
            signed char value = cpu.stackPop();
            cpu.stackPush(value + 1);
        }

        string getName() const override {
            return "INC";
        }
}; 

//Decrement
class DECInstruction : public Instruction {
    public:
        void execute(CPU& cpu) override {
            signed char value = cpu.stackPop();
            cpu.stackPush(value - 1);
        }

    string getName() const override {
        return "DEC";
    }
};  

// //state/pointer reset 
// class RESETInstruction : public Instruction {
//     public:
//         void execute(CPU& cpu) override {
//            while (!cpu.stackIsEmpty()) {
//                cpu.stackPop(); or cpu.resetPC()
//         }
//     }
// }; 
