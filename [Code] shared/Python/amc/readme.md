# amc - 环境差异嗅探功能 [未完成]

用于 AI 训练的差异性对比功能，由 scm 修改而来。对此项目感兴趣，请 Fork。

:warning: ***请详细阅读 “版本历史”，避免不必要的性能、资源损耗！！！***


### 构造理论

### **授权信息**:
MIT license for everyone - from ZoMaii

### **环境信息**:
OS: Windows 11 pro
Edit Tool: 
- VSCode
- Python(MS)
- Pylance(MS)
- Python Debugger(MS)

Python:
1. Version: Python_3.11.9 64-bit (MS-Store)


### **版本历史**：
>[:notice:] 代号：WHE0_P0
>标靶任务：将 *未定义* 的架构属性/内容 **抛出** 为 *新的属性* 作为 **相对环境**。同时对并数据的不同
>此版本只会简单 roll 出代表环境的 array，此版本的遗忘机制 *不适用* WHE.base 的内存优化，只会在最后 **roll周期完成** 时释放全部