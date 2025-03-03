import json

# MIT 2024 From ZoMaii
# 17点23分 2024/5/11
##############################################
#                                            #
#    面向对象编程，代码不会考虑主动优化循环性能     #
#                                            #
##############################################

# 准备对应的 JSON 文件，创建数据读写层的访问对象
with open('idb.json','r',encoding="utf-8") as db_file:
    db_data=json.load(db_file)

class personal:
    # 定义 ID，名称，绩效
    # 本应该有三个参数，但因为我懒，直接把 cn,math,en 固定在关系当中，没有合并为 Json Object 对象 grade
    # ~~ 这是为了降低AI学习难度！ ~~
    # 凑合着用吧，反正难受的不是我，需要的话，加上'.属性[对应键]'改改就行了
    def __init__(self,id,name,cn,math,en):
        self.id=id
        self.name=name
        self.cn=cn
        self.math=math
        self.en=en
    
    # 返回 json 的 object 对象节点
    def __str__(self):
        return f'"id":{self.id},"姓名":{self.name},"中文成绩":{self.cn},"数学成绩":{self.math},"英语成绩":{self.en}'
    
# 定义系统类
class UserAdminSystem:
    # 初始化存储 Json 的 arry 数组
    def __init__(self):
        self.personal=[]

#######################################################################
###                         核心对象关系中心区                         ###
#######################################################################

    # 完全查询，未设计冒泡排序
    # 冒泡思路 str=[],str[i],str[i+1]=str[i+1],str[i] ,如不支持这种表达式尝试加入第三存储区:op=[]
    # a1>a2则交换

    def list(self):
        for personal in self.personal:
            print(personal)
    # 定义增
    def add(self,id,name,cn,math,en):
        # 确定增加的方式为指针指向内存的方式
        self.personal.append(personal(id,name,cn,math,en))
    # 定义查，支持 ID 查询
    def list_id(self,id):
        # 确定循环次数，循环 personal 本身数组查找.id 属性是否符合条件
        for personal in self.personal:
            if personal.id == id:
                print(personal)
                break
            else: print('[UA list] > 非目标用户')

    # 定义删，只能ID精准删除
    def remove(self,id):
        # 铆钉Json Object对象内的id
        for personal in self.personal:
            if personal.id == id:
                self.personal.remove(personal)
            else:print('[UA list] > 非目标用户')

    # 定义改（重新上传覆盖，降低属性修改复杂程度）
    def updata(self,id,name,cn,math,en):
        for personal in self.personal:
            if personal.id == id:
                UA.remove(10)
                UA.add(id,name,cn,math,en)
                break
            else:print('[UA list] > 非目标用户')

#######################################################################
###               美观设计，不返回提示（非性能提升的不显示）               ###
#######################################################################
            
    def _list_id(self,id):
        # 非显示
        for personal in self.personal:
            if personal.id == id:
                print(personal)
                break

    def _updata(self,id,name,cn,math,en):
        # 非提示
        for personal in self.personal:
            if personal.id == id:
                UA._remove(10)
                UA.add(id,name,cn,math,en)
                break

    def _remove(self,id):
        # 非提示
        for personal in self.personal:
            if personal.id == id:
                self.personal.remove(personal)

    # 冒泡排序显示
    def _list(self):
        obj=[]
        for personal in self.personal:
            # 获取到对应的 ID 属性
            obj.append(personal.id)
        #开始冒泡
        mum=len(obj)
        for i in range(mum):
            # 追随数组获取列表,减一防溢出,减 i 实现列表位的读取指针移动
            for e in range(0,mum-i-1):
                if obj[e]>obj[e+1]:
                    obj[e],obj[e+1]=obj[e+1],obj[e]
        for u in obj:
            UA._list_id(u)

# 实例化程序系统类
UA=UserAdminSystem()

# 载入Json 文件信息
for item in db_data:
    UA.add(int(item),db_data[item].get("name"),db_data[item].get('cn'),db_data[item].get('math'),db_data[item].get('en'))


# 将大部分功能交予前端设计
print('创建角色')
UA.add(10,"王五",100,100,100)
UA._list()
print('\n')

# 修改不能动 ID!只能移除 ID 重新添加
print('更改王五数据并展示')
UA._updata(10,"王五",20,30,40)
UA._updata(1,"王五",20,30,40)
UA._list_id(10)
print('\n')

print('移除王五并展示')
UA._remove(10)
print('\n')

print('目前存在的用户：')
UA.list()
