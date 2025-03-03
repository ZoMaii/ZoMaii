import re


# 判断是否是闰年
def is_leap_year(year):
    return (year % 4 == 0 and year % 100 != 0) or (year % 400 == 0)


# 处理日期格式的函数
def replace_date(match, post_2020, year):
    month = int(match.group(1))  # 提取月份
    day = int(match.group(2))  # 提取日

    # 检查日期的有效性
    if month < 1 or month > 12:
        return None  # 无效的月份，舍弃

    # 检查每个月的天数是否有效
    if (month in [1, 3, 5, 7, 8, 10, 12] and day > 31) or \
            (month in [4, 6, 9, 11] and day > 30) or \
            (month == 2 and (day > 29 if is_leap_year(year) else day > 28)):
        return None  # 无效的日期，舍弃

    # 如果已经进入 2021 年，直接设为 2021
    if post_2020:
        year = 2021
    else:
        # 特别处理 12-31，应该仍然是 2020
        if month == 12 and day == 31:
            year = 2020  # 12-31 属于 2020 年
        else:
            year = 2020  # 其他情况属于 2020 年

    return f"{year}-{month:02d}-{day:02d}"


# 处理列表的函数
def process_date_list(date_list):
    processed_list = []
    post_2020 = False  # 状态变量，标识是否已经进入 2021 年

    for date_str in date_list:
        # 如果是 12-31，从下一项开始进入 2021 年，但 12-31 本身还是 2020 年
        if re.match(r'12\.31', date_str):
            post_2020 = True

        # 使用 lambda 将 post_2020 传入 replace_date 函数
        new_date_str = re.sub(r'(\d+)\.(\d+)', lambda match: replace_date(match, post_2020, 2020), date_str)

        if new_date_str:  # 舍弃无效日期
            processed_list.append(new_date_str)

    return processed_list


# # 示例输入
# date_list = ["12.29", "12.30", "12.31", "1.1", "1.5", "12.32"]
#
# # 处理日期列表
# result = process_date_list(date_list)
#
# print(result)
