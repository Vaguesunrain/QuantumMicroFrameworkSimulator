#include "Qubits.hpp"

Qubits::Qubits(int num) : m_num_qubits(num) {
   
}

void Qubits::install_module(std::shared_ptr<QubitModule> mod) {
    mod->on_init(m_num_qubits); 
    m_modules.push_back(mod);
}


void Qubits::apply_gate(std::string name, int target) {
    std::cout << "[System] Applying " << name << " on Q" << target << std::endl;
    for (auto& mod : m_modules) {
        mod->on_gate(name, target);
    }
}

void Qubits::print_status() {
    for (auto& mod : m_modules) {
        mod->on_print();
    }
}

void Qubits::reset() {
    std::cout << "[System] Resetting all modules..." << std::endl;
    for (auto& mod : m_modules) {
        mod->reset(); 
    }
}