# Mit license - love from maichc.club
# 
# QCR_Framework-Release(这是开源的正式版代码,无 debug 调试类注入,Python版本)
#
# Windows 本地Python开发实例化支持
#
# 开发标准灵感来源于 Unix/linux

import MaicQcr

class qcr(MaicQcr.CR):
    def __init__(self, path: str) -> None:
        super().__init__(path)
        
        # 定义操作系统的默认缺省值
        pwd = MaicQcr.os.getcwd()
        if self.boot == '' or self.boot=='.' or self.boot==None:
            self.boot=pwd
        elif self.boot == '..':
            # 此段主要面向Windows系统平台
            check=list(pwd.split('\\'))
            mum=len(check)-1
            self.boot=pwd[:-len(check[mum])-1]

    def root(self):
        return self.boot        
    
# 使用Python语言对译 ms-dos 和 powershell 的部分代码
class native(qcr):
    def __init__(self, path: str) -> None:
        super().__init__(path)

    def chdir(self):
        qcr.GetFileMapping(self.boot)

run=native('..')
print(run.chdir())