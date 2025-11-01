#include <bits/stdc++.h>
#include <iomanip>
#include <chrono>

using namespace std;

int main (int argc, char* argv[]) {
    
    cout << "-------------------------------------------------------\n";

    // if(argc < 3) { // pra ver se a quantidade de parametro ta certinha 

    //     cerr << "Parametros Incorretos" << endl;
    //     return 1;
    // }

    for(int i =0; i < argc; i++) { // ta certinha
        cout << "argv[" << i << "] = " << argv[i] << endl;
    }

    ifstream input("../1_empty.hex");
   // ofstream output(argv[2]);

    if(!input.is_open()){
        cerr << "Rapaz os arquivo nao abriu nao" << endl;
        return 1;
    }

    const uint32_t offset = 0x80000000;
    vector<uint8_t> mem(32 * 1024, 0); // olha a  memoria ai pessoal

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
    uint32_t pc = offset;

    while(run) {

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
                    cout << hex << setw(8) <<  "0x" << pc << ":add   " << regs[rd] << "," << regs[rs1] << "," << regs[rs2] << "   " << regs[rd] << "=0x" << setw(8) <<  x[rs1] << "+0x" << setw(8) << x[rs2] << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                }
                else if(func3 == 0b000 && func7 == 0b0100000) { // sub
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = val1 - val2; 
                    
                    if(rd != 0) x[rd] = resultado;
                
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":sub   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << "-0x" << setw(8) << x[rs2] 
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
                    cout << hex << "0x" << setw(8) << pc << ":xor   " 
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
                    uint32_t resultado = val1 & val2;

                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) <<  pc << ":or   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << "^0x" << setw(8) << val2
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

                    if(rd != 0) x[rd] = x[rs1] & x[rs2]; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":and   " 
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
                    cout << hex << "0x" << setw(8) << pc << ":sll   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << "<<0x" << setw(8) << shamt 
                    << "=0x" << setw(8) << resultado 
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
                    cout << hex << "0x" << setw(8) << pc << ":srl    " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << ">>0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado
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
                    cout << hex << "0x" << setw(8) << pc << ":slt   " 
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
                    cout << hex << "0x" << setw(8) << pc << ":sltu   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=(0x" << setw(8) <<  val1 
                    << "<0x" << setw(8) << val2 
                    << ")=" << dec << resultado 
                    << endl; 
                   
                    cout << dec;
                }

                else if (func3 == 0b010 && func7 == 0b0100000) { //sra

                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t shamt = val2 & 0x1F;
                    uint32_t resultado = (int32_t)val1 >> shamt;


                    if(rd != 0) {
                        x[rd] = resultado; 
                    }
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":srl   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << ">>0x" << setw(8) << shamt 
                    << "=0x" << setw(8) << resultado 
                    << endl; 
                   
                    cout << dec;
                }

                else if(func3 == 0b000 && func7 == 0b111111) { // mult
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = val1 * val2;
                    
                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":mul   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  val1 
                    << "*0x" << setw(8) << val2 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                   
                    cout << dec;
                }

                 else if(func3 == 0b000 && func7 == 0b0000001) { // mul
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = val1 * val2;
                    
                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":mul   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << "*0x" << setw(8) << x[rs2] 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                   
                    cout << dec;
                }

                else if(func3 == 0b001 && func7 == 0b0000001) { // mulh
                    int64_t val1 = (int64_t)(int32_t)((rs1 == 0) ? 0 : x[rs1]);
                    int64_t val2 = (int64_t)(int32_t)((rs2 == 0) ? 0 : x[rs2]);

                    uint32_t resultado = (uint32_t) ((val1 * val2) >> 32);
                    
                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":mulh   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << "*0x" << setw(8) << x[rs2] 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                    cout << dec;
                }

                else if(func3 == 0b010 && func7 == 0b0000001) { // mulhsu
                    int64_t val1 = (int64_t)(int32_t)((rs1 == 0) ? 0 : x[rs1]);
                    int64_t val2 = (int64_t)(uint32_t)((rs2 == 0) ? 0 : x[rs2]);

                    uint32_t resultado = (uint32_t) ((val1 * val2) >> 32);
                    
                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) <<  pc << ":mulhsu   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << "*0x" << setw(8) << x[rs2] 
                    << "=0x" << setw(8) << resultado
                    << endl; 
                    cout << dec;
                }

                else if(func3 == 0b011 && func7 == 0b0000001) { // mulhu
                    int64_t val1 = (int64_t)(uint32_t)((rs1 == 0) ? 0 : x[rs1]);
                    int64_t val2 = (int64_t)(uint32_t)((rs2 == 0) ? 0 : x[rs2]);

                    uint32_t resultado = (uint32_t) ((val1 * val2) >> 32);
                    
                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << "0x" << setw(8) << pc << ":mulhu   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << "*0x" << setw(8) << x[rs2] 
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
                    cout << hex << "0x" << setw(8) << pc << ":div   " 
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
                    cout << hex << "0x" << setw(8) << pc << ":divu   " 
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
                    cout << hex << "0x" << setw(8) << pc << ":remu  " 
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
                    cout << hex << "0x" << setw(8) << pc << ":rem  " 
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
				const uint32_t simm = (imm20 >> 19) ? (0xFFF00000 | imm20) : (imm20);
				const uint32_t address = pc + (simm << 1);
				cout << hex << setfill('0');

                cout << "0x" << setw(8) << pc << ":jal    " << regs[rd] << ",0x" << setw(5) << imm20
                << "    pc=0x" << setw(8) << address << "," << regs[rd] << "=0x" << setw(8) << (pc + 4) 
                << endl;

                cout << dec;
			
				if(rd != 0) x[rd] = pc + 4;
				pc = address - 4;
				
				break;
            }

            case 0b1110011:{
				// // ebreak (funct3 == 000 and imm == 1)
				if(func3 == 0b000 && imm == 1) {
                    // Outputting instruction to console
					printf("0x%08x:ebreak\n", pc);
					// Retrieving previous and next instructions
					uint32_t prev_index = (pc - 4) - offset;

                    const uint32_t previous = (uint32_t)mem[prev_index + 3] << 24 |(uint32_t)mem[prev_index + 2] << 16 |(uint32_t)mem[prev_index + 1] << 8  | (uint32_t)mem[prev_index + 0];
                    
                    uint32_t next_index = (pc + 4) - offset;
                                         
					const uint32_t next = (uint32_t)mem[next_index + 3] << 24 |(uint32_t)mem[next_index + 2] << 16 |(uint32_t)mem[next_index + 1] << 8  | (uint32_t)mem[next_index + 0];
					
            		if(previous == 0x01f01013 && next == 0x40705013) run = 0;

				 }
				break;
            }

            case 0b0010111: {
                //auipc

				uint32_t resultado = pc + imm_20;
                
                if(rd != 0) {
                    x[rd] = resultado;
                }

                cout << hex << setfill('0');
                cout << hex <<  "0x" << setw(8) << pc << ":auipc   " 
                << regs[rd] << ",0x" << setw(5) << (imm_20 >> 12) <<  
                "   " << regs[rd] 
                << "=0x" << setw(8) <<  imm_20  
                << "+0x" << setw(8) << pc 
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
                    cout << "0x" << setw(8) << pc << ":addi   " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  imms 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "+0x" << setw(8) << imms
                    << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                }

                if(func3 == 0b111) { // andi 
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = val1 & imms;

                    if (rd != 0) {
                        x[rd] = resultado;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":andi   " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  imms 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "&0x" << setw(8) << imms
                    << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                }

                if(func3 == 0b110) { // ori 
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = val1 | imms;

                    if (rd != 0) {
                        x[rd] = resultado;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":ori   " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  imms 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "|0x" << setw(8) << imms
                    << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                }

                if(func3 == 0b100) { // xori 
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = val1 | imms;

                    if (rd != 0) {
                        x[rd] = resultado;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":xori   " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  imms 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "^0x" << setw(8) << imms
                    << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                }

                if(func3 == 0b010) { // slti 
                    int32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    int32_t resultado = val1 < imms;

                    if (rd != 0) {
                        x[rd] = resultado;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":slti   " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  imms 
                    << "   " << regs[rd] 
                    << "=(0x" << setw(8) << val1
                    << "<0x" << setw(8) << imms
                    << ")=" << dec << resultado << endl;
                    cout << dec;
                }

                if(func3 == 0b011) { // sltiu 
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = val1 < imms;

                    if (rd != 0) {
                        x[rd] = resultado;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":sltiu   " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  imms 
                    << "   " << regs[rd] 
                    << "=(0x" << setw(8) << val1
                    << "<0x" << setw(8) << imms
                    << ")=" << dec << resultado << endl;
                    cout << dec;
                }

                else if(func3 == 0b001 && func7 == 0b0000000) { //slli
                        uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                        uint32_t resultado = val1 << shamt;
                        
                        if (rd != 0) x[rd] = resultado;
                        
                        cout << hex << setfill('0');
                        cout << "0x" << setw(8) << pc << ":slli   " 
                        << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  imms 
                        << "   " << regs[rd] 
                        << "=0x" << setw(8) << val1
                        << "<<0x" << setw(8) << shamt
                        << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                }

                else if(func3 == 0b101 && func7 == 0b0000000) { //srli 
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = val1 >> shamt;

                    if (rd != 0) x[rd] = resultado;

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":srli   " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  shamt 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << ">>0x" << setw(8) << shamt
                    << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                    
                }

                else if(func3 == 0b101 && func7 == 0b0000000) { //srai
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t resultado = (int32_t)val1 >> shamt;

                    if (rd != 0) x[rd] = resultado;

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":srai   " 
                    << regs[rd] << "," << regs[rs1] << ",0x" << hex <<  shamt 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << ">>0x" << setw(8) << shamt
                    << "=0x" << setw(8) << resultado << endl;
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
                cout << hex <<  "0x" << setw(8) << pc << ":lui   " 
                << regs[rd] << ",0x" << setw(5) << (imm_20 >> 12 ) <<  
                "   " << regs[rd] 
                << "=0x" << setw(8) <<  resultado;
                cout << dec;
				
				break;
            }

            case 0b1100111 : { // jalr
                int32_t imms = (imm & 0x800) ? (imm | 0xFFFFF000) : imm;
                uint32_t valors1 = (rs1 == 0) ? 0 : x[rs1];
                uint32_t jump = valors1 + imms; // pra calcular o endereço do pulo

				cout << hex << setfill('0');
                cout << "0x" << setw(8) << pc << ":jalr    " << regs[rd] << ",0x" << regs[rs1] << ",0x" << hex << imm
                << "    pc=0x" << setw(8) << jump << "," << regs[rd] << "=0x" << setw(8) << (pc + 4) 
                << endl;
                cout << dec;
                
                if (rd != 0) {
                    x[rd] = pc + 4;
                 }   
				
                pc = jump - 4;
				
				break;
            }

            case 0b0000011: { // LOAD - STORE

                int32_t imms = (imm & 0x800) ? (imm | 0xFFFFF000) : imm; // extensao de sinal 
                
                if(func3 == 0b000) { // lb - load byte
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    int32_t address = val1 + imms;
                    int8_t byte = mem[address - offset];
                    int32_t bytes = (byte & 0x800) ? (byte | 0xFFFFFF00) : byte;
                    
                    if(rd != 0) {
                        x[rd] = bytes;
                    }

                cout << hex << setfill('0');
                cout << "0x" << setw(8) << pc << ":lb    " << regs[rd] << "," << imms 
                << "(" << regs[rs1] << ")"
                << "     " << regs[rd] << "=mem[0x" <<  address
                << "=0x" << setw(8) << bytes << endl;
                cout << dec;

                } 

                else if(func3 == 0b100) { // lbu - load byte unsigned
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t address = val1 + imms;
                    uint8_t byte = mem[address - offset];
                    
                    if(rd != 0) {
                        x[rd] = byte;
                    }

                cout << hex << setfill('0');
                cout << "0x" << setw(8) << pc << ":lbu    " << regs[rd] << "," << imms 
                << "(" << regs[rs1] << ")"
                << "     " << regs[rd] << "=mem[0x" <<  address
                << "=0x" << setw(8) << static_cast<int>(byte) << endl;
                cout << dec;

                } 

                else if(func3 == 0b001) { // lh - load half
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t address = val1 + imms;
    
                    uint16_t metadinha = (uint16_t)mem[address - offset + 1] << 8 | 
                    (uint16_t)mem[address - offset];
                    
                    int32_t bytes = (metadinha & 0x8000) ? (metadinha | 0xFFFF0000) : metadinha;

                    if(rd != 0) {
                        x[rd] = bytes;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":lh    " << regs[rd] << "," << imms 
                    << "(" << regs[rs1] << ")"
                    << "     " << regs[rd] << "=mem[0x" <<  address
                    << "=0x" << setw(8) << bytes << endl;
                    cout << dec;

                }

                else if(func3 == 0b101) { // lhu - load half unsigned
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t address = val1 + imms;
    
                    uint16_t metadinha = (uint16_t)mem[address - offset + 1] << 8 | 
                    (uint16_t)mem[address - offset];
                    
                    if(rd != 0) {
                        x[rd] = metadinha;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":lhu    " << regs[rd] << "," << imms 
                    << "(" << regs[rs1] << ")"
                    << "     " << regs[rd] << "=mem[0x" <<  address
                    << "=0x" << setw(8) << static_cast<int>(metadinha) << endl;
                    cout << dec;

                }

                else if(func3 == 0b010) { // lw - load word
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t address = val1 + imms;
    
                    uint32_t todinha = (uint32_t)mem[address - offset + 1] << 8 | 
                    (uint32_t)mem[address - offset] | (uint32_t)mem[address - offset + 2] << 16 |(uint32_t)mem[address - offset + 3] << 24;
                    
                    if(rd != 0) {
                        x[rd] = todinha;
                    }

                    cout << hex << setfill('0');
                    cout << "0x" << setw(8) << pc << ":lw    " << regs[rd] << "," << imms 
                    << "(" << regs[rs1] << ")"
                    << "     " << regs[rd] << "=mem[0x" <<  address
                    << "=0x" << setw(8) << todinha << endl;
                    cout << dec;

                }


            }

            case 0b1100011 : { // TIPO B

            }
            default:
                cout << "0x" << hex << pc << ": codigo desconhecido\n";
                run = 0; 
        }
        pc += 4;
    }

    return 0;
}