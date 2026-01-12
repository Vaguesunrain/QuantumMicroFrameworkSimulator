#ifndef QUBITS_HPP
#define QUBITS_HPP

#include <vector>
#include <string>
#include <memory>
#include <iostream>


class QubitModule {
public:
    virtual ~QubitModule() = default;
    virtual void on_init(int num_qubits) {} 
    virtual void on_gate(const std::string& gate, int target_q) {}
    virtual void on_print() {}
    virtual void reset() {}
};


class Qubits {
private:
    int m_num_qubits;
  
    std::vector<std::shared_ptr<QubitModule>> m_modules;

public:
    
    Qubits(int num);

 
    void install_module(std::shared_ptr<QubitModule> mod);
    void apply_gate(std::string name, int target); 
    void print_status();
    void reset();

};

#endif