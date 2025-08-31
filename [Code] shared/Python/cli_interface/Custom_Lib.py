import json
import os,shutil

class Tools:
    def __init__(self):
        self.__work = ABL_SYSTEM()
        self.alias = {
            's':{'cli':'sketch','help':"-s\t{}\t初始化速写文件或获取对应值\n\t\t-s 创建速写文件\n\t\t-s <JSON检索...> 检索速写文件的存储数据\n"},
            'h':{'cli':'help','help':"-h\t{}\t显示帮助信息\n"},
            't':{'cli':'template','help':"-t\t{}\t在 sketch 的基础上深度定制网页页面与功能\n"},
            'v':{'cli':'version','help':"-v\t{}\t显示当前 LIB 版本信息\n"}
        }

    def __empty_arg(self,arg):
        if arg != []:
            print(
                "此外，你似乎附加了此版本没有的参数 {} ，它在此工具版本中并非必须的！\n".format(arg)
            )

    def help(self,arg=None):
        print(
         "LIB - Help from MaicHubClub \n"
        )

        for i in self.alias.values():
            print(i['help'].format(i['cli']))

        self.__empty_arg(arg)

    def version(self,arg=None):
        print(
            '\n'
            "LIB - MaicHubClub 20250830 Cor.0 \n",
            "---【open-source GitHub 】---\n"
            "LIB：这是 GitHub 开源代码版本，无 Hash 值验证、i18n 本地化语言适配、并发处理(如:-vh) 等其他未完全列举内容。\n"
            "Other：如遇已有功能在使用过程中存在 BUG、安全威胁、无法使用的情况，请在 GitHub 提交问题(issues)！\n"
            "Tips：主页位于：https://github.com/zomaii/，请提前确认此项目是否属于支持的生命周期！\n"
            '\n'
        )
        self.__empty_arg(arg)

    def sketch(self,arg=None):
        if arg != None and arg != []:
            print(self.__work.ShowSketchFile(arg))
        else:self.__work.CreatSketchFile()

    def template(self,parameter:list[str]):
        print(parameter[0])



class ABL_SYSTEM:
    def __init__(self):
        self.script_dir = os.path.dirname(os.path.abspath(__file__))
        self.script_root_dir = os.path.dirname(self.script_dir)
        self.pwd_dir = os.getcwd()

    def CreatSketchFile(self):
        space = os.path.join(self.script_root_dir,'Archive','.lib')
        source_file = os.path.join(space,'sketch.json')
        pwd_file = os.path.join(self.pwd_dir,'sketch.json')
        if os.path.exists(pwd_file):
            if input('SKETCH程序：当前文件夹内已存在速写文件，是否重置它？（Y/N）') == 'Y':
                shutil.copy(source_file,pwd_file)
        else:
            shutil.copy(source_file,self.pwd_dir)

    def ShowSketchFile(self,parameter:list[str]):
        try:
            with open(os.path.join(self.pwd_dir,'sketch.json')) as f:
                sketch = json.load(f)
            Query = sketch
            for i in parameter:Query = Query[i]
            return Query
        except Exception as e:
            print('无法检索到 {} 的存储索引'.format(e))
            return 'ERROR\n'

