# MIT 2024 From maichc.club
# 准 MVC 模型，部分模块并未深度修改

# 重构/复刻负责人:
# ZoMaii
# 版本：第一版，Python 样本，AI学习数据

import date
import maiclog.json

#####################################################
###                 视觉优化区                      ###
#####################################################

prx=30
def menu():
    print("\n")
    print('-'*prx)
    print("学生管理系统 \t 帮助输入：help")
    print('-'*prx)
    print("insert \t\t 添加学生信息")
    print("delete \t\t 删除学生信息")
    print("show \t\t 查询学生信息")
    print("update \t\t 修改学生信息")
    print("return \t\t 返回")
    print("\n")
    

def show():
    print('\n')
    print(f'全部学生信息如下：\t 来源：{date.db_data_name}')
    print('-'*prx)

    # 刷新列表
    date.Json_reload()
    date.list()


    print('-'*prx)
    print("id >按编号查找 \t name >按姓名查找 \t q> 退出")

def show_1(id):
    obj=date.list(id)
    
    
    print('\n')
    print('用户的详细信息如下：')
    print('-'*prx)
    print(f'用户名称：{obj[maiclog.json.JFItem_1]} \t\t 用户ID：{obj[maiclog.json.JFItem_0]}')
    print('\n')
    print(f'语文成绩是：{obj[maiclog.json.JFItem_2]}')
    print(f'数学成绩是：{obj[maiclog.json.JFItem_3]}')
    print(f'英语成绩是：{obj[maiclog.json.JFItem_4]}')
    print('\n')
    input('[点击任意键以退出]')


def show_2(name):
    obj=date.list(name)
    
    print('\n')
    print('用户的详细信息如下：')
    print('-'*prx)
    print(f'用户名称：{obj[maiclog.json.JFItem_1]} \t\t 用户ID：{obj[maiclog.json.JFItem_0]}')
    print('\n')
    print(f'语文成绩是：{obj[maiclog.json.JFItem_2]}')
    print(f'数学成绩是：{obj[maiclog.json.JFItem_3]}')
    print(f'英语成绩是：{obj[maiclog.json.JFItem_4]}')
    print('\n')
    input('[点击任意键以退出]')

def insert():
    print('\n')
    obj={}
    obj[maiclog.json.JFItem_0]=input(date.SN_Send('insert','','请输入创建的用户 ID'))
    obj[maiclog.json.JFItem_1]=input(date.SN_Send('insert','','请输入创建的用户 名称'))
    obj[maiclog.json.JFItem_2]=input(date.SN_Send('insert','(@grade)','请输入创建的用户 语文成绩'))
    obj[maiclog.json.JFItem_3]=input(date.SN_Send('insert','(@grade)','请输入创建的用户 数学成绩'))
    obj[maiclog.json.JFItem_4]=input(date.SN_Send('insert','(@grade)','请输入创建的用户 英语成绩'))
    return obj

def delete():
    print('\n')
    obj={}
    obj[maiclog.json.JFItem_0]=input(date.SN_Send('delete','','请输入要删除用户的 ID'))
    return obj

def update():
    print('\n')
    obj={}
    obj[maiclog.json.JFItem_0]=input(date.SN_Send('update','','请输入要更新的用户的 ID'))
    print(date.SN_Send('update','请在表格中填入此学生的新信息！','make'))
    clone=insert()
    obj[maiclog.json.JF_DataBase]=clone
    return obj


#####################################################
###                 运行输出                       ###
#####################################################
# 检查数据是否载入
if date.test():
    while(1):
        try:
            menu()
            select=input(date.SN_Send('boot'))
            if select == 'insert':
                xod=insert()
                print(f'\n 是否确定插入数据：{xod} 到数据库 {date.db_data_name}？（yes/no）')
                _se=input(date.SN_Send('insert'))
                if _se=='yes':
                    print(date.add(xod[maiclog.json.JFItem_0],xod))
                    input('[点击任意键以继续]')
                else:
                    xod.clear()
                    print(date.SN_Send('insert','数据已清除！'))
                
            elif select == 'delete':
                xod=delete()
                print(f'\n 是否确定删除用户： [{xod[maiclog.json.JFItem_0]}]（yes/no）')
                _se=input(date.SN_Send('delete'))

                if _se=='yes':
                    print(date.remove(xod[maiclog.json.JFItem_0]))
                    input('[点击任意键以继续]')
                else:
                    xod.clear()
                    print(date.SN_Send('delete','数据已清除！'))
            elif select == 'show':
                while(1):
                    show()
                    _show=input(date.SN_Send('show'))
                    # 判断输入
                    if _show == 'id':
                        _show_1=int(input(date.SN_Send('show','','请输入要查询的用户 ID')))
                        # date.list(_show_1)
                        show_1(_show_1)
                    elif _show =='name':
                        _show_2=input(date.SN_Send('show','','请输入要查询的用户 名称'))
                        # date.list(_show_2)
                        show_2(_show_2)
                    elif _show =='q' or 'quit' or 'return':
                        break
                    else:print(date.SN_Send('show','没有这项服务！'))

            elif select == 'update':
                obj=update()
                print(date.upadta(obj[maiclog.json.JFItem_0],obj[maiclog.json.JF_DataBase]))
            elif select == 'return':
                break
            elif select == 'root':
                # 运行环境测试区域
                date.list_poop()
                print(date.db_data)
                input('[点击任意键以继续]')
            else: print(date.SN_Send('boot','未知的服务'))
        except Exception as e:
            print(date.SN_Send('boot',e))
        finally:
            print(date.SN_Send('boot','此次操作将被记录！'))
else:print('数据载入异常')
