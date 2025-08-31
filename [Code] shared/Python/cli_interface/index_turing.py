import sys, re
from Custom_Lib import Tools

class main:
    def __init__(self):
        self.file = sys.argv[0]
        self.args = sys.argv[1:]
        self.worklist = self.__analysis_parameter(self.args)
        self.__tools = Tools()

    def __analysis_parameter(self,args:list):
        isOpts = False;nowOpts = None;work = {}
        for i in args:
            if i[0:1] == '-':
                isOpts = True
                nowOpts = i[1:]
                work[nowOpts] = []
            elif isOpts:
                work[nowOpts].append(i)
            else:pass
        return work

    def __replace_aliases(self,s:str, alias_dict:dict):
        pattern = re.compile(r'\b(' + '|'.join(re.escape(k) for k in alias_dict.keys()) + r')\b')
        return pattern.sub(lambda m: alias_dict[m.group(0)]['cli'], s)

    def worker(self,worklist:dict=None):
        if worklist is None:
            worklist = self.worklist

        if worklist == {}:
            # 无参数介入
            self.worker({'h':[]})

        invalid_chars = []
        for i in worklist:
            char = self.__replace_aliases(i,self.__tools.alias)
            if not hasattr(self.__tools, char) or not callable(getattr(self.__tools, char, None)):
                    invalid_chars.append(i)

        if invalid_chars:
            print(f"index：无 {invalid_chars} 选项可执行，已阻止本次函数执行\n")
            return

        for i in worklist:
            char = self.__replace_aliases(i, self.__tools.alias)
            method = getattr(self.__tools, char)
            method(worklist[i])

if __name__ == '__main__':
    main().worker()
else:
    print('被动调用是被禁止的行为')