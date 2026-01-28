## Qubit model

```mermaid

classDiagram
    class Qubits {
        -int m_num_qubits
        -vector~shared_ptr~QubitModule~~ m_modules
        +install_module(mod)
        +apply_gate(name, target)
        +reset()
        +print_status()
    }

    class QubitModule {
        <<Interface>>
        +on_init(num)
        +on_gate(name, target)
        +reset()
        +on_print()
    }

    class BlochSphereModule {
        -vector~Vector3~ m_vectors
        +on_init(num)
        +on_gate(name, target)
        +reset()
        +on_print()
    }

    class StateVectorModule {
        -vector~complex~ m_amplitudes
        +on_init(num)
        +on_gate(name, target)
        +reset()
        +on_print()
    }

    %% 关系描述
    Qubits o-- QubitModule : 聚合 (拥有列表)
    BlochSphereModule --|> QubitModule : 继承/实现
    StateVectorModule --|> QubitModule : 继承/实现

    %% 注释
    note for Qubits "总线 / 传声筒\n负责分发指令"
    note for BlochSphereModule "插件 A\n处理 Bloch 旋转"
    note for StateVectorModule "插件 B\n处理全态演化"
```


## 时钟树

1.建议分区域，qubit附近采用差分低频信号
2.注意在差分线路加入延时
3.使用时钟分发模块
4.避免glitch
5.差分信号做低摆幅评估


## 实验
1.慢速有时间差的时钟向qubit和gates发送流水线指令
vs
2.指令堆集在qubit附近，传入低平时钟，然后在PLL产高速时钟爆发执行