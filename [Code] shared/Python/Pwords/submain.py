import random
import json
from datetime import datetime

class NumberMonthWeekTester:
    def __init__(self):
        # 基础数据
        self.base_numbers = self._generate_base_numbers()
        self.ordinal_numbers = self._generate_ordinal_numbers()
        self.months = self._generate_months()
        self.weekdays = self._generate_weekdays()
        
        # 测试结果
        self.test_results = {
            "test_time": "",
            "total_questions": 0,
            "correct_answers": 0,
            "wrong_answers": 0,
            "accuracy": 0,
            "details": []
        }
        
        # 测试配置
        self.test_config = {
            "test_mode": "mixed",  # 测试模式: base, ordinal, month, weekday, mixed
            "base_min": 0,
            "base_max": 100,
            "ordinal_min": 1,
            "ordinal_max": 120,
            "question_count": 10,
        }
    
    def _generate_base_numbers(self):
        """生成0-100的基数词"""
        base_0_19 = [
            "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
            "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", 
            "seventeen", "eighteen", "nineteen"
        ]
        
        tens = ["", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"]
        
        numbers = {}
        # 0-19
        for i, word in enumerate(base_0_19):
            numbers[i] = word
        
        # 20-99
        for i in range(20, 100):
            if i % 10 == 0:
                numbers[i] = tens[i // 10]
            else:
                numbers[i] = f"{tens[i // 10]}-{base_0_19[i % 10]}"
        
        # 100
        numbers[100] = "one hundred"
        
        return numbers
    
    def _generate_ordinal_numbers(self):
        """生成1-120的序数词"""
        ordinal_1_19 = [
            "first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eighth", "ninth", "tenth",
            "eleventh", "twelfth", "thirteenth", "fourteenth", "fifteenth", "sixteenth", 
            "seventeenth", "eighteenth", "nineteenth"
        ]
        
        tens_ordinal = [
            "", "", "twentieth", "thirtieth", "fortieth", "fiftieth", "sixtieth", 
            "seventieth", "eightieth", "ninetieth"
        ]
        
        tens_cardinal = [
            "", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"
        ]
        
        numbers = {}
        
        # 1-19
        for i, word in enumerate(ordinal_1_19, 1):
            numbers[i] = word
        
        # 20-99
        for i in range(20, 100):
            if i % 10 == 0:
                numbers[i] = tens_ordinal[i // 10]
            else:
                numbers[i] = f"{tens_cardinal[i // 10]}-{ordinal_1_19[i % 10 - 1]}"
        
        # 100-120
        for i in range(100, 121):
            if i == 100:
                numbers[i] = "one hundredth"
            elif i < 110:
                numbers[i] = f"one hundred and {ordinal_1_19[i - 100 - 1]}"
            elif i == 110:
                numbers[i] = "one hundred and tenth"
            elif i < 120:
                numbers[i] = f"one hundred and {ordinal_1_19[i - 100 - 1]}"
            elif i == 120:
                numbers[i] = "one hundred and twentieth"
        
        return numbers
    
    def _generate_months(self):
        """生成1-12月单词"""
        return {
            1: "January",
            2: "February", 
            3: "March",
            4: "April",
            5: "May",
            6: "June",
            7: "July",
            8: "August",
            9: "September",
            10: "October",
            11: "November",
            12: "December"
        }
    
    def _generate_weekdays(self):
        """生成周一到周日单词"""
        return {
            1: "Monday",
            2: "Tuesday",
            3: "Wednesday", 
            4: "Thursday",
            5: "Friday",
            6: "Saturday",
            7: "Sunday"
        }
    
    def _get_user_config(self):
        """获取用户测试配置"""
        print("\n=== 测试配置 ===")
        
        # 选择测试模式
        print("\n选择测试模式:")
        print("1. 基数词")
        print("2. 序数词")
        print("3. 月份")
        print("4. 星期")
        print("5. 混合测试")
        
        mode_choice = input("请选择测试模式 (1-5): ").strip()
        mode_map = {
            "1": "base",
            "2": "ordinal", 
            "3": "month",
            "4": "weekday",
            "5": "mixed"
        }
        
        if mode_choice not in mode_map:
            print("无效选择，使用默认混合测试")
            self.test_config["test_mode"] = "mixed"
        else:
            self.test_config["test_mode"] = mode_map[mode_choice]
        
        # 设置题目数量
        self.test_config["question_count"] = int(input("\n测试题目数量: ") or 10)
        
        # 根据测试模式设置范围
        if self.test_config["test_mode"] in ["base", "mixed"]:
            print("\n基数词测试范围:")
            self.test_config["base_min"] = int(input("最小数字 (0-99): ") or 0)
            self.test_config["base_max"] = int(input("最大数字 (1-100): ") or 100)
        
        if self.test_config["test_mode"] in ["ordinal", "mixed"]:
            print("\n序数词测试范围:")
            self.test_config["ordinal_min"] = int(input("最小数字 (1-119): ") or 1)
            self.test_config["ordinal_max"] = int(input("最大数字 (2-120): ") or 120)
    
    def _generate_questions(self):
        """根据配置生成测试题目"""
        test_mode = self.test_config["test_mode"]
        
        if test_mode == "base":
            return self._generate_base_questions(self.test_config["question_count"])
        elif test_mode == "ordinal":
            return self._generate_ordinal_questions(self.test_config["question_count"])
        elif test_mode == "month":
            return self._generate_month_questions(self.test_config["question_count"])
        elif test_mode == "weekday":
            return self._generate_weekday_questions(self.test_config["question_count"])
        else:  # mixed
            return self._generate_mixed_questions(self.test_config["question_count"])
    
    def _generate_base_questions(self, count):
        """生成基数词题目"""
        questions = []
        min_val = max(0, self.test_config["base_min"])
        max_val = min(100, self.test_config["base_max"])
        
        for _ in range(count):
            num = random.randint(min_val, max_val)
            questions.append({
                "type": "base",
                "number": num,
                "correct_answer": self.base_numbers[num],
                "question": f"基数词 {num}: ",
                "user_answer": ""
            })
        
        return questions
    
    def _generate_ordinal_questions(self, count):
        """生成序数词题目"""
        questions = []
        min_val = max(1, self.test_config["ordinal_min"])
        max_val = min(120, self.test_config["ordinal_max"])
        
        for _ in range(count):
            num = random.randint(min_val, max_val)
            questions.append({
                "type": "ordinal",
                "number": num,
                "correct_answer": self.ordinal_numbers[num],
                "question": f"序数词 第{num}: ",
                "user_answer": ""
            })
        
        return questions
    
    def _generate_month_questions(self, count):
        """生成月份题目"""
        questions = []
        
        for _ in range(count):
            month_num = random.randint(1, 12)
            questions.append({
                "type": "month",
                "number": month_num,
                "correct_answer": self.months[month_num],
                "question": f"月份 {month_num}月: ",
                "user_answer": ""
            })
        
        return questions
    
    def _generate_weekday_questions(self, count):
        """生成星期题目"""
        questions = []
        
        for _ in range(count):
            day_num = random.randint(1, 7)
            questions.append({
                "type": "weekday",
                "number": day_num,
                "correct_answer": self.weekdays[day_num],
                "question": f"星期 {day_num}: ",
                "user_answer": ""
            })
        
        return questions
    
    def _generate_mixed_questions(self, count):
        """生成混合测试题目"""
        questions = []
        
        # 计算每种类型的题目数量
        base_count = count // 4
        ordinal_count = count // 4
        month_count = count // 4
        weekday_count = count - base_count - ordinal_count - month_count
        
        # 生成各种类型的题目
        if base_count > 0:
            questions.extend(self._generate_base_questions(base_count))
        
        if ordinal_count > 0:
            questions.extend(self._generate_ordinal_questions(ordinal_count))
        
        if month_count > 0:
            questions.extend(self._generate_month_questions(month_count))
        
        if weekday_count > 0:
            questions.extend(self._generate_weekday_questions(weekday_count))
        
        # 随机打乱题目顺序
        random.shuffle(questions)
        return questions
    
    def run_test(self):
        """运行测试"""
        self._get_user_config()
        questions = self._generate_questions()
        
        print(f"\n=== 开始测试 ===")
        print(f"测试模式: {self._get_mode_name(self.test_config['test_mode'])}")
        print(f"共 {len(questions)} 道题目")
        
        correct_count = 0
        
        for i, q in enumerate(questions, 1):
            print(f"\n[{i}/{len(questions)}] {q['question']}", end="")
            user_answer = input().strip()
            q['user_answer'] = user_answer
            
            # 更灵活的答案检查，允许连字符和空格的变化
            normalized_user_answer = user_answer.lower().replace('-', ' ').replace('  ', ' ').strip()
            normalized_correct_answer = q['correct_answer'].lower().replace('-', ' ').replace('  ', ' ').strip()
            
            if normalized_user_answer == normalized_correct_answer:
                print("✓ 正确！")
                correct_count += 1
                q['is_correct'] = True
            else:
                print(f"✗ 错误！正确答案: {q['correct_answer']}")
                q['is_correct'] = False
        
        # 记录测试结果
        self.test_results = {
            "test_time": datetime.now().isoformat(),
            "test_mode": self.test_config["test_mode"],
            "total_questions": len(questions),
            "correct_answers": correct_count,
            "wrong_answers": len(questions) - correct_count,
            "accuracy": round(correct_count / len(questions) * 100, 1),
            "details": questions
        }
        
        self._show_test_summary()
    
    def _get_mode_name(self, mode):
        """获取测试模式名称"""
        mode_names = {
            "base": "基数词",
            "ordinal": "序数词",
            "month": "月份",
            "weekday": "星期",
            "mixed": "混合测试"
        }
        return mode_names.get(mode, mode)
    
    def _show_test_summary(self):
        """显示测试总结"""
        print(f"\n=== 测试完成 ===")
        print(f"测试模式: {self._get_mode_name(self.test_results['test_mode'])}")
        print(f"正确: {self.test_results['correct_answers']}/{self.test_results['total_questions']}")
        print(f"正确率: {self.test_results['accuracy']}%")
        
        # 显示错误题目
        wrong_questions = [q for q in self.test_results['details'] if not q.get('is_correct', True)]
        if wrong_questions:
            print(f"\n错误题目 ({len(wrong_questions)}个):")
            for q in wrong_questions:
                print(f"  {q['question']} 你的答案: '{q['user_answer']}', 正确答案: '{q['correct_answer']}'")
    
    def show_statistics(self):
        """显示统计信息"""
        if not self.test_results.get('details'):
            print("暂无测试数据")
            return
        
        print(f"\n=== 统计信息 ===")
        print(f"测试时间: {self.test_results['test_time'][:19]}")
        print(f"测试模式: {self._get_mode_name(self.test_results['test_mode'])}")
        print(f"总题目数: {self.test_results['total_questions']}")
        print(f"正确率: {self.test_results['accuracy']}%")
        
        # 按类型统计
        type_stats = {}
        for q in self.test_results['details']:
            q_type = q['type']
            if q_type not in type_stats:
                type_stats[q_type] = {'total': 0, 'correct': 0}
            
            type_stats[q_type]['total'] += 1
            if q.get('is_correct', False):
                type_stats[q_type]['correct'] += 1
        
        print("\n按类型统计:")
        for q_type, stats in type_stats.items():
            accuracy = round(stats['correct'] / stats['total'] * 100, 1) if stats['total'] > 0 else 0
            type_name = {
                'base': '基数词',
                'ordinal': '序数词', 
                'month': '月份',
                'weekday': '星期'
            }.get(q_type, q_type)
            print(f"  {type_name}: {stats['correct']}/{stats['total']} ({accuracy}%)")
    
    def save_results(self):
        """保存测试结果到文件"""
        if not self.test_results.get('details'):
            print("没有测试结果可保存")
            return
        
        filename = f"test_results_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        try:
            with open(filename, 'w', encoding='utf-8') as f:
                json.dump(self.test_results, f, ensure_ascii=False, indent=2)
            print(f"✓ 测试结果已保存到: {filename}")
        except Exception as e:
            print(f"✗ 保存失败: {e}")
    
    def show_menu(self):
        """显示主菜单"""
        print("\n" + "="*40)
        print("          数字月份星期检测系统")
        print("="*40)
        print("1. 开始测试")
        print("2. 查看统计")
        print("3. 保存结果")
        print("4. 退出程序")
        print("="*40)
    
    def run(self):
        """运行主程序"""
        while True:
            self.show_menu()
            choice = input("请选择 (1-4): ").strip()
            
            if choice == '1':
                self.run_test()
            elif choice == '2':
                self.show_statistics()
            elif choice == '3':
                self.save_results()
            elif choice == '4':
                print("感谢使用，再见！")
                break
            else:
                print("无效选择，请重新输入")

def main():
    """主函数"""
    tester = NumberMonthWeekTester()
    tester.run()

if __name__ == "__main__":
    main()