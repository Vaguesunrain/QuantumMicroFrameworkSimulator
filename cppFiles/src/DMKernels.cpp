#include "QubitModule/DMKernels.hpp"
#include <omp.h>
#include <random>    
#include <algorithm> 
#include <cmath>    
namespace {

    static inline size_t insert_bit(size_t val, int pos) {
        size_t mask = (1ULL << pos) - 1;
        return ((val & ~mask) << 1) | (val & mask);
    }

    //insert_two_zeros at positions q1 and q2,where q1 < q2
    static inline size_t insert_two_zeros(size_t val, int q1, int q2) {
        if (q1 > q2) std::swap(q1, q2);
        // insert 0 at position q1
        size_t res = insert_bit(val, q1);
        return insert_bit(res, q2);
    }
}

namespace DMKernels {
   // Math: rho_new = U * rho_sub * U_dag
    void apply_single_qubit_gate(std::complex<double>* rho, size_t dim, 
                             int target, const Eigen::Matrix2cd& U) {
        size_t target_mask = 1ULL << target;
        std::complex<double> u00 = U(0,0), u01 = U(0,1), u10 = U(1,0), u11 = U(1,1);
        std::complex<double> u00_c = std::conj(u00), u01_c = std::conj(u01),
                            u10_c = std::conj(u10), u11_c = std::conj(u11);

        #pragma omp parallel for schedule(static)
        for (size_t r_idx = 0; r_idx < dim / 2; ++r_idx) {
            size_t r0 = insert_bit(r_idx, target);
            size_t r1 = r0 | target_mask;
            std::complex<double>* ptr0 = rho + r0 * dim;
            std::complex<double>* ptr1 = rho + r1 * dim;

            for (size_t c_idx = 0; c_idx < dim / 2; ++c_idx) {
                size_t c0 = insert_bit(c_idx, target);
                size_t c1 = c0 | target_mask;

            
                std::complex<double> r00 = ptr0[c0], r01 = ptr0[c1];
                std::complex<double> r10 = ptr1[c0], r11 = ptr1[c1];

                // rho = U * rho * U_dag
                // 1. t = U * rho
                std::complex<double> t00 = u00*r00 + u01*r10;
                std::complex<double> t01 = u00*r01 + u01*r11;
                std::complex<double> t10 = u10*r00 + u11*r10;
                std::complex<double> t11 = u10*r01 + u11*r11;

                // 2. res = t * U_dag
                ptr0[c0] = t00*u00_c + t01*u01_c;
                ptr0[c1] = t00*u10_c + t01*u11_c;
                ptr1[c0] = t10*u00_c + t11*u01_c;
                ptr1[c1] = t10*u10_c + t11*u11_c;
            }
        }
    }


    //parameters: control bit , target bit , single-qubit matrix V on target bit
    // logic：rho' = CU * rho * CU_dag
    // we do not need a full 4x4 matrix here , just a 2x2 matrix V
    void apply_controlled_gate(std::complex<double>* rho, size_t dim, 
                           int ctrl, int target, const Eigen::Matrix2cd& V) {
        size_t ctrl_mask = 1ULL << ctrl;
        size_t target_mask = 1ULL << target;

       
        std::complex<double> v00 = V(0,0), v01 = V(0,1), v10 = V(1,0), v11 = V(1,1);
        std::complex<double> v00_c = std::conj(v00), v01_c = std::conj(v01),
                            v10_c = std::conj(v10), v11_c = std::conj(v11);

        #pragma omp parallel for schedule(static)
        for (size_t r_idx = 0; r_idx < dim / 2; ++r_idx) {
            size_t r0 = insert_bit(r_idx, target); 
            size_t r1 = r0 | target_mask;
            bool r_ctrl = (r0 & ctrl_mask); // check if this pair of rows has control bit

            std::complex<double>* ptr0 = rho + r0 * dim;
            std::complex<double>* ptr1 = rho + r1 * dim;

            for (size_t c_idx = 0; c_idx < dim / 2; ++c_idx) {
                size_t c0 = insert_bit(c_idx, target);
                size_t c1 = c0 | target_mask;
                bool c_ctrl = (c0 & ctrl_mask); // check if this pair of columns has control bit

                // --- core skip ---
                if (!r_ctrl && !c_ctrl) continue; 

                std::complex<double> r00 = ptr0[c0], r01 = ptr0[c1];
                std::complex<double> r10 = ptr1[c0], r11 = ptr1[c1];

                if (r_ctrl && c_ctrl) {
                    //  V * rho * V_dag
                    std::complex<double> t00 = v00*r00 + v01*r10;
                    std::complex<double> t01 = v00*r01 + v01*r11;
                    std::complex<double> t10 = v10*r00 + v11*r10;
                    std::complex<double> t11 = v10*r01 + v11*r11;

                    ptr0[c0] = t00*v00_c + t01*v01_c;
                    ptr0[c1] = t00*v10_c + t01*v11_c;
                    ptr1[c0] = t10*v00_c + t11*v01_c;
                    ptr1[c1] = t10*v10_c + t11*v11_c;
                } 
                else if (r_ctrl) {
                    // V * rho
                    ptr0[c0] = v00*r00 + v01*r10;
                    ptr0[c1] = v00*r01 + v01*r11;
                    ptr1[c0] = v10*r00 + v11*r10;
                    ptr1[c1] = v10*r01 + v11*r11;
                } 
                else {
                    //  rho * V_dag
                    ptr0[c0] = r00*v00_c + r01*v01_c;
                    ptr0[c1] = r00*v10_c + r01*v11_c;
                    ptr1[c0] = r10*v00_c + r11*v01_c;
                    ptr1[c1] = r10*v10_c + r11*v11_c;
                }
            }
        }
    }


    //parameter: two target qubits q1, q2. For convience , we require q1 < q2
    void apply_general_2q_gate(std::complex<double>* rho, size_t dim, 
                               int q1, int q2, const Eigen::Matrix4cd& U){
        if (q1 > q2) std::swap(q1, q2); 

        size_t mask1 = 1ULL << q1;
        size_t mask2 = 1ULL << q2;

        Eigen::Matrix4cd U_dag = U.adjoint(); 

      
        // outer loop: indexes for rows
        #pragma omp parallel for schedule(static)
        for (size_t r_i = 0; r_i < dim / 4; ++r_i) {
            size_t r00 = insert_two_zeros(r_i, q1, q2);
            size_t r_indices[4] = { r00, r00 | mask1, r00 | mask2, r00 | mask1 | mask2 };
            // extract row pointers,to reduce multiplication overhead in inner loops
            std::complex<double>* r_ptrs[4];
            for(int k=0; k<4; ++k) r_ptrs[k] = rho + r_indices[k] * dim;

            // inner loop: indexes for columns
            for (size_t c_i = 0; c_i < dim / 4; ++c_i) {
                size_t c00 = insert_two_zeros(c_i, q1, q2);
                size_t c_indices[4] = { c00, c00 | mask1, c00 | mask2, c00 | mask1 | mask2 };
                //1. load 4x4 sub-matrix to register/stack
                std::complex<double> rho_sub[4][4];
                for (int i = 0; i < 4; ++i) {
                    for (int j = 0; j < 4; ++j) {
                        rho_sub[i][j] = r_ptrs[i][c_indices[j]];
                    }
                }

                // 2. tmp = U * rho_sub
                std::complex<double> tmp[4][4];
                for (int i = 0; i < 4; ++i) {     // Row of U
                    for (int j = 0; j < 4; ++j) { // Col of rho_sub
                        std::complex<double> sum = 0;
                        for (int k = 0; k < 4; ++k) {
                            sum += U(i, k) * rho_sub[k][j];
                        }
                        tmp[i][j] = sum;
                    }
                }

                // 3.  final = tmp * U_dag
                for (int i = 0; i < 4; ++i) {     // Row of tmp
                    for (int j = 0; j < 4; ++j) { // Col of U_dag
                        std::complex<double> val = 0;
                        for (int k = 0; k < 4; ++k) {
                            val += tmp[i][k] * U_dag(k, j);
                        }
                        // Write back
                        r_ptrs[i][c_indices[j]] = val;
                    }
                }
            }
        }
    }


    void apply_swap(std::complex<double>* rho, size_t dim, 
                    int q1, int q2) {
        size_t mask1 = 1ULL << q1;
        size_t mask2 = 1ULL << q2;
        size_t combo_mask = mask1 | mask2;
        
        // It is a ram swap operation: rho(i, j) <-> rho(swap(i), swap(j))
        // we iterate all elements in rho,but only swap when current index < swapped index
        #pragma omp parallel for schedule(static)
        for (size_t r = 0; r < dim; ++r) {
           
            size_t r_swap = r;
            if ( ((r & mask1) != 0) != ((r & mask2) != 0) ) { // XOR check
                r_swap = r ^ combo_mask;
            }

            for (size_t c = 0; c < dim; ++c) {
                
                size_t c_swap = c;
                if ( ((c & mask1) != 0) != ((c & mask2) != 0) ) {
                    c_swap = c ^ combo_mask;
                }

                bool current_is_smaller = (r < r_swap) || ((r == r_swap) && (c < c_swap));

                if (current_is_smaller) {
                    std::swap(rho[r * dim + c], rho[r_swap * dim + c_swap]);
                }
            }
        }
    }




    int measure_single_qubit(std::complex<double>* rho, size_t dim, int target) {
        if (!rho) return -1;

        size_t target_mask = 1ULL << target;
        double p0 = 0.0;

        // 1. 高并发计算 p0 (只遍历符合条件的对角线元素，循环次数减半)
        #pragma omp parallel for reduction(+:p0) schedule(static)
        for (size_t idx = 0; idx < dim / 2; ++idx) {
            size_t i = insert_bit(idx, target); //  target  0
            // 行优先存储下，对角线元素 (i, i) 的一维偏移量为 i * dim + i
            p0 += std::max(0.0, rho[i * dim + i].real());
        }

        // 2. 随机采样
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);

        int outcome = (dis(gen) < p0) ? 0 : 1;
        double p_outcome = (outcome == 0) ? p0 : (1.0 - p0);

        // 3. 状态坍缩 (并行行扫描)
        if (p_outcome > 1e-15) {
            double scale = 1.0 / p_outcome;

            if (outcome == 0) {
                #pragma omp parallel for schedule(static)
                for (size_t r = 0; r < dim; ++r) {
                    bool r_bit = (r & target_mask) != 0;
                    std::complex<double>* row_ptr = rho + r * dim;

                    if (r_bit) {
                        // 行索引不符：整行瞬间归零 (高度优化，触发 std::fill_n 的向量化写入)
                        std::fill_n(row_ptr, dim, std::complex<double>(0.0, 0.0));
                    } else {
                        // 行索引相符：处理列
                        for (size_t c_idx = 0; c_idx < dim / 2; ++c_idx) {
                            size_t c0 = insert_bit(c_idx, target);
                            size_t c1 = c0 | target_mask;
                            row_ptr[c0] *= scale; // 保留项归一化
                            row_ptr[c1] = 0.0;    // 冲突项归零
                        }
                    }
                }
            } 
            else { // outcome == 1
                #pragma omp parallel for schedule(static)
                for (size_t r = 0; r < dim; ++r) {
                    bool r_bit = (r & target_mask) != 0;
                    std::complex<double>* row_ptr = rho + r * dim;

                    if (!r_bit) {
                        // 行索引不符：整行归零
                        std::fill_n(row_ptr, dim, std::complex<double>(0.0, 0.0));
                    } else {
                        // 行索引相符：处理列
                        for (size_t c_idx = 0; c_idx < dim / 2; ++c_idx) {
                            size_t c0 = insert_bit(c_idx, target);
                            size_t c1 = c0 | target_mask;
                            row_ptr[c0] = 0.0;    // 冲突项归零
                            row_ptr[c1] *= scale; // 保留项归一化
                        }
                    }
                }
            }
        } 
        else {
            // 极低概率边界处理：若出现除零风险，强行将矩阵重置为目标结果的纯态
            #pragma omp parallel for schedule(static)
            for (size_t r = 0; r < dim; ++r) {
                std::fill_n(rho + r * dim, dim, std::complex<double>(0.0, 0.0));
            }
            // 寻找第一个符合测量结果的基态，将其概率设为 1.0
            for (size_t i = 0; i < dim; ++i) {
                if (((i & target_mask) != 0) == (outcome == 1)) {
                    rho[i * dim + i] = 1.0;
                    break;
                }
            }
        }

        return outcome;
    }
/*
ToDo:
还可以压榨的性能点：
SIMD (AVX-512)：目前的 std::complex 很难自动向量化。如果你极度追求性能，可以将实部和虚部分开存储（Structure of Arrays, SoA），或者使用 reinterpret_cast<double*>(rho) 将其视为 double 数组，手写 AVX-512 Intrinsics 来一次处理 4 个复数（8 个 double）。这通常能再带来 2-4 倍的提升。
Cache Blocking (分块)：虽然现在的遍历方式对于行是连续的，但如果 col0 和 col1 跨度很大（即 target qubit 索引很大），会导致 Cache Thrashing。可以引入 Cache Blocking 技术，将内层循环分块处理，保证处理的数据块能放入 L2 Cache。
*/
}