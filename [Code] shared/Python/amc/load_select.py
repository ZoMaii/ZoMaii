# 此文件为配置的互动窗口

# 程序功能页
class Send:
    def __init__(self,obj,value,warning):
        self.obj=obj
        self.value=value
        self.warning=warning
    
    def __str__(self) -> str:
        return f'[amc {self.obj}] {self.warning}> {self.value}'

    def select(self,id):
        if id:
            return id
        else:
            return Send(id)

def Root():
    try:
        root()
    except Exception as e:
        return Send('Root',f'过程权柄获取失败 <{e}>','Python')
        
# 前端显示页
def root():
    print('\n')
    print('已获取到此机器上的 AMC [Root]系统权限')
    
def meun():
    print('-'*20)
    print('[1] 节点管控')
    print('[2] 自动化脚本')
    print('\n')
    print('[默认] 所有操作都将强制执行且不检查，慎重操作')
    print('\n')

Root()
meun()
