#ifndef GATE_LIBRARY_HPP
#define GATE_LIBRARY_HPP

#include "Gate.hpp"
#include <map>
#include <stdexcept>
#include <complex>

class GateLibrary {
private:
    std::map<std::string, Gate> m_gate_map;

public:
    GateLibrary() {
        using namespace std::complex_literals; 
        // ==========================================
        // 1-Qubit originated gate/composite gate (
        // ==========================================

        // Pauli-X  
        MatrixXc X(2, 2); 
        X << 0, 1, 
             1, 0;
        m_gate_map.emplace("X", Gate("X", 1, 20.0, false, X));

        // Pauli-Y 
        MatrixXc Y(2, 2); 
        Y << 0, -1i, 
             1i, 0;
        m_gate_map.emplace("Y", Gate("Y", 1, 20.0, false, Y));

        // SX (\sqrt{X}, pi/2 rotation) - 20ns
        MatrixXc SX(2, 2);
        SX << std::complex<double>(0.5, 0.5),  std::complex<double>(0.5, -0.5),
              std::complex<double>(0.5, -0.5), std::complex<double>(0.5, 0.5);
        m_gate_map.emplace("SX", Gate("SX", 1, 20.0, false, SX));

        // Hadamard - 20ns
        //  Rz(pi/2) * SX * Rz(pi/2)， Rz consume 0ns，
        MatrixXc H(2, 2); 
        H << 1, 1, 
             1, -1; 
        H /= sqrt(2.0);
        m_gate_map.emplace("H", Gate("H", 1, 20.0, false, H));

        // Pauli-Z 
        MatrixXc Z(2, 2); 
        Z << 1, 0, 
             0, -1;
        m_gate_map.emplace("Z", Gate("Z", 1, 0.0, false, Z));

        // S 门 
        MatrixXc S(2, 2); 
        S << 1, 0, 
             0, 1i;
        m_gate_map.emplace("S", Gate("S", 1, 0.0, false, S));

        // S_dagger 
        MatrixXc S_dag(2, 2); 
        S_dag << 1, 0, 
                 0, -1i;
        m_gate_map.emplace("S_dag", Gate("S_dag", 1, 0.0, false, S_dag));

        // T 门 (pi/4)
        MatrixXc T(2, 2); 
        T << 1, 0, 
             0, std::complex<double>(1.0 / sqrt(2.0), 1.0 / sqrt(2.0));
        m_gate_map.emplace("T", Gate("T", 1, 0.0, false, T));

        // T_dagger (-pi/4)
        MatrixXc T_dag(2, 2); 
        T_dag << 1, 0, 
                 0, std::complex<double>(1.0 / sqrt(2.0), -1.0 / sqrt(2.0));
        m_gate_map.emplace("T_dag", Gate("T_dag", 1, 0.0, false, T_dag));


        // ==========================================
        // 2-Qubit 
        // ==========================================

        // CZ (control Z ) - 80ns
        // TODO: 详细研究——多数超导芯片（如可调耦合磁通比特）的原生双比特门，性能优于 CNOT
        // 多数超导芯片（如可调耦合磁通比特）的原生双比特门，性能优于 CNOT;
        MatrixXc CZ(4, 4);
        CZ << 1,0,0,0,
              0,1,0,0,
              0,0,1,0,
              0,0,0,-1;
        m_gate_map.emplace("CZ", Gate("CZ", 2, 80.0, true, CZ));

        // CNOT - 200ns
        // TODO: 详细研究——IBM 交叉共振（CR）架构的常用原生/半原生双比特门，微波驱动耗时较长
        // IBM 交叉共振（CR）架构的常用原生/半原生双比特门，微波驱动耗时较长
        MatrixXc CNOT(4, 4); 
        CNOT << 1,0,0,0, 
                0,1,0,0, 
                0,0,0,1, 
                0,0,1,0;
        m_gate_map.emplace("CNOT", Gate("CNOT", 2, 200.0, true, CNOT));

        // SWAP  - 600ns 
        // 3  CNOT gates (200ns * 3 = 600ns)
        MatrixXc SWAP(4, 4); 
        SWAP << 1,0,0,0, 
                0,0,1,0, 
                0,1,0,0, 
                0,0,0,1;
        m_gate_map.emplace("SWAP", Gate("SWAP", 2, 600.0, false, SWAP));
    }

    //make your own gate library
    void register_gate(const Gate& gate) {
        m_gate_map[gate.name] = gate;
    }
 
    const Gate& get(const std::string& name) const {
        auto it = m_gate_map.find(name);
        if (it == m_gate_map.end()) {

            throw std::runtime_error("Gate not found: " + name);
        }
        return it->second;
    }
};

#endif