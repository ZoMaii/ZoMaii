#!/bin/sh

# 颜色定义
RED="\033[31;1m"
GREEN="\033[32;1m"
YELLOW="\033[33;1m"
CYAN="\033[36;1m"
BLUE="\033[34;1m"
MAGENTA="\033[35;1m"
RESET="\033[0m"

# 进度动画函数
show_progress() {
  while true; do
    printf "."
    sleep 0.3
  done
}

# 获取关键路径信息
get_path_info() {
  # 脚本绝对路径
  SCRIPT_PATH=$(cd "$(dirname "$0")" && pwd -P)/$(basename "$0")
  
  # 脚本目录
  SCRIPT_DIR=$(dirname "$SCRIPT_PATH")
  
  # 用户信息
  USER_NAME=$(whoami)
  USER_DIR=$(pwd)
}

# 打印头部信息
print_header() {
  echo -e "${CYAN}"
  echo "══════════════════════════════════════"
  echo "          编译配置信息"
  echo "══════════════════════════════════════"
  echo -e "${RESET}"
  echo -e "${YELLOW}[用户信息]${RESET}"
  echo -e " 用户名  : ${BLUE}$USER_NAME${RESET}"
  echo -e " 当前目录: ${BLUE}$USER_DIR${RESET}"
  echo
  echo -e "${YELLOW}[脚本信息]${RESET}"
  echo -e " 脚本路径: ${BLUE}$SCRIPT_PATH${RESET}"
  echo -e " 脚本目录: ${BLUE}$SCRIPT_DIR${RESET}"
  echo
}

# 编译流程
compile_process() {
  # 构建路径
  SRC_FILE="${SCRIPT_DIR}/../src/main.c"
  OUTPUT_DIR="${SCRIPT_DIR}/../output"
  OUTPUT_FILE="${OUTPUT_DIR}/app"

  # 检查源文件
  echo -e "${YELLOW}[1/4] 检查源文件...${RESET}"
  if [ ! -f "$SRC_FILE" ]; then
    echo -e "${RED}错误: 源文件不存在于 ${SRC_FILE}${RESET}"
    exit 1
  fi
  echo -e "${GREEN}✓ 找到源文件: ${SRC_FILE}${RESET}"

  # 创建输出目录
  echo -e "\n${YELLOW}[2/4] 创建输出目录...${RESET}"
  mkdir -p "$OUTPUT_DIR"
  echo -e "${GREEN}✓ 输出目录已创建: ${OUTPUT_DIR}${RESET}"

  # 开始编译
  echo -e "\n${YELLOW}[3/4] 正在编译程序...${RESET}"
  show_progress &
  PID=$!
  gcc "$SRC_FILE" -o "$OUTPUT_FILE" > /dev/null 2>&1
  kill $PID
  wait $PID 2>/dev/null
  
  # 检查编译结果
  echo -e "\n\n${YELLOW}[4/4] 验证编译结果...${RESET}"
  if [ -f "$OUTPUT_FILE" ]; then
    echo -e "${GREEN}✓ 编译成功！生成文件: ${OUTPUT_FILE}${RESET}"
  else
    echo -e "${RED}✗ 编译失败，请检查源代码${RESET}"
    exit 1
  fi
}

# 主流程
main() {
  get_path_info
  print_header
  compile_process
  
  echo -e "\n${CYAN}══════════════════════════════════════${RESET}"
  echo -e "${MAGENTA}▶ 编译流程已完成 ◀${RESET}"
  echo -e "${CYAN}══════════════════════════════════════${RESET}\n"
}

# 执行主函数
main

