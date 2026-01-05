#include <bits/stdc++.h>
#include <iomanip>
using namespace std;

    const uint32_t offset = 0x80000000;
    uint32_t pc = offset;
    const uint32_t MEM_SIZE = 128 * 1024;

    uint32_t mstatus = 0;
    uint32_t mie = 0;     
    uint32_t mtvec = 0;   
    uint32_t mepc = 0;    
    uint32_t mcause = 0;  
    uint32_t mtval = 0;   
    uint32_t mip = 0;
    uint64_t mtime = 0;         
    uint64_t mtimecmp = -1;
    uint8_t uart_buffer = 0;      // O dado que chegou (Byte da UART)
    bool uart_pendente = false;   // Indica que o PLIC recebeu um sinal do dispositivo

    const uint32_t enderecoUart  = 0x10000000;
    const uint32_t enderecoPlic = 0x0C200004; // Endereço de Claim/Complete 
    const uint32_t IDuard       = 10;

    #define CSR_ADDR_MSTATUS 0x300
    #define CSR_ADDR_MIE 0x304
    #define CSR_ADDR_MTVEC 0x305
    #define CSR_ADDR_MEPC 0x341
    #define CSR_ADDR_MCAUSE 0x342
    #define CSR_ADDR_MTVAL 0x343
    #define CSR_ADDR_MIP 0x344

    #define instruction_access_fault  0x1  
    #define illegal_instruction 0x2  
    #define load_access_fault 0x5  
    #define store_access_fault 0x7
    #define ecall_Mmode 0xb

void trap(uint32_t cause, uint32_t value) {
    mcause = cause;
    mepc = pc;
    mtval = value;

    
    uint32_t mie_bit = (mstatus >> 3) & 1; // Salva o estado atual
    mstatus = (mstatus & ~0x88) | (mie_bit << 7); // Desliga MIE(3) e salva em MPIE(7)

    // Lógica do vetor...
    uint32_t mode = mtvec & 0x1;
    uint32_t base = mtvec & ~0x3;
    bool is_interrupt = (cause & 0x80000000);

    if (mode == 1 && is_interrupt) {
        pc = base + (4 * (cause & 0xFF));
    } else {
        pc = base;
    }
}

int main (int argc, char* argv[]) {
    
    cout << "-------------------------------------------------------\n";

     if(argc < 3) { // pra ver se a quantidade de parametro ta certinha 
    
        cerr << "Parametros Incorretos." << endl;
        return 1;
     }

    ifstream input(argv[1]);
    ofstream output(argv[2]);

    if(!input.is_open()){
        cerr << "Erro no Arquivo de Entrada." << endl;
        return 1;
    }

    if(!output.is_open()){
        cerr << "Erro no Arquivo de Saída." << endl;
        return 1;
    }

    streambuf *coutbuffer = cout.rdbuf();
    cout.rdbuf(output.rdbuf());

    vector<uint8_t> mem(32 * 1024, 0); // olha a  memoria ai pessoal
    FILE *infile = nullptr;
    FILE *outfile = nullptr;
    infile = fopen("qemu.terminal.in", "rb");   
    outfile = fopen("qemu.terminal.out", "wb");

    if (!infile) {
        cout << "[ERRO CRITICO] Nao consegui abrir qemu.terminal.in!" << endl;
       
    } 

    if (!outfile) {
        cerr << "Erro: Nao foi possivel criar qemu.terminal.out" << endl;
        return 1;
    }

    array<string, 32> regs { "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6" };

    vector<uint32_t> x(32, 0);

    string linha; 
    uint32_t index_memoria = 0;

    while (getline(input, linha)) {
        
        if (linha.empty() || linha[0] == '#') continue;

        if(linha[0] == '@') {
            string tirarroba = linha.substr(1);
            uint32_t novo_endereco = stoul(tirarroba, nullptr, 16);
            index_memoria = novo_endereco - offset;
            continue;
        }

        stringstream ss(linha); 
        uint32_t byte_val; // Variável para guardar CADA byte (6F, 00, 0A, etc.)

        
        while (ss >> hex >> byte_val) {
            
            // Armazena o byte lido diretamente na memória
            if (index_memoria < mem.size()) {
                mem[index_memoria] = static_cast<uint8_t>(byte_val);
                index_memoria++; 
            } else {
                cerr << "Erro: O arquivo estourou o limite da memória!" << endl;
                break;
            }
        }
    }

    bool run = true;
    
    while(run) {

        if(!uart_pendente && infile != nullptr) {
            int c = fgetc(infile);
            if(c != EOF) {
                uart_buffer = (uint8_t)c;
                uart_pendente = true;
            } 
        }

        if(uart_pendente) {
            mip = mip | 0x800;
        } else {
            mip = mip & (~0x800); 
        }
        mtime++;
        if (mtime >= mtimecmp) mip = mip | 0x80;
        else mip = mip & (~0x80);

        bool pedidoSoftware = mip&8;
        bool softwareHabilitado = mie&8;
        bool chaveGeral = (mstatus >> 3) & 1;
        bool pedidoTimer      = (mip & 128);   
        bool timerHabilitado = (mie & 128);

        if ((mip & 0x800) && (mie & 0x800) && chaveGeral) { 
            cout << ">interrupt:external                   cause=0x8000000b,epc=0x" 
                 << hex << setw(8) << setfill('0') << pc 
                 << ",tval=0x00000000" << endl;
            
            trap(0x8000000b, 0);

            uart_pendente = false; 
            mip &= ~0x800; 
            
            continue;
        }

        else if (pedidoSoftware && softwareHabilitado && chaveGeral) {

            cout << ">interrupt:software                   cause=0x80000003,epc=0x" 
                 << hex << setw(8) << setfill('0') << pc 
                 << ",tval=0x00000000" << endl;

            trap(0x80000003, 0);
            
            mip &= ~8; 

            continue;
        }

        else if (pedidoTimer && timerHabilitado && chaveGeral) {
            
            cout << ">interrupt:timer                   cause=0x80000007,epc=0x" 
                 << hex << setw(8) << setfill('0') << pc 
                 << ",tval=0x00000000" << endl;
            
            trap(0x80000007, 0); 
            continue;
        }

        if(pc < offset || pc >= offset + MEM_SIZE || (pc % 4 != 0)) {
            cout << hex << setfill('0');
            cout << ">exception:instruction_fault   cause=0x" << setw(8) << instruction_access_fault
            << ",epc=0x" << setw(8) << pc 
            << ",tval=0x" << setw(8) << pc // O valor ruim é o próprio PC
            << endl;
            cout << dec;
            trap(instruction_access_fault, pc);
            continue;
        } 

        uint32_t index_memoria = pc - offset;
        uint32_t instruction = (uint32_t)mem[index_memoria + 3] << 24 |
                               (uint32_t)mem[index_memoria + 2] << 16 |
                               (uint32_t)mem[index_memoria + 1] << 8  |
                               (uint32_t)mem[index_memoria + 0];

                               
		const uint8_t opcode = instruction & 0b1111111;
		const uint8_t func7 = instruction >> 25;
		const uint16_t imm = instruction >> 20;
		const uint8_t uimm = (instruction & (0b11111 << 20)) >> 20;
		const uint8_t rs1 = (instruction & (0b11111 << 15)) >> 15;
        const uint8_t rs2 = uimm;
		const uint8_t func3 = (instruction & (0b111 << 12)) >> 12;
        const uint8_t rd = (instruction & (0b11111 << 7)) >> 7;
        uint32_t imm_20 = instruction & 0xFFFFF000;
        const uint32_t imm20 = ((instruction >> 31) << 19) | (((instruction & (0b11111111 << 12)) >> 12) << 11) | (((instruction & (0b1 << 20)) >> 20) << 10) | ((instruction & (0b1111111111 << 21)) >> 21);
        
         switch (opcode) {
            case 0b0110011: { 
                if(func3 == 0b000 && func7 == 0b0000000) { // add
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = val1 + val2;

                    if (rd != 0) {
                        x[rd] = resultado;
                    }

                    cout << hex << setfill('0');
                    cout << hex << "0x" << pc << ":add      " << regs[rd] << "," << regs[rs1] << "," << regs[rs2] << "   " << regs[rd] << "=0x" << setw(8) <<  val1 << "+0x" 
                    << setw(8) << val2 << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                }
                else if(func3 == 0b000 && func7 == 0b0100000) { // sub
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = val1 - val2; 
                    
                    if(rd != 0) x[rd] = resultado;
                
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":sub      " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1
                    << "-0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                    
                    cout << dec;
                }

                else if(func3 == 0b100 && func7 == 0b0000000) { // xor

                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = val1 ^ val2;

                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":xor      " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << "^0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado 
                    << endl; 
                   
                    cout << dec;
                }

                else if(func3 == 0b110 && func7 == 0b0000000) { // or

                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = val1 | val2;

                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) <<  pc << ":or       " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << "|0x" << setw(8) << val2
                    << "=0x" << setw(8) << resultado 
                    << endl; 
                   
                    cout << dec;
                }

                else if (func3 == 0b111 && func7 == 0b0000000) { // and

                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = val1 & val2;

                    if(rd != 0) {
                        x[rd] = resultado; 
                    } 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":and      " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << "&0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado 
                    << endl; 
                   
                    cout << dec;
                }

                else if (func3 == 0b001 && func7 == 0b0000000) { //sll

                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t shamt = val2 & 0x1F;
                    uint32_t resultado = val1 << shamt;

                    if(rd != 0) {
                        x[rd] = resultado; 
                    }
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":sll      " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << "<<" << dec << shamt 
                    << "=0x" << hex << setw(8) << resultado 
                    << endl; 
                   
                    cout << dec;
                }

                else if (func3 == 0b101 && func7 == 0b0000000) { //srl

                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t shamt = val2 & 0x1F;
                    uint32_t resultado = val1 >> shamt;

                    if(rd != 0) {
                        x[rd] = resultado; 
                    }
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":srl      " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << ">>" << dec << shamt 
                    << "=0x" << hex << setw(8) << resultado
                    << endl; 
                   
                    cout << dec;
                }

                else if (func3 == 0b010 && func7 == 0b0000000) { //slt

                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = ((int32_t)val1 < (int32_t)val2) ? 1 : 0;

                    if(rd != 0) {
                        x[rd] = resultado; 
                    }
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":slt      " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=(0x" << setw(8) <<  val1 
                    << "<0x" << setw(8) << val2 
                    << ")=" << dec << resultado 
                    << endl; 
                   
                    cout << dec;
                }

                 else if (func3 == 0b011 && func7 == 0b0000000) { //sltu

                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = (val1 < val2) ? 1 : 0;

                    if(rd != 0) {
                        x[rd] = resultado; 
                    }
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":sltu     " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=(0x" << setw(8) <<  val1 
                    << "<0x" << setw(8) << val2 
                    << ")=" << dec << resultado 
                    << endl; 
                   
                    cout << dec;
                }

                else if (func3 == 0b101 && func7 == 0b0100000) { //sra

                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t shamt = val2 & 0x1F;
                    uint32_t resultado = (int32_t)val1 >> shamt;


                    if(rd != 0) {
                        x[rd] = resultado; 
                    }
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":sra      " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << ">>>" << dec << shamt 
                    << "=0x" <<  hex << setw(8) << resultado 
                    << endl; 
                   
                    cout << dec;
                }


                 else if(func3 == 0b000 && func7 == 0b0000001) { // mul
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = val1 * val2;
                    
                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":mul      " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << "*0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                   
                    cout << dec;
                }

                else if(func3 == 0b001 && func7 == 0b0000001) { // mulh
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];

                    int64_t val1grande = (int64_t)(int32_t)val1;
                    int64_t val2grande = (int64_t)(int32_t)val2;
                    uint32_t resultado = (uint32_t) ((val1grande * val2grande) >> 32);
                    
                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":mulh     " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << "*0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                    cout << dec;
                }

                else if(func3 == 0b010 && func7 == 0b0000001) { // mulhsu
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    
                    
                    int64_t val1grande = (int64_t)(int32_t)val1;
                    int64_t val2grande = (int64_t)(uint32_t)val2;
                    uint32_t resultado = (uint32_t) ((val1grande * val2grande) >> 32);
                    
                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) <<  pc << ":mulhsu   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << "*0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                    cout << dec;
                }

                else if(func3 == 0b011 && func7 == 0b0000001) { // mulhu
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];

                    uint64_t val1grande = (int64_t)val1;
                    uint64_t val2grande = (int64_t)val2;
                    uint32_t resultado = (uint32_t) ((val1grande * val2grande) >> 32);
                    
                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":mulhu    " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << "*0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                    cout << dec;
                }

                else if(func3 == 0b100 && func7 == 0b0000001) { //div
                    int32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    int32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado;

                    if(val2 == 0) {
                        resultado = 0xFFFFFFFF; // -1 ;
                    }

                    else if(val1 == (int32_t)0x80000000 && val2 == -1) {
                        resultado = 0x80000000;
                    } else {      
                        resultado = (uint32_t)(val1/val2);  
                    }    

                    if(rd != 0) x[rd] = resultado;

                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":div      " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "/0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                    cout << dec;
                }

                else if(func3 == 0b101 && func7 == 0b0000001) { //divu
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado;

                    if(val2 == 0) {
                        resultado = 0xFFFFFFFF;
                    }
                    else {
                        resultado = val1/val2;
                    }

                    if(rd != 0) x[rd] = resultado;

                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":divu     " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "/0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                    cout << dec;
                }

                else if(func3 == 0b111 && func7 == 0b0000001) { //remuu
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado;

                    if(val2 == 0) {
                        resultado =  val1; 
                    } 
                     else {      
                        resultado = (val1%val2);  
                    }    

                    if(rd != 0) x[rd] = resultado;

                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":remu     " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "%" << "0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                    cout << dec;
                }

                else if(func3 == 0b110 && func7 == 0b0000001) { //rem
                    int32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    int32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado;

                    if(val2 == 0) {
                        resultado =  val1; // -1 ;
                    } else if (val1 == (int32_t)0x80000000 && val2 == -1) {
                        resultado = 0; 
                    }
                     else {      
                        resultado = (uint32_t)(val1%val2);  
                    }    

                    if(rd != 0) x[rd] = resultado;

                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":rem      " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "%" << "0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                    cout << dec;
                }
                
                break;
            }

            case 0b1101111: { // jal

				uint32_t imm = ((instruction >> 31) & 0x1) << 20 |   // imm[20]
                ((instruction >> 21) & 0x3FF) << 1 |  // imm[10:1]
                ((instruction >> 20) & 0x1) << 11 |   // imm[11]
                ((instruction >> 12) & 0xFF) << 12;

                int32_t simm = (imm & 0x100000) ? (imm | 0xFFE00000) : imm;

                uint32_t address = pc + simm;

                cout << hex << setfill('0');
                cout << "0x" << setw(8) << pc << ":jal      "
                << regs[rd] << ",0x" << setw(5) << (imm >> 1)
                << "     pc=0x" << setw(8) << address << "," 
                << regs[rd] << "=0x" << setw(8) << (pc + 4)
                << endl;
              
                if(rd != 0)  {x[rd] = pc + 4;}
                pc = address - 4;
				break;
            }

            case 0b1110011:{ // system

                uint32_t csr_addr = (instruction >> 20) & 0xFFF;

                uint32_t *csr_ptr = nullptr;
                switch(csr_addr) {
                    case CSR_ADDR_MSTATUS: csr_ptr = &mstatus; break;
                    case CSR_ADDR_MIE:     csr_ptr = &mie;     break;
                    case CSR_ADDR_MTVEC:   csr_ptr = &mtvec;   break;
                    case CSR_ADDR_MEPC:    csr_ptr = &mepc;    break;
                    case CSR_ADDR_MCAUSE:  csr_ptr = &mcause;  break;
                    case CSR_ADDR_MTVAL:   csr_ptr = &mtval;   break;
                    case CSR_ADDR_MIP:     csr_ptr = &mip;     break; 
                }

				// // ebreak (funct3 == 000 and imm == 1)
				if(func3 == 0b000) {

                    if(csr_addr == 0x000) { // ecall
                        cout << hex << setfill('0');
                        cout << "0x" << setw(8) << pc << ":ecall" << endl;
                        cout << dec;

                        cout << hex << setfill('0');
                        cout << ">exception:environment_call                cause=0x" << setw(8) << ecall_Mmode
                        << ",epc=0x" << setw(8) << pc 
                        << ",tval=0x00000000" // ECALL não tem valor extra (tval é 0)
                        << endl;
                        cout << dec;

                        trap(ecall_Mmode, 0);
                        continue;
                    }
                    else if(csr_addr == 0x001) { //ebreak
 
					    cout << hex << setfill('0');
                        cout << "0x" << setw(8) << pc << ":ebreak" << endl;
                        cout << dec;
                        run = false;
                    }

                    else if(csr_addr == 0x302) { //mret
                        cout << hex << setfill('0');
                        cout << "0x" << setw(8) << pc << ":mret                 pc=0x" 
                        << setw(8) << mepc << endl;
                        cout << dec;
                        
                        pc = mepc;
                        uint32_t mpie = (mstatus >> 7) & 1;
                        mstatus = (mstatus & ~0x88) | (mpie << 3); // Restaura MIE
                        

                        continue;
                        }
                    } 
                    // bloco de CSRs
                    else if(csr_ptr != nullptr) {
                        uint32_t valor_antigo = *csr_ptr;
                        uint32_t novo = 0;
                        uint32_t valor_rs1 = x[rs1];

                    string csr_name = "seinao";
                    switch (csr_addr) {
                        case CSR_ADDR_MSTATUS: csr_name = "mstatus"; break;
                        case CSR_ADDR_MIE:     csr_name = "mie";     break;
                        case CSR_ADDR_MTVEC:   csr_name = "mtvec";   break;
                        case CSR_ADDR_MEPC:    csr_name = "mepc";    break;
                        case CSR_ADDR_MCAUSE:  csr_name = "mcause";  break;
                        case CSR_ADDR_MTVAL:   csr_name = "mtval";   break;
                        case CSR_ADDR_MIP:     csr_name = "mip";     break;
                        }   

                        cout << hex << setfill('0');

                        if(func3 == 0b001) { //csrrw
                            novo = valor_rs1;
                            *csr_ptr = novo;
                            if (rd != 0) x[rd] = valor_antigo;

                            cout << "0x" << setw(8) << pc << ":csrrw    " 
                            << regs[rd] << "," << csr_name << "," << regs[rs1] 
                            << "     " << regs[rd] << "=" << csr_name << "=0x" << setw(8) << valor_antigo
                            << "," << csr_name << "=" << regs[rs1] << "=0x" << setw(8) << novo 
                            << endl;
                        }

                        else if(func3 == 0b010){ //csrrs    
                            novo = valor_antigo | valor_rs1;
                            *csr_ptr = novo;
                            if (rd != 0) x[rd] = valor_antigo;

                            cout << "0x" << setw(8) << pc << ":csrrs    " 
                            << regs[rd] << "," << csr_name << "," << regs[rs1]
                            << "     " << regs[rd] << "=" << csr_name << "=0x" << setw(8) << valor_antigo
                            << "," << csr_name << "|=" << regs[rs1] 
                            << "=0x" << setw(8) << valor_antigo << "|0x" << setw(8) << valor_rs1 
                            << "=0x" << setw(8) << novo 
                             << endl;
                        }

                        else if (func3 == 0b011) { // csrrc
                        novo = valor_antigo & ~valor_rs1;
                        *csr_ptr = novo;
                        if (rd != 0) x[rd] = valor_antigo;

                        cout << "0x" << setw(8) << pc << ":csrrc    " 
                        << regs[rd] << "," << csr_name << "," << regs[rs1]
                        << "     " << regs[rd] << "=" << csr_name << "=0x" << setw(8) << valor_antigo
                        << "," << csr_name << "&=" << "~" << regs[rs1] 
                        << "=0x" << setw(8) << valor_antigo << "&~0x" << setw(8) << valor_rs1 
                        << "=0x" << setw(8) << novo 
                        << endl;
                        }

                        else if (func3 == 0b101) { //csrrwi

                            novo = uimm;
                            *csr_ptr = novo;
                            if (rd != 0) x[rd] = valor_antigo;

                            // Ex: csrrwi t0, mstatus, 3
                            cout << "0x" << setw(8) << pc << ":csrrwi   " 
                            << regs[rd] << "," << csr_name << ",0x" << hex << uimm
                            << "     " << regs[rd] << "=" << csr_name << "=0x" << setw(8) << valor_antigo
                            << "," << csr_name << "=0x" << uimm << "=0x" << setw(8) << novo 
                            << endl;
                         }

                        else if (func3 == 0b110) {  //csrrsi
                            novo = valor_antigo | uimm;
                            *csr_ptr = novo;
                            if (rd != 0) x[rd] = valor_antigo;

                            cout << "0x" << setw(8) << pc << ":csrrsi   " 
                            << regs[rd] << "," << csr_name << ",0x" << hex << uimm
                            << "     " << regs[rd] << "=" << csr_name << "=0x" << setw(8) << valor_antigo
                            << "," << csr_name << "|=" << "0x" << uimm 
                            << "=0x" << setw(8) << valor_antigo << "|0x" << setw(1) << uimm // setw(1) pra ficar justo
                            << "=0x" << setw(8) << novo 
                            << endl;
                        }

                        else if(func3 == 0b111){
                            novo = valor_antigo & ~uimm;
                            *csr_ptr = novo;
                            if (rd != 0) x[rd] = valor_antigo;

                            // Ex: csrrci t0, mstatus, 2
                            cout << "0x" << setw(8) << pc << ":csrrci   " 
                            << regs[rd] << "," << csr_name << ",0x" << hex << uimm
                            << "     " << regs[rd] << "=" << csr_name << "=0x" << setw(8) << valor_antigo
                            << "," << csr_name << "&=" << "~0x" << uimm 
                            << "=0x" << setw(8) << valor_antigo << "&~0x" << setw(1) << uimm
                            << "=0x" << setw(8) << novo 
                            << endl;
                        }
                    }
					// // Retrieving previous and next instructions
					// uint32_t prev_index = (pc - 4) - offset;

                    // const uint32_t previous = (uint32_t)mem[prev_index + 3] << 24 |(uint32_t)mem[prev_index + 2] << 16 |(uint32_t)mem[prev_index + 1] << 8  | (uint32_t)mem[prev_index + 0];
                    
                    // uint32_t next_index = (pc + 4) - offset;
                                         
					// const uint32_t next = (uint32_t)mem[next_index + 3] << 24 |(uint32_t)mem[next_index + 2] << 16 |(uint32_t)mem[next_index + 1] << 8  | (uint32_t)mem[next_index + 0];
					
            		// if(previous == 0x01f01013 && next == 0x40705013) run = 0;

				 
				break;
            }

            case 0b0010111: {
                //auipc
				uint32_t resultado = pc + imm_20;
                
                if(rd != 0) {
                    x[rd] = resultado;
                }

                cout << hex << setfill('0');
                cout << hex <<  "0x" << setw(8) << pc << ":auipc    " 
                << regs[rd] << ",0x" << setw(5) << (imm_20 >> 12) <<  
                "   " << regs[rd] 
                << "=0x" << setw(8) << pc  
                << "+0x" << setw(8) << imm_20
                << "=0x" << setw(8) << resultado << endl;
                cout << dec;
				
				break;
            }

            case 0b0010011: {
                
                int32_t imms = (imm & 0x800) ? (imm | 0xFFFFF000) : imm; // extensao de sinal 
                uint32_t shamt = imm & 0x1F; //imm[4:0]

                if(func3 == 0b000) { // addi 
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = val1 + imms;

                    if (rd != 0) {
                        x[rd] = resultado;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":addi     " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  setfill('0') << setw(3) 
                    << imm 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "+0x" << setw(8) << imms
                    << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                }

                else if(func3 == 0b111) { // andi 
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = val1 & imms;

                    if (rd != 0) {
                        x[rd] = resultado;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":andi     " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  setfill('0') 
                    << setw(3) << imm 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "&0x" << setw(8) << imms
                    << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                }

                else if(func3 == 0b110) { // ori 
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = val1 | imms;

                    if (rd != 0) {
                        x[rd] = resultado;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":ori      " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  setfill('0') << setw(3) << imm
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "|0x" << setw(8) << imms
                    << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                }

                else if(func3 == 0b100) { // xori 
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = val1 ^ imms;

                    if (rd != 0) {
                        x[rd] = resultado;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":xori     " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  setfill('0') << setw(3) << imm
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "^0x" << setw(8) << imms
                    << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                }

                else if(func3 == 0b010) { // slti 
                    int32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    int32_t resultado = val1 < imms;

                    if (rd != 0) {
                        x[rd] = resultado;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":slti     " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex << setfill('0') << setw(3) <<  imm 
                    << "   " << regs[rd] 
                    << "=(0x" << setw(8) << val1
                    << "<0x" << setw(8) << imms
                    << ")=" << dec << resultado << endl;
                    cout << dec;
                }

                else if(func3 == 0b011) { // sltiu 
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = val1 < imms;

                    if (rd != 0) {
                        x[rd] = resultado;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":sltiu    " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  setfill('0') << setw(3) << imm 
                    << "   " << regs[rd] 
                    << "=(0x" << setw(8) << val1
                    << "<0x" << setw(8) << imms
                    << ")=" << dec << resultado << endl;
                    cout << dec;
                }

                else if(func3 == 0b001) { //slli
                        uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                        uint32_t resultado = val1 << shamt;
                        
                        if (rd != 0) x[rd] = resultado;
                        
                        cout << hex << setfill('0');
                        cout << "0x" << setw(8) << pc << ":slli     " 
                        << regs[rd] << "," << regs[rs1] << "," << dec <<  shamt 
                        << "   " << regs[rd] 
                        << "=0x" << setw(8) << hex << val1
                        << "<<" << dec << shamt
                        << "=0x" << hex << setw(8) << resultado << endl;
                    cout << dec;
                }

                else if(func3 == 0b101 && func7 == 0b0000000) { //srli 
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = val1 >> shamt;

                    if (rd != 0) x[rd] = resultado;

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":srli     " 
                    << regs[rd] << "," << regs[rs1] << "," << dec << shamt 
                    << "   " << regs[rd] 
                    << "=0x" << hex << setw(8) << val1
                    << ">>" << dec << shamt
                    << "=0x" << hex << setw(8) << resultado << endl;
                    cout << dec;
                    
                }

                else if(func3 == 0b101 && func7 == 0b0100000) { //srai
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = (int32_t)val1 >> shamt;

                    if (rd != 0) x[rd] = resultado;

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":srai     " 
                    << regs[rd] << "," << regs[rs1] << "," << dec <<  shamt 
                    << "   " << regs[rd] 
                    << "=0x" << hex << setw(8) << val1
                    << ">>>" << dec << shamt
                    << "=0x" << hex << setw(8) << resultado << endl;
                    cout << dec;
                }
                
                break;
            }

            case 0b0110111 : { // lui - load upper imeddiate
                uint32_t resultado = imm_20;
                
                if(rd != 0) {
                    x[rd] = resultado;
                }

                cout << hex << setfill('0');
                cout << hex <<  "0x" << setw(8) << pc << ":lui      " 
                << regs[rd] << ",0x" << setw(5) << (imm_20 >> 12 ) <<  
                "   " << regs[rd] 
                << "=0x" << setw(8) <<  resultado << endl;
                cout << dec;
				
				break;
            }

            case 0b1100111 : { // jalr
                int32_t imms = (imm & 0x800) ? (imm | 0xFFFFF000) : imm;
                uint32_t valors1 = (rs1 == 0) ? 0 : x[rs1];
                uint32_t jump = valors1 + imms; // pra calcular o endereço do pulo

				cout << hex << setfill('0');
                cout << "0x" << setw(8) << pc << ":jalr     " 
                 << regs[rd] << "," << regs[rs1] << ",0x" << hex << setfill('0') << setw(3) << imm
                 << "     pc=0x" << setw(8) << valors1 
                 << "+0x" << setw(8) << (uint32_t)imms 
                 << "," << regs[rd] << "=0x" << setw(8) << (pc + 4) 
                 << endl;
                cout << dec;
                
                if (rd != 0) {
                    x[rd] = pc + 4;
                 }   
				
                pc = jump - 4;
				
				break;
            }

            case 0b0000011: { // LOAD
                int32_t imms = (imm & 0x800) ? (imm | 0xFFFFF000) : imm;
                uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                uint32_t address = val1 + imms;

                // Variáveis para o print ficar correto (lb, lh, lw...)
                string instr_name = "lb      "; // Padrão
                if (func3 == 0b000) instr_name = "lb      ";
                else if (func3 == 0b001) instr_name = "lh      ";
                else if (func3 == 0b010) instr_name = "lw      ";
                else if (func3 == 0b100) instr_name = "lbu     ";
                else if (func3 == 0b101) instr_name = "lhu     ";

                bool eh_periferico = false;
                
                // --- UART (0x10000000 - 0x100000FF) ---
                if (address >= 0x10000000 && address < 0x10000100) {
                    eh_periferico = true;
                    if (address == 0x10000000) { // DADOS
                        x[rd] = (uint32_t)uart_buffer;
                        uart_pendente = false; 
                    }
                    else if (address == 0x10000005) { // STATUS
                        x[rd] = uart_pendente ? 0x61 : 0x60;
                    } 
                    else { // CONFIG (Ex: 0x10000002) - Retorna 0
                        x[rd] = 0; 
                    }
                }
                // --- PLIC (0x0C000000 - 0x10000000) ---
                else if (address >= 0x0C000000 && address < 0x10000000) {
                    eh_periferico = true;
                    x[rd] = 0; // Leitura de PLIC retorna 0 por padrão
                }
                // --- CLINT (0x02000000 - 0x0200C000) ---
                else if (address >= 0x02000000 && address < 0x0200C000) {
                    eh_periferico = true;
                    if (address == 0x02000000)      x[rd] = (mip >> 3) & 1; 
                    else if (address == 0x02004000) x[rd] = (uint32_t)mtimecmp; 
                    else if (address == 0x02004004) x[rd] = (uint32_t)(mtimecmp >> 32); 
                    else if (address == 0x0200BFF8) x[rd] = (uint32_t)mtime; 
                    else if (address == 0x0200BFFC) x[rd] = (uint32_t)(mtime >> 32); 
                    else x[rd] = 0;
                }

                // Se foi um periférico, imprime e sai AQUI.
                if (eh_periferico) {
                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":" << instr_name 
                         << regs[rd] << ",0x" << setw(3) << imm << "(" << regs[rs1] << ")"
                         << "     " << regs[rd] << "=mem[0x" << setw(8) << address // <--- O FIX DOS 7 DÍGITOS TÁ AQUI
                         << "]=0x" << setw(8) << x[rd] << endl;
                    cout << dec;
                    break; // Sai do switch
                }

                // ==========================================================
                // 2. MEMÓRIA RAM (Só chega aqui se não for periférico)
                // ==========================================================
                if(address < offset || address >= offset + MEM_SIZE) {
                    cout << hex << setfill('0');
                    cout << ">exception:load_fault          cause=0x" << setw(8) << load_access_fault
                    << ",epc=0x" << setw(8) << pc << ",tval=0x" << setw(8) << address << endl;
                    cout << dec;
                    trap(load_access_fault, address);
                    continue;
                }

                // Leitura Real da RAM
                uint32_t final_val = 0;
                if(func3 == 0b000) { // lb
                    int8_t byte = mem[address - offset];
                    final_val = (byte & 0x80) ? (byte | 0xFFFFFF00) : byte;
                } 
                else if(func3 == 0b100) { // lbu
                    final_val = (uint8_t)mem[address - offset];
                } 
                else if(func3 == 0b001) { // lh
                    uint16_t half = (uint16_t)mem[address - offset + 1] << 8 | (uint16_t)mem[address - offset];
                    final_val = (half & 0x8000) ? (half | 0xFFFF0000) : half;
                }
                else if(func3 == 0b101) { // lhu
                     final_val = (uint16_t)mem[address - offset + 1] << 8 | (uint16_t)mem[address - offset];
                }
                else if(func3 == 0b010) { // lw
                    final_val = (uint32_t)mem[address - offset + 3] << 24 | (uint32_t)mem[address - offset + 2] << 16 | (uint32_t)mem[address - offset + 1] << 8  | (uint32_t)mem[address - offset];
                }

                if (rd != 0) x[rd] = final_val;

                // Print Padrão RAM
                cout << hex << setfill('0');
                cout << "0x" << setw(8) << pc << ":" << instr_name 
                     << regs[rd] << ",0x" << setw(3) << imm << "(" << regs[rs1] << ")"
                     << "     " << regs[rd] << "=mem[0x" << setw(8) << address // <--- O FIX DOS 7 DÍGITOS
                     << "]=0x" << setw(8) << final_val << endl;
                cout << dec;

                break;
            }

            case 0b0100011 : { // STORE
                int32_t imm_40 = rd;
                int32_t imm_115 = func7;
                int32_t immu = imm_115 << 5 | imm_40; 
                int32_t imms = (immu & 0x800) ? (immu | 0xFFFFF000) : immu;
                uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                int32_t address = val1 + imms;
                uint32_t val_to_write = (rs2 == 0) ? 0 : x[rs2];

                string instr_name = "sw      ";
                int width = 8; // Padrão SW
                uint32_t print_val = val_to_write;

                if (func3 == 0b000) {      // sb
                    instr_name = "sb      ";
                    width = 2;             // Usa 2 dígitos!
                    print_val &= 0xFF;     
                } 
                else if (func3 == 0b001) { // sh
                    instr_name = "sh      ";
                    width = 4;             // Usa 4 dígitos!
                    print_val &= 0xFFFF;
                }

                // ==========================================================
                // 1. ZONA DE PERIFÉRICOS
                // ==========================================================
                bool eh_periferico = false;

                // --- UART ---
                if (address >= 0x10000000 && address < 0x10000100) {
                    eh_periferico = true;
                    if (address == 0x10000000) { // DADOS
                         if (outfile) { fputc((uint8_t)val_to_write, outfile); fflush(outfile); }
                         uart_pendente = true; // Dispara interrupção
                    }
                    // Outros endereços (0x10000001) são ignorados mas printados abaixo
                }
                // --- PLIC ---
                else if (address >= 0x0C000000 && address < 0x10000000) {
                    eh_periferico = true;
                    if (address == 0x0C200004 && val_to_write == 10) {
                        uart_pendente = false;
                        mip &= ~0x800;
                    }
                }
                // --- CLINT ---
                else if (address >= 0x02000000 && address < 0x0200C000) {
                     eh_periferico = true;
                     if (address == 0x02000000) { if(val_to_write > 0) mip|=8; else mip&=~8; }
                     else if (address == 0x02004000) mtimecmp = (mtimecmp & 0xFFFFFFFF00000000) | (uint64_t)val_to_write;
                     else if (address == 0x02004004) mtimecmp = (mtimecmp & 0x00000000FFFFFFFF) | ((uint64_t)val_to_write << 32);
                     else if (address == 0x0200BFF8) mtime = (mtime & 0xFFFFFFFF00000000) | (uint64_t)val_to_write;
                     else if (address == 0x0200BFFC) mtime = (mtime & 0x00000000FFFFFFFF) | ((uint64_t)val_to_write << 32);
                }

                if (eh_periferico) {
                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":" << instr_name 
                         << regs[rs2] << ",0x" << setw(3) << immu << "(" << regs[rs1] << ")"
                         << "     " << "mem[0x" << setw(8) << address // <--- O FIX DOS 7 DÍGITOS
                         << "]=0x" << setw(width) << print_val << endl; // Valor ajustado
                    cout << dec;
                    break;
                }

                // ==========================================================
                // 2. MEMÓRIA RAM
                // ==========================================================
                if(address < offset || address >= offset + MEM_SIZE) {
                     // ... trap store_fault ...
                     cout << ">exception:store_fault          cause=0x" << setw(8) << store_access_fault 
                          << ",epc=0x" << setw(8) << pc 
                          << ",tval=0x" << setw(8) << address 
                          << endl;
                     
                     cout << dec;

                     trap(store_access_fault, address);
                     continue;
                }

                // Escrita Real na RAM
                if(func3 == 0b000) { // sb
                    mem[address - offset] = (uint8_t)val_to_write;
                }
                else if(func3 == 0b001) { // sh
                    mem[address - offset] = (uint8_t)val_to_write;
                    mem[address - offset+1] = (uint8_t)(val_to_write >> 8);
                }
                else if(func3 == 0b010) { // sw
                    mem[address - offset] = (uint8_t)val_to_write;
                    mem[address - offset+1] = (uint8_t)(val_to_write >> 8);
                    mem[address - offset+2] = (uint8_t)(val_to_write >> 16);
                    mem[address - offset+3] = (uint8_t)(val_to_write >> 24);
                }

                cout << hex << setfill('0');
                cout << "0x" << setw(8) << pc << ":" << instr_name 
                     << regs[rs2] << ",0x" << setw(3) << immu << "(" << regs[rs1] << ")"
                     << "     " << "mem[0x" << setw(8) << address
                     << "]=0x" << setw(width) << print_val << endl;
                cout << dec;

                break;
            }

            case 0b1100011: { // tipo B
                uint32_t imm1a4  = (instruction >> 8)  & 0xF; 
                uint32_t imm5a10 = (instruction >> 25) & 0x3F; 
                uint32_t imm11   = (instruction >> 7)  & 0x1;      
                uint32_t imm12   = (instruction >> 31) & 0x1;

                uint32_t immu = (imm12 << 12) | (imm11 << 11) | (imm5a10 << 5) | (imm1a4 << 1);
                int32_t imms = ((int32_t)(immu << 19)) >> 19;
                

                if(func3 == 0b000) { // beq 
                    int32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    int32_t val2 = (rs2 == 0) ? 0 : x[rs2];

                    bool condicao = (val1 == val2);
                    uint32_t enderecoPulo = (condicao) ? (pc + imms) : (pc + 4);

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":beq      " 
                    << regs[rs1] << "," << regs[rs2] << ",0x"  << hex << setfill('0') << setw(3) << ((imms>> 1) & 0xFFF)
                    << "     (0x" << setw(8) << val1 << "==0x" 
                    << setw(8) << val2 << ")=" << dec << condicao
                    << "->pc=0x" << hex << setw(8) << enderecoPulo << endl;
                    cout << dec;
                    
                    if(condicao) pc = enderecoPulo - 4;

                }
                

                else if(func3 == 0b001){ //bne
                    int32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    int32_t val2 = (rs2 == 0) ? 0 : x[rs2];

                    bool condicao = (val1 != val2);
                    uint32_t enderecoPulo = (condicao) ? (pc + imms) : (pc + 4);

                    
                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":bne      " 
                    << regs[rs1] << "," << regs[rs2] << ",0x" << hex << setfill('0')<< setw(3) 
                    << ((imms>> 1) & 0xFFF)
                    << "     (0x" << setw(8) << val1 << "!=0x" 
                    << setw(8) << val2 << ")=" << dec << condicao
                    << "->pc=0x" << hex << setw(8) << enderecoPulo << endl;
                    cout << dec;
                    
                    if(condicao) pc = enderecoPulo - 4;
                }

                else if(func3 == 0b100) { //blt

                    int32_t val1 = (rs1 == 0) ? 0 : (int32_t)x[rs1];
                    int32_t val2 = (rs2 == 0) ? 0 : (int32_t)x[rs2];

                    bool condicao = (val1 < val2);
                    uint32_t enderecoPulo = (condicao) ? (pc + imms) : (pc + 4);
                   
                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":blt      " 
                    << regs[rs1] << "," << regs[rs2] << ",0x" << hex << setfill('0') << setw(3) << ((imms>> 1) & 0xFFF)
                    << "     (0x" << setw(8) << val1 << "<0x" 
                    << setw(8) << val2 << ")=" << dec << condicao
                    << "->pc=0x" << hex << setw(8) << enderecoPulo << endl;
                    cout << dec;
                    
                    if(condicao) pc = enderecoPulo - 4;
                }

                else if(func3 == 0b101){ //bge
                    int32_t val1 = (rs1 == 0) ? 0 : (int32_t)x[rs1];
                    int32_t val2 = (rs2 == 0) ? 0 : (int32_t)x[rs2];

                    bool condicao = (val1 >= val2);
                    uint32_t enderecoPulo = (condicao) ? (pc + imms) : (pc + 4);
                    
                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":bge      " 
                    << regs[rs1] << "," << regs[rs2] << ",0x" << hex << setfill('0') << setw(3) << ((imms>> 1) & 0xFFF) 
                    << "     (0x" << setw(8) << val1 << ">=0x" 
                    << setw(8) << val2 << ")=" << dec << condicao
                    << "->pc=0x" << hex << setw(8) << enderecoPulo << endl ;
                    cout << dec;
                    
                    if(condicao) pc = enderecoPulo - 4;
                }

                else if(func3 == 0b111){ //bgeu
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];

                    bool condicao = (val1 >= val2);
                    uint32_t enderecoPulo = (condicao) ? (pc + imms) : (pc + 4);
                    
                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":bgeu     " 
                    << regs[rs1] << "," << regs[rs2] << ",0x" << hex << setfill('0') << setw(3) << ((imms>> 1) & 0xFFF) 
                    << "     (0x" << setw(8) << val1 << ">=0x" 
                    << setw(8) << val2 << ")=" << dec << condicao
                    << "->pc=0x" << hex << setw(8) << enderecoPulo << endl ;
                    cout << dec;
                    
                    if(condicao) pc = enderecoPulo - 4;
                    
                }

                else if(func3 == 0b110) { // bltu
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];

                    bool condicao = (val1 < val2);
                    uint32_t enderecoPulo = (condicao) ? (pc + imms) : (pc + 4);

                    
                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":bltu     " 
                    << regs[rs1] << "," << regs[rs2] << ",0x" << hex << setfill('0') << setw(3) << ((imms>> 1) & 0xFFF) 
                    << "     (0x" << setw(8) << val1 << "<0x" 
                    << setw(8) << val2 << ")=" << dec << condicao
                    << "->pc=0x" << hex << setw(8) << enderecoPulo << endl;
                    cout << dec;
                    
                    if(condicao) pc = enderecoPulo - 4;
                }
                break;
            }

            default:
                cout << hex << setfill('0');
                cout << ">exception:illegal_instruction        cause=0x" << setw(8) << illegal_instruction
                << ",epc=0x" << setw(8) << pc 
                << ",tval=0x" << setw(8) << instruction // O valor ruim é a instrução ilegível
                << endl;
                trap(2, instruction);
                cout << dec; 
                continue;
        }
        pc += 4;

        
    }
    cout.rdbuf(coutbuffer);
    input.close();
    output.close();

    return 0;
    }