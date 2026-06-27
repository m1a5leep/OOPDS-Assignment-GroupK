#include <iostream>
using namespace std;
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
//==================================================

class VirtualMachine {
private:
    int* stack;
    int top;
    int capacity;

public:
    VirtualMachine(int size = 100){
        capacity = size;
        stack = new int[capacity];
        top = -1;
    }

    ~VirtualMachine() {
        delete[] stack;
    }

    void push(int value) {
        if (top < capacity - 1) {
            stack[++top] = value;
        }
    }

    int pop() {
        if (top >= 0) {
            return stack[top--];
        }

        //error handling
        return 0;
    }

    bool empty() {
        return top == -1;
    }
};

class VirtualMachine;

class Instruction {
public:
    virtual ~Instruction() {}

    virtual void execute(VirtualMachine& vm) = 0; //pure virtual function

};

//handle specific LIFO (Last-In, First-Out) stack or register-based fetching protocols.
class ArithmeticIns : public Instruction {
protected:
    virtual int compute(int a, int b) = 0;

public:
    //Stack architecture, Right-to-Left (First out is operand B)
    void execute(CPU& cpu) override {
        int b = cpu.stackPop(); 
        int a = cpu.stackPop();

        int result = compute(a, b);

        // Push the calculated value back to the stack
        cpu.stackPush(compute(a, b));
    }
};

// my own
class ADDInstruction : public ArithmeticIns {
    protected:
        int compute(int a, int b){
            return a + b;
        }
};

class SUBInstruction : public ArithmeticIns {
    protected:
        int compute(int a, int b){
            return a - b;
        }
};

class MULInstruction : public ArithmeticIns {
    protected:
        int compute(int a, int b){
            return a * b;
        }
};

class DIVInstruction : public ArithmeticIns {
    protected:
        int compute(int a, int b){
            //error handling
            if (b == 0)
                return 0;
            //other outputs
                //print error
                //halt VM
                //throw exception

            return a / b;
        }
};

//Increment
class INCInstruction : public Instruction { 
    public:
        void execute(VirtualMachine& vm) {
            int value = vm.pop();
            vm.push(value + 1);
        }
}; 

//Decrement
class DECInstruction : public Instruction {
    public:
        void execute(VirtualMachine& vm) {
            int value = vm.pop();
            vm.push(value - 1);
        }
};  

// //state/pointer reset 
// class RESETInstruction : public Instruction {
//     public:
//         void execute(VirtualMachine& vm){
//             vm.pop();
//             vm.push(0);
//         }
// }; 

class IOInstruction : public Instruction {};
class ShiftInstruct : public Instruction {};
class MemoryInstruct : public Instruction {};
class StackInstruct : public Instruction {};

class InstructionList {
    private:
      Instruction** data;
       int size ;
       int capacity
    
    public:
        add();
        get();
        removed();

}