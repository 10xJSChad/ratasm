#include <fstream>
#include <iostream>
#include <vector>
#include <map>


using namespace std;


struct Instruction {
    string operation;
    string left;
    string right;
};


enum Operation {
    INVALID = 0,
    MOV, ADD, SUB,
    MUL, DIV,
    JMP,
    CMP,
    JE,  JG,  JL,
    JNE, JGE, JLE,
    LABEL,
    PRINTINT, PRINTCHAR
};


map<string, Operation> op_table = {
    {"mov", MOV},
    {"add", ADD},
    {"sub", SUB},
    {"mul", MUL},
    {"div", DIV},
    {"jmp", JMP},
    {"cmp", CMP},
    {"je",  JE},
    {"jg",  JG},
    {"jl",  JL},
    {"jne",  JNE},
    {"jge", JGE},
    {"jle", JLE},
    {"printint", PRINTINT},
    {"printchar", PRINTCHAR}
};


string 
strip(string s) {
    string s_stripped = "";
    int len = s.length();
    int left = 0;
    int right = len;

    while (isspace(s[left]))
        if (++left >= len)
            exit(1);
    
    while (isspace(s[right]))
        if (--right <= 0)
            exit(1);

    return s.substr(left, right - left + 1);
}


Instruction
parse_line_to_instruction(string line) {
    enum Step {
        OPERATION,
        LEFT,
        RIGHT,
        INVALID
    };

    Instruction instruction;
    string curr = "";
    Step step = OPERATION;

    for (int i = 0; i < line.length(); ++i) {
        if (!isspace(line[i]))
            curr += line[i];
        
        if (isspace(line[i]) || i == line.length() - 1) {
            if (curr != "") {
                switch (step)
                {
                    case OPERATION:
                        instruction.operation = curr;
                        step = LEFT;
                        break;

                    case LEFT:
                        instruction.left = curr;
                        step = RIGHT;
                        break;

                    case RIGHT:
                        instruction.right = curr;
                        step = INVALID;
                        break;
                    
                    case INVALID:
                        cout << "Invalid step";
                        exit(1);

                    default:
                        exit(1);
                }

                curr = "";
            }

            continue;
        }
    }

    return instruction;
}


vector<Instruction>
parse_lines_to_instructions(vector<string> lines) {
    vector<Instruction> instructions = {};

    for (auto line : lines)
        instructions.push_back(parse_line_to_instruction(line));

    return instructions;
}


vector<string> 
format_code(string code) {
    vector<string> formatted_code = {};
    string curr = "";

    for (auto c : code) {
        if (c == '\n') {
            formatted_code.push_back(strip(curr));
            curr = "";
            continue;
        }

        curr += c;
    }

    if (curr != "")
        formatted_code.push_back(strip(curr));

    return formatted_code;
}


bool 
is_valid_label(string label) {
    int len = label.length();
    
    if (len < 2)
        return false;

    for (int i = 0; i < len - 1; ++i)
        if (!isalnum(label[i]) && label[i] != '_')
            return false;

    return label[len - 1] == ':';
}


bool
is_valid_operation(string operation) {
    return op_table.count(operation) == 1;
}


string 
strip_label(string label) {
    label.pop_back();
    return label;
}


map<string, int>
create_label_table(vector<Instruction>& instructions) {
    int len = instructions.size();
    map<string, int> table = {};

    /* parse for labels */
    for (int i = 0; i < len; ++i) {
        string operation = instructions[i].operation;
        if (!is_valid_operation(operation)) {
            if (!is_valid_label(operation)) {
                cout << "Invalid operation: " << operation << endl;
                exit(1);
            }

            table.insert({strip_label(operation), i});
        }
    }

    return table;
}

class VirtualMachine {
private:
    map<string, int> label_table;
    int idx; /* instruction index */;
    
    int r1, r2, r3, r4, r5, literal;
    int zf, cf;
    

    int label_index(string label) {
        try {
            return label_table.at(label);
        } catch (const out_of_range& e) {
            cout << "Label not found " << label << endl;
            exit(1);
        }
    }


    bool is_valid_number(string s) {
        for (char c : s)
            if (!isdigit(c))
                return false;
        
        return true;
    }


    int*
    parse_operand(string operand, bool allow_literal) {
        if (operand == "r1") return &r1;
        if (operand == "r2") return &r2;
        if (operand == "r3") return &r3;
        if (operand == "r4") return &r4;
        if (operand == "r5") return &r5;

        if (allow_literal && is_valid_number(operand)) {
            literal = stoi(operand);
            return &literal;
        } else {
            cout << "Invalid operand: " << operand << endl;
            exit(1);
        }
    }


    void mov(Instruction instruction) {
        *parse_operand(instruction.left, false) = *parse_operand(instruction.right, true); 
    }

    void add(Instruction instruction) {
        *parse_operand(instruction.left, false) += *parse_operand(instruction.right, true); 
    }

    void sub(Instruction instruction) {
        *parse_operand(instruction.left, false) -= *parse_operand(instruction.right, true); 
    }

    void mul(Instruction instruction) {
        int result = (*parse_operand(instruction.left, true)) * (*parse_operand(instruction.right, true));
        r5 = result;
    }

    void div(Instruction instruction) {
        int result = (*parse_operand(instruction.left, true)) / (*parse_operand(instruction.right, true));
        r5 = result;
    }

    void cmp(Instruction instruction) {
        long result = (*parse_operand(instruction.left, true)) - (*parse_operand(instruction.right, true));
        zf = (result == 0);
        cf = (result < 0);
    }

    void jmp(Instruction instruction) {
        idx = label_index(instruction.left);
    }

    void jl(Instruction instruction) {
        if (cf)
            idx = label_index(instruction.left);
    }

    void jg(Instruction instruction) {
        if (!cf && !zf)
            idx = label_index(instruction.left);
    }

    void je(Instruction instruction) {
        if (zf)
            idx = label_index(instruction.left);
    }

    void jne(Instruction instruction) {
        if (!zf)
            idx = label_index(instruction.left);
    }

    void jge(Instruction instruction) {
        if (zf || !cf)
            idx = label_index(instruction.left);
    }

    void jle(Instruction instruction) {
        if (zf || cf)
            idx = label_index(instruction.left);
    }

    void printint(Instruction instruction) {
        cout << *parse_operand(instruction.left, true);
    }

    void printchar(Instruction instruction) {
        cout << (char) (*parse_operand(instruction.left, true));
    }


public:
    void 
    execute(vector<Instruction> instructions) {
        int len = instructions.size();
        label_table = create_label_table(instructions);

        for (idx = 0; idx < len; ++idx) {
            Instruction instruction = instructions[idx];
            
            /* exit if operation not found in op table and is not a recognized label */
            if (!is_valid_operation(instruction.operation)) {
                if (is_valid_label(instruction.operation))
                    continue;
                
                cout << "Invalid operation: " << instruction.operation << endl;
                exit(1);
            }

            switch(op_table.at(instruction.operation))
            {
                case MOV: mov(instruction); break;
                case ADD: add(instruction); break;
                case SUB: sub(instruction); break;
                case MUL: mul(instruction); break;
                case DIV: div(instruction); break;
                case CMP: cmp(instruction); break;

                case JMP: jmp(instruction); break;

                case JE: je(instruction); break;
                case JL: jl(instruction); break;
                case JG: jg(instruction); break;

                case JNE: jne(instruction); break;
                case JLE: jle(instruction); break;
                case JGE: jge(instruction); break;

                case PRINTINT:  printint(instruction); break;
                case PRINTCHAR: printchar(instruction); break;

                default:
                    cout << "Unimplemented operation: " << instruction.operation << endl;
                    exit(1);
            }
        }
    }
};


int 
main(int argc, char **argv) {
    if (argc == 1) {
        cout << "No filename provided" << endl;
        exit(1);
    }

    string filename = string(argv[1]);
    ifstream ifs(filename);
    string content((istreambuf_iterator<char>(ifs)),
                    (istreambuf_iterator<char>()));

    VirtualMachine vm;
    vm.execute(parse_lines_to_instructions(format_code(content)));    
    return 0;
}
