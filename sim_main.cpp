#include "Vmodule_top.h"
#include "verilated.h"
#include "verilated_vcd_c.h" 
#include "SimDriver.hpp" 
int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vmodule_top* top = new Vmodule_top;

    // --- open wave ---
    Verilated::traceEverOn(true);
    VerilatedVcdC* tfp = new VerilatedVcdC;

    SimDriver* driver = new SimDriver(top,4,1);
    top->trace(tfp, 99);
    tfp->open("wave.vcd"); 

    uint64_t main_time = 0; 

    std::cout << "[Testbench] Asserting hardware reset..." << std::endl;
    for (int rst_cycle = 0; rst_cycle < 2; rst_cycle++) {
        top->rst_n = 0; 
        
        top->clk = 0; top->eval(); tfp->dump(main_time++); 
        top->clk = 1; top->eval(); tfp->dump(main_time++); 
    }

    std::cout << "[Testbench] Releasing reset, running main simulation pipeline..." << std::endl;
    top->rst_n = 1; 

    while (main_time < 400) { 
        
        top->clk = 0;
        top->eval();
        tfp->dump(main_time++); 

        top->clk = 1;
        top->eval(); 
        
        driver->step(main_time); 
        tfp->dump(main_time++); 
    }

    tfp->close();
    delete driver;
    delete top;

    return 0;
}