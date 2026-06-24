class CPU;
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