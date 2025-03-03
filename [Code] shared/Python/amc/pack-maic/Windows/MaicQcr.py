''' 
MIT License - Love From MaicNeo-Plan

通用抽象超类协议
'''

import os


class CR:
    ''' 使用此功能必须选定工作区路径(path)

    :param path: str类别，用于指定命令工作空间
    '''
    def __init__(self,path:str) -> None:
        self.boot=path

    def GetFileName(self) -> list:
        '''创建动态文件池，操作系统内的文件名

        默认输出工作空间的全部文件内容，包括但不限于文件、文件夹等
        '''
        feed=[]
        for item in os.listdir(self.boot):
            feed.append(item)
        return feed

    def GetFileAddr(self) -> list:
        '''创建动态文件地址池，操作系统内的文件地址

        默认输出工作空间的全部文件的地址，包括但不限于文件、文件夹等
        '''
        feed=[]
        for item in os.listdir(self.boot):
            feed.append(os.path.join(self.boot,item))
        return feed

    def GetFileMapping(self) -> dict:
        '''创建动态地址字典，操作系统内的文件路径映射

        为工作空间内的文件、文件夹之类的文件创建在盘中的具体位置信息
        
        特别提醒：由于Windows盘符和转义盘符一样，需要在填写应用时额外注意！
        ''' 
        feed={}
        for item in os.listdir(self.boot):
            feed[item]=str(os.path.join(self.boot,item))
        return feed

    # 创建动态内存字典，操作系统内的文件大小映射
    def GetFileSize(self) -> dict:
        feed={}
        for item in os.listdir(self.boot):
            size=os.path.getsize(os.path.join(self.boot,item))
            feed[item]=size
        return feed


# 功能测试区，对此代码进行保留
# qcr=CR(os.getcwd())
# print(qcr.)
