import os
from homework.MJson.DATE import *
from homework.MJson.input_Stream import fileJsonOpen
from homework.MJson.run import run_base
from pyecharts.charts import Line
from pyecharts import options


# 系统项目值预注册
MJ_DIR = os.path.dirname(__file__)
MJ_DATABASE = os.path.join(MJ_DIR, 'json\\')


# 初始化日本数据内容
fileJP = fileJsonOpen(MJ_DATABASE+'日本.txt')
JPS = run_base(fileJP)
Jline = Line()


# print(JPS.getListData('确诊',JPS.getListName()))

# 设置时间为 X轴
# 设置情况为 Y轴
x_data = process_date_list(JPS.getUpdataDate())
Jline.add_xaxis(x_data)
JData = JPS.getListName()
for item in JData:
    Jline.add_yaxis(item,JPS.getListData(item,JData))

Jline.set_global_opts(
    # 标题设置
    title_opts=options.TitleOpts(
        title='统计表 - ' + JPS.getDataName(),
        subtitle='maic.zomaii',
    ),

    # 工具箱设置
    toolbox_opts=options.ToolboxOpts(
        is_show=True
    )
)
Jline.render('./output/日本统计图.html')