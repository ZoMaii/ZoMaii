import json
import random
import os
from datetime import datetime

class WordManager:
    def __init__(self):
        self.words = []
        self.wrong_words = []  # 回答错误的单词
        self.unknown_words = []  # 不知道的单词（空输入）
        self.current_file = ""
        
    def input_mode(self):
        """录入模式"""
        print("\n=== 单词录入模式 ===")
        print("输入 'q' 退出录入模式")
        
        while True:
            word = input("\n单词: ").strip()
            if word.lower() == 'q':
                break
                
            pos = input("词性: ").strip()
            meaning = input("意思: ").strip()
            
            if word and pos and meaning:
                self.words.append({
                    "word": word,
                    "pos": pos,
                    "meaning": meaning
                })
                print(f"✓ 已添加: {word}")
            else:
                print("✗ 输入不完整，请重新输入")
        
        if self.words:
            self.save_to_json()
    
    def save_to_json(self):
        """保存到JSON文件"""
        if not self.words:
            print("没有数据需要保存")
            return
            
        filename = input("\n输入保存的文件名 (不带.json后缀): ").strip()
        if not filename:
            filename = f"words_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        
        filename = f"{filename}.json"
        
        data = {
            "created_time": datetime.now().isoformat(),
            "total_words": len(self.words),
            "words": self.words
        }
        
        try:
            with open(filename, 'w', encoding='utf-8') as f:
                json.dump(data, f, ensure_ascii=False, indent=2)
            print(f"✓ 已保存到文件: {filename}")
            self.current_file = filename
        except Exception as e:
            print(f"✗ 保存失败: {e}")
    
    def load_from_json(self):
        """从JSON文件加载数据"""
        json_files = [f for f in os.listdir('.') if f.endswith('.json')]
        
        if not json_files:
            print("没有找到JSON文件")
            return False
        
        print("\n可用的JSON文件:")
        for i, file in enumerate(json_files, 1):
            print(f"{i}. {file}")
        
        try:
            choice = int(input("\n选择文件编号: "))
            if 1 <= choice <= len(json_files):
                self.current_file = json_files[choice-1]
                with open(self.current_file, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                self.words = data['words']
                print(f"✓ 已加载文件: {self.current_file}, 共 {len(self.words)} 个单词")
                return True
            else:
                print("无效的选择")
                return False
        except (ValueError, IndexError) as e:
            print(f"加载失败: {e}")
            return False
    
    def recite_mode(self):
        """记背模式 - 随机选择两种测试方式，区分错误和不知道"""
        if not self.words:
            if not self.load_from_json():
                return
        
        print("\n=== 单词记背模式 ===")
        print("输入 'q' 退出记背模式")
        print("直接回车表示不知道")
        
        # 创建待测试的单词副本
        test_words = self.words.copy()
        random.shuffle(test_words)
        self.wrong_words = []
        self.unknown_words = []
        
        tested_count = 0
        total_count = len(test_words)
        
        while test_words:
            word_obj = test_words.pop()
            tested_count += 1
            
            # 随机选择测试方式
            test_type = random.choice(["word_to_meaning", "meaning_to_word"])
            
            print(f"\n进度: {tested_count}/{total_count}")
            
            if test_type == "word_to_meaning":
                # 方式1: 给出单词和词性，要求输入意思
                print(f"单词: {word_obj['word']}")
                print(f"词性: {word_obj['pos']}")
                user_input = input("请输入意思: ").strip()
                
                if user_input.lower() == 'q':
                    print("退出记背模式")
                    break
                
                # 处理空输入（不知道）
                if not user_input:
                    print(f"不知道。正确答案: {word_obj['meaning']}")
                    # 记录不知道的单词
                    unknown_info = word_obj.copy()
                    unknown_info["test_type"] = "单词→意思"
                    unknown_info["user_answer"] = "(不知道)"
                    self.unknown_words.append(unknown_info)
                # 检查答案是否正确
                elif user_input.lower() == word_obj['meaning'].lower():
                    print("✓ 正确！")
                else:
                    print(f"✗ 错误！正确答案: {word_obj['meaning']}")
                    # 记录错误信息，包括测试方式
                    wrong_info = word_obj.copy()
                    wrong_info["test_type"] = "单词→意思"
                    wrong_info["user_answer"] = user_input
                    self.wrong_words.append(wrong_info)
                    
            else:
                # 方式2: 给出意思和词性，要求输入单词
                print(f"意思: {word_obj['meaning']}")
                print(f"词性: {word_obj['pos']}")
                user_input = input("请输入单词: ").strip()
                
                if user_input.lower() == 'q':
                    print("退出记背模式")
                    break
                
                # 处理空输入（不知道）
                if not user_input:
                    print(f"不知道。正确答案: {word_obj['word']}")
                    # 记录不知道的单词
                    unknown_info = word_obj.copy()
                    unknown_info["test_type"] = "意思→单词"
                    unknown_info["user_answer"] = "(不知道)"
                    self.unknown_words.append(unknown_info)
                # 检查答案是否正确
                elif user_input.strip().lower() == word_obj['word'].lower():
                    print("✓ 正确！")
                else:
                    print(f"✗ 错误！正确答案: {word_obj['word']}")
                    # 记录错误信息，包括测试方式
                    wrong_info = word_obj.copy()
                    wrong_info["test_type"] = "意思→单词"
                    wrong_info["user_answer"] = user_input
                    self.wrong_words.append(wrong_info)
        
        if tested_count > 0:
            correct_count = tested_count - len(self.wrong_words) - len(self.unknown_words)
            accuracy = correct_count / tested_count * 100
            print(f"\n记背完成！")
            print(f"正确: {correct_count}/{tested_count} ({accuracy:.1f}%)")
            print(f"错误: {len(self.wrong_words)}/{tested_count}")
            print(f"不知道: {len(self.unknown_words)}/{tested_count}")
        
        if self.wrong_words or self.unknown_words:
            self.summary_mode()
    
    def summary_mode(self):
        """总结模式 - 分别显示错误单词和不知道的单词"""
        if not self.wrong_words and not self.unknown_words:
            print("\n恭喜！所有单词都答对了！")
            return
        
        # 显示错误单词
        if self.wrong_words:
            print("\n=== 错误单词总结 ===")
            print(f"错误单词数量: {len(self.wrong_words)}")
            
            for i, word in enumerate(self.wrong_words, 1):
                print(f"\n{i}. 测试方式: {word['test_type']}")
                print(f"   单词: {word['word']}")
                print(f"   词性: {word['pos']}")
                print(f"   意思: {word['meaning']}")
                print(f"   你的答案: {word['user_answer']}")
        
        # 显示不知道的单词
        if self.unknown_words:
            print("\n=== 不知道的单词总结 ===")
            print(f"不知道的单词数量: {len(self.unknown_words)}")
            
            for i, word in enumerate(self.unknown_words, 1):
                print(f"\n{i}. 测试方式: {word['test_type']}")
                print(f"   单词: {word['word']}")
                print(f"   词性: {word['pos']}")
                print(f"   意思: {word['meaning']}")
        
        # 保存错误和不知道的单词到文件
        if self.wrong_words or self.unknown_words:
            save_choice = input("\n是否将结果保存到文件? (y/n): ").lower()
            if save_choice == 'y':
                result_filename = f"test_result_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
                
                correct_count = len(self.words) - len(self.wrong_words) - len(self.unknown_words)
                accuracy = correct_count / len(self.words) * 100 if self.words else 0
                
                result_data = {
                    "original_file": self.current_file,
                    "test_time": datetime.now().isoformat(),
                    "total_words": len(self.words),
                    "correct_count": correct_count,
                    "wrong_count": len(self.wrong_words),
                    "unknown_count": len(self.unknown_words),
                    "accuracy": f"{accuracy:.1f}%",
                    "wrong_words": self.wrong_words,
                    "unknown_words": self.unknown_words
                }
                
                try:
                    with open(result_filename, 'w', encoding='utf-8') as f:
                        json.dump(result_data, f, ensure_ascii=False, indent=2)
                    print(f"✓ 测试结果已保存到: {result_filename}")
                except Exception as e:
                    print(f"✗ 保存失败: {e}")
    
    def show_menu(self):
        """显示主菜单"""
        print("\n" + "="*40)
        print("          单词记忆管理系统")
        print("="*40)
        print("1. 录入模式 - 录入单词并保存")
        print("2. 记背模式 - 随机检测单词")
        print("3. 总结模式 - 查看错误和不知道的单词")
        print("4. 退出程序")
        print("="*40)
    
    def run(self):
        """运行主程序"""
        while True:
            self.show_menu()
            choice = input("请选择模式 (1-4): ").strip()
            
            if choice == '1':
                self.input_mode()
            elif choice == '2':
                self.recite_mode()
            elif choice == '3':
                self.summary_mode()
            elif choice == '4':
                print("感谢使用，再见！")
                break
            else:
                print("无效选择，请重新输入")

def main():
    """主函数"""
    manager = WordManager()
    manager.run()

if __name__ == "__main__":
    main()