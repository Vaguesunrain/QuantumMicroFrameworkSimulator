#include "SimDriver.hpp"
#include "Vmodule_top.h" 
#include "GateLibrary.hpp"
GateLibrary gate_lib;

SimDriver::SimDriver(Vmodule_top* top_ptr, int num_qubits, short select_module) : dut(top_ptr) {
    
    init_qubits(num_qubits);
    qubits->bind_sim_time(&m_sim_clock);
    if(select_module == 1) {
        auto density_module = std::make_shared<DensityMatrixModule>(gate_lib);
        qubits->install_module(density_module);
    }
    else if(select_module == 2) {
        auto bloch_module = std::make_shared<BlochSphereModule>(gate_lib);
        qubits->install_module(bloch_module);
    }
    else if(select_module == 3) {
        auto density_module = std::make_shared<DensityMatrixModule>(gate_lib);
        qubits->install_module(density_module);
        auto bloch_module = std::make_shared<BlochSphereModule>(gate_lib);
        qubits->install_module(bloch_module);
    }
    else{
        auto density_module = std::make_shared<DensityMatrixModule>(gate_lib);
        qubits->install_module(density_module);
        std::cout << "[SimDriver] Default: DensityMatrixModule installed." << std::endl;
    }
}

SimDriver::~SimDriver() {
    delete qubits;
}

void SimDriver::step(uint64_t time) {

    if (dut->rst_n == 0) {
        if (m_last_rst_n == 1) {
            std::cout << "[SimDriver] Time " << time << ": SV Hardware Reset Detected. Resetting Qubit States..." << std::endl;
            qubits->reset();
        }
        m_last_rst_n = dut->rst_n; 
        return;                    
    }
    if (dut->rst_n == 1 && m_last_rst_n == 0) {
        std::cout << "[SimDriver] Time " << time << ": SV Hardware Reset Released. Recovery cycle..." << std::endl;
        m_last_rst_n = dut->rst_n; 
        return;                    
    }
    m_last_rst_n = dut->rst_n; 

    if (dut->trigger) {
        m_sim_clock = time;
        
        uint32_t raw_cmd = dut->control_bus;
        
        uint8_t  opcode  = (raw_cmd >> 24) & 0xFF;
        uint16_t src_qid = (raw_cmd >> 12) & 0xFFF;
        uint16_t dst_qid = raw_cmd & 0xFFF;
        switch(opcode){
            // ==================== 1-Qubit  ====================
            case 0x01: // H 
                qubits->apply_gate("H", src_qid);

                break;
            case 0x02: // X 
                qubits->apply_gate("X", src_qid);
      
                break;
            case 0x03: // Y 
                qubits->apply_gate("Y", src_qid);
         
                break;
            case 0x04: // Z 
                qubits->apply_gate("Z", src_qid);

                break;
            case 0x05: // SX (Square Root X) 
                qubits->apply_gate("SX", src_qid);

                break;
            case 0x06: // S 
                qubits->apply_gate("S", src_qid);
     
                break;
            case 0x07: // S_dag (S^\dagger) 
                qubits->apply_gate("S_dag", src_qid);
 
                break;
            case 0x08: // T 
                qubits->apply_gate("T", src_qid);
                break;
            case 0x09: // T_dag (T^\dagger) 
                qubits->apply_gate("T_dag", src_qid);

                break;
            case 0x0A: // Measurement 
                qubits->apply_gate("Measurement", src_qid);
                break;

            // ==================== 2-Qubit  ====================
            case 0x10: // CNOT 
                qubits->apply_multi_gate("CNOT", {src_qid, dst_qid});
                break;
            case 0x11: // CZ  
                qubits->apply_multi_gate("CZ", {src_qid, dst_qid});
                break;
            case 0x12: // SWAP 
                qubits->apply_multi_gate("SWAP", {src_qid, dst_qid});
                break;

            default:
                break;
        }
    }
}


void SimDriver::init_qubits(int num_qubits) {
    qubits = new Qubits(num_qubits);

    std::cout << "[SimDriver] Qubit system initialized with " << num_qubits << " qubits." << std::endl;
}

void SimDriver::rst_n() {
    int current_rst_n   = dut->rst_n;
    if (m_last_rst_n == 1 && current_rst_n == 0) {
        std::cout << "[SimDriver] Detected Reset Asserted. Resetting Qubits..." << std::endl;
        qubits->reset();
    }
    m_last_rst_n       = current_rst_n;
}