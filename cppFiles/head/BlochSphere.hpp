#ifndef BLOCH_SPHERE_HPP
#define BLOCH_SPHERE_HPP

#include "Qubits.hpp"
#include <vector>
#include <cmath>
#include <iomanip>

struct Vector3 { double x, y, z; };


class BlochSphereModule : public QubitModule {
private:
    std::vector<Vector3> m_vectors; 

public:
    
    void on_init(int num) override {
        m_vectors.resize(num, {0.0, 0.0, 1.0});
        std::cout << "  -> [Bloch] Initialized storage for " << num << " vectors.\n";
    }

    void on_gate(const std::string& gate, int target) override {
        Vector3& v = m_vectors[target];

        if (gate == "X") {
            // X 门：绕 X 轴旋转 180 度 -> (x, -y, -z)
            v.y = -v.y;
            v.z = -v.z;
        } 
        else if (gate == "H") {
            // H 门简单模拟：把 Z (1) 变成 X (1)
            double temp = v.z;
            v.z = v.x;
            v.x = temp;
        }
        // ... 其他门 ...
    }

    // 打印状态
    void on_print() override {
        std::cout << "--- Bloch Sphere Status ---\n";
        for (size_t i = 0; i < m_vectors.size(); ++i) {
            const auto& v = m_vectors[i];
            std::cout << "Q" << i << ": [" 
                      << std::fixed << std::setprecision(2)
                      << v.x << ", " << v.y << ", " << v.z << "]\n";
        }
    }

    void reset() {
        for (auto& v : m_vectors) {
            v = {0.0, 0.0, 1.0};
        }
        std::cout << "  -> [Bloch] All vectors reset to |0> state.\n";
    }
};

#endif