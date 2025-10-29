#include <bits/stdc++.h>
#include <iomanip>

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

    ifstream input("../1_emptydois.hex");
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
        
        if (linha.empty() || linha[0] == '#' || linha[0] == '@') continue;

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

    x[6] = 20; // x6 (t1) = 20
    x[7] = 10; // x7 (t2) = 10

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
                    cout << hex << setw(8) <<  pc << ":add   " << regs[rd] << "," << regs[rs1] << "," << regs[rs2] << "   " << regs[rd] << "=0x" << setw(8) <<  x[rs1] << "+0x" << setw(8) << x[rs2] << "=0x" << setw(8) << resultado << endl;
                    cout << dec;
                }
                else if(func3 == 0b000 && func7 == 0b0100000) { // sub
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = val1 - val2; 
                    
                    if(rd != 0) x[rd] = resultado;
                
                    cout << hex << setfill('0');
                    cout << hex << setw(8) <<  pc << ":sub   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << "-0x" << setw(8) << x[rs2] 
                    << "=0x" << setw(8) << x[rd] 
                    << endl; 
                    
                    cout << dec;
                }

                else if(func3 == 0b100 && func7 == 0b0000000) { // xor
                    if(rd != 0) x[rd] = x[rs1] ^ x[rs2]; 
                    
                    cout << hex << setfill('0');
                    cout << hex << setw(8) <<  pc << ":xor   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << "^0x" << setw(8) << x[rs2] 
                    << "=0x" << setw(8) << x[rd] 
                    << endl; 
                   
                    cout << dec;
                }

                else if(func3 == 0b110 && func7 == 0b0000000) { // or
                    if(rd != 0) x[rd] = x[rs1] | x[rs2]; 
                    
                    cout << hex << setfill('0');
                    cout << hex << setw(8) <<  pc << ":or   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << "^0x" << setw(8) << x[rs2] 
                    << "=0x" << setw(8) << x[rd] 
                    << endl; 
                   
                    cout << dec;
                }

                else if (func3 == 0b111 && func7 == 0b0000000) { // and
                    if(rd != 0) x[rd] = x[rs1] & x[rs2]; 
                    
                    cout << hex << setfill('0');
                    cout << hex << setw(8) <<  pc << ":and   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << "&0x" << setw(8) << x[rs2] 
                    << "=0x" << setw(8) << x[rd] 
                    << endl; 
                   
                    cout << dec;
                }

                else if (func3 == 0b001 && func7 == 0b0000000) { //sll
                    if(rd != 0) {
                        x[rd] = x[rs1] << x[rs2]; 
                    }
                    
                    cout << hex << setfill('0');
                    cout << hex << setw(8) <<  pc << ":sll   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << "<<0x" << setw(8) << x[rs2] 
                    << "=0x" << setw(8) << x[rd] 
                    << endl; 
                   
                    cout << dec;
                }

                else if (func3 == 0b101 && func7 == 0b0000000) { //srl
                    if(rd != 0) {
                        x[rd] = x[rs1] >> x[rs2]; 
                    }
                    
                    cout << hex << setfill('0');
                    cout << hex << setw(8) <<  pc << ":srl    " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << ">>0x" << setw(8) << x[rs2] 
                    << "=0x" << setw(8) << x[rd] 
                    << endl; 
                   
                    cout << dec;
                }

                else if (func3 == 0b010 && func7 == 0b0000000) { //slt
                    if(rd != 0) {
                        x[rd] = ((int32_t)x[rs1] < (int32_t)x[rs2] ? 1 :0); 
                    }
                    
                    cout << hex << setfill('0');
                    cout << hex << setw(8) <<  pc << ":slt   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=(0x" << setw(8) <<  x[rs1] 
                    << "<0x" << setw(8) << x[rs2] 
                    << ")=u" << dec << x[rd] 
                    << endl; 
                   
                    cout << dec;
                }

                 else if (func3 == 0b111 && func7 == 0b0000000) { //sltu
                    if(rd != 0) {
                        x[rd] = (x[rs1] < x[rs2] ? 1 :0); 
                    }
                    
                    cout << hex << setfill('0');
                    cout << hex << setw(8) <<  pc << ":sltu   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=(0x" << setw(8) <<  x[rs1] 
                    << "<0x" << setw(8) << x[rs2] 
                    << ")=u" << dec << x[rd] 
                    << endl; 
                   
                    cout << dec;
                }

                else if (func3 == 0b010 && func7 == 0b0100000) { //sra
                    if(rd != 0) {
                        x[rd] = ((int32_t)x[rs1]) >> x[rs2]; 
                    }
                    
                    cout << hex << setfill('0');
                    cout << hex << setw(8) <<  pc << ":srl   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << ">>0x" << setw(8) << x[rs2] 
                    << "=0x" << setw(8) << x[rd] 
                    << endl; 
                   
                    cout << dec;
                }

                else if(func3 == 0b000 && func7 == 0b111111) { // mult
                    uint32_t val1 = (rs1 == 0) ? 0 : x[rs1];
                    uint32_t val2 = (rs2 == 0) ? 0 : x[rs2];
                    uint32_t resultado = val1 * val2;
                    
                    if(rd != 0) x[rd] = resultado; 
                    
                    cout << hex << setfill('0');
                    cout << hex << setw(8) <<  pc << ":mul   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) <<  x[rs1] 
                    << "*0x" << setw(8) << x[rs2] 
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
                    cout << hex << setw(8) <<  pc << ":mul   " 
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
                    cout << hex << setw(8) <<  pc << ":mulh   " 
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
                    cout << hex << setw(8) <<  pc << ":mulhsu   " 
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
                    cout << hex << setw(8) <<  pc << ":mulhu   " 
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
                    cout << hex << setw(8) <<  pc << ":div   " 
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
                        resultado = 0xFFFFFFFF; // -1 ;
                    }
                    else {
                        resultado = val1/val2;
                    }

                    if(rd != 0) x[rd] = resultado;

                    cout << hex << setfill('0');
                    cout << hex << setw(8) <<  pc << ":divu   " 
                    << regs[rd] << "," << regs[rs1] << "," << regs[rs2] 
                    << "   " << regs[rd] 
                    << "=0x" << setw(8) << val1
                    << "/0x" << setw(8) << val2 
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
				// if(funct3 == 0b000 && imm == 1) {
				// 	// Outputting instruction to console
				// 	printf("0x%08x:ebreak\n", pc);
				// 	// Retrieving previous and next instructions
				// 	const uint32_t previous = ((uint32_t*)(mem))[(pc - 4 - offset) >> 2];
				// 	const uint32_t next = ((uint32_t*)(mem))[(pc + 4 - offset) >> 2];
				// 	// Halting condition
            	// 	if(previous == 0x01f01013 && next == 0x40705013) run = 0;
				// }
				// // Breaking case
				break;
            }
            default:
                cout << "0x" << hex << pc << ": codigo desconhecido\n";
                run = 0; 
        }
        pc += 4;
    }

    return 0;
}