import json
import maiclog.json

#####################################################
###                 文件处理区                      ###
#####################################################
#
#           未对操作系统的 I/O 进行深度处理
#
# 初始化数据层
with open('idb.json','r',encoding="utf-8") as db_file:
    db_data=json.load(db_file)

# 反馈文件读取情况
def test():return db_data

####################################################
###                   引导检索区                   ###
####################################################
Index_Clone={}                # 克隆副本

def SN_Send(level='',text='',warning=''):return f'[UA {level}] {warning}> {text}'


#####################################################
###                 JSon功能处理区                  ###
#####################################################

try:
    # Json string(auto) 表名称
    db_data_name=db_data[maiclog.json.JF_name]
    # Json Array 数据存储区
    db_data_su=db_data[maiclog.json.JF_DataBase]
except Exception as e:
    print(SN_Send('date',f'数据库文本载入失败！<{e}>','System'))
# 获取 Json 的文件结构

# 刷新 array 存储池中的 object 对象存储数量
def Json_reload():
    return len(db_data_su)

# 创建 id 引导池
def Json_id():
    obj=[]
    for i in db_data_su:
        # 和 name 一样采用数组拓扑字典序列的方式获取对象目标值，允许其他程序访问序列
        obj.append(i[maiclog.json.JFItem_0])
    return obj

# 创建 name 引导池
def Json_name():
    obj=[]
    for i in db_data_su:
        obj.append(i[maiclog.json.JFItem_1])
    return obj

# 按 ID 进行单条数据输出
def Json_item(id):
    if id in Json_id():
        # 寻找 db_data_su 的存储池
        for item in db_data_su:
            if item[maiclog.json.JFItem_0]==id:
                return item

# 按 name 进行单条数据输出
def Json_itemn(name):
    if name in Json_name():
        for item in db_data_su:
            if item[maiclog.json.JFItem_1]==name:
                return item
        
# 通过 id 获取单条 JSon数据对象 Object 在 db_Data_su 中的位置
def Json_item_bit(id):
    # 查看动态池是否存在对应 id 地址
    if id in Json_id():
        x=0
        for i in Json_id():
            # 记录 id 在动态池里面是在第几位
            if id == i:
                return str(x)
            x=x+1
    else:return False

# 询问新加 id 与池内 id 是否冲突(是冲突的吗？)
def Json_id_onlist(id):
    # 冲突
    if id in Json_id():return True

    # 不冲突
    else:return False

#####################################################
###                 程序功能区                      ###
#####################################################
#
# 可以另外设置一个中间层管理，因为用于 AI 学习，并没有独立出来

# 定义增
# 这里单纯的定义为了对象操作，id属于功能划分时的预留位，后弃用。
def add(id,ary):
    # 修正指针指向的切片数据，将其对象化(!!! 对象化步骤不可使用内存替换 !!!)
    try:
        le=True
        ary[maiclog.json.JFItem_0]=int(ary[maiclog.json.JFItem_0])
        ary[maiclog.json.JFItem_1]=str(ary[maiclog.json.JFItem_1])
        ary[maiclog.json.JFItem_2]=int(ary[maiclog.json.JFItem_2])
        ary[maiclog.json.JFItem_3]=int(ary[maiclog.json.JFItem_3])
        ary[maiclog.json.JFItem_4]=int(ary[maiclog.json.JFItem_4])
    except Exception as e:
        le=False
        return SN_Send('date',f'数据初始化失败 <{e}>','add')

    if le:
        # 查验 ID 命名是否重复
        if Json_id_onlist(ary[maiclog.json.JFItem_0]):return SN_Send('date','此用户的 ID 命名重复了！','add')
        else:
            # 成功格式化数据后执行内容
            db_data_su.append(ary)
            return SN_Send('date','已成功插入数据，用户创建成功！')
    else:return SN_Send('date','已拒绝数据插入请求!','add')
    
# 定义删
def remove(id):
    # Python 参数为指向，而未实例化，所以可以更改
    try:
        # 特别说明：整数 0，1 在运算层并非严格的 true 与 false,所以需要判断其参数类型
        # 在此是判断 Json_item_bit 的运算返回值是否为 string.
        if isinstance(Json_item_bit(int(id)),str):
            db_data_su.pop(int(Json_item_bit(id)))
            return SN_Send('date','已成功移除用户！')
        else:return SN_Send('date','用户不存在，删除失败','remove')
    except Exception as e:
        return SN_Send('date',f'数据初始化失败 <{e}>','remove')

# 定义查,默认全输出,将显示的权限移交外部，方便前端多次调用(再封装)
def list(vaule="none"):
    if vaule == "none":
        for i in db_data_su:
            print(i)
    elif isinstance(vaule,int):
        print(Json_item(vaule))
        return Json_item(vaule)
    elif isinstance(vaule,str):
        print(Json_itemn(vaule))
        return Json_itemn(vaule)
    else:pass

            
# 定义改
def upadta(id,ary):
    try:
        x=int(id)
        input_id=int(ary[maiclog.json.JFItem_0])
        if isinstance(x,int):
            # 在 动态ID 池中存在才更改,填写 x 变量防止id类型
            if x in Json_id():
                remove(x)
                # 校验要添加的对象 所占用的ID 是否重复
                if input_id in Json_id():
                    return SN_Send('date','提交的 ID 变更申请失败<此 ID 已被占用>','updata')
                else:add(id,ary)
                return SN_Send('date','已成功更改数据')
            else:return SN_Send('date','未找到指定用户','updata')
        else:return SN_Send('date','此用户的 ID 命名重复了！','updata')
    except Exception as e:
        return SN_Send('date',f'数据初始化失败 <{e}>','remove')

# 冒泡排序功能,2024/5/12 附加
# 大学生别 Call 我了，就这破事，烦死了。
def list_poop(way='id'):
    if way=='id':
        arry=Json_id()

    if arry:
        n=len(arry)
        for x in range(n):
            # 减一追随数组排序，每次向下比较一位
            for i in range(0,n-x-1):
                if arry[i]>arry[i+1]:
                    arry[i],arry[i+1]=arry[i+1],arry[i]
        # 根据 ID 去查找对象
        for obj in arry:
            print(Json_item(obj))
    else:return SN_Send('date','自己加索引去','list_poop')
    

# 加功能实验
# ls={'id': '100', 'name': 'o', 'cn': '100', 'math': '100', 'en': '100'}
# print(f"ID是：{ls['id']}，数据是{ls}")
# print(id(ls))
# print('-'*20)
# print(add(ls['id'],ls))
# list()

# ls={'id': '100', 'name': 'o', 'cn': '80', 'math': '10', 'en': '200'}
# print(add(ls['id'],ls))
# list()

# 删功能实验
# list()
# remove(1004)
# list()

# # 改功能实验
# ls=[{'id': '100', 'name': '张三', 'cn': '100', 'math': '100', 'en': '100'},{'id': '101', 'name': '李四', 'cn': '100', 'math': '100', 'en': '100'}]
# for item in ls:
#     add(item['id'],item)
# list()
# print('-'*20)


# remove(100)
# list()
# print('-'*20)

# ls=[{'id': '100', 'name': '张三', 'cn': '80', 'math': '80', 'en': '70'},{'id': '101', 'name': '李四', 'cn': '90', 'math': '80', 'en': '85'}]
# for item in ls:
#     upadta(item['id'],item)
# list()