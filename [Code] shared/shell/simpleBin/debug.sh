#!/bin/sh

# 颜色配置
RED="\033[31;1m"
GREEN="\033[32;1m"
YELLOW="\033[33;1m"
CYAN="\033[36;1m"
BLUE="\033[34;1m"
MAGENTA="\033[35;1m"
RESET="\033[0m"

# 路径配置（与编译脚本保持一致）
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd -P)
OUTPUT_DIR="${SCRIPT_DIR}/../output"
APP_PATH="${OUTPUT_DIR}/app"
LOG_DIR="${OUTPUT_DIR}/logs"

# 调试参数配置
DEFAULT_BREAKPOINT="main"
GDB_COMMANDS="set confirm off\nhandle SIGPIPE nostop\n"

# 动画效果
spinner() {
  local pid=$!
  local delay=0.15
  local spinstr='|/-\'
  while [ "$(ps a | awk '{print $1}' | grep $pid)" ]; do
    local temp=${spinstr#?}
    printf " [%c]  " "$spinstr"
    local spinstr=$temp${spinstr%"$temp"}
    sleep $delay
    printf "\b\b\b\b\b\b"
  done
  printf "    \b\b\b\b"
}

# 初始化环境
init_env() {
  mkdir -p "$LOG_DIR"
  current_log="${LOG_DIR}/debug_$(date +%Y%m%d_%H%M%S).log"
}

# 交互式调试菜单
show_menu() {
  clear
  echo -e "${CYAN}"
  echo "╔══════════════════════════════════╗"
  echo "║        调试控制面板             ║"
  echo "╠══════════════════════════════════╣"
  echo -e "║ ${YELLOW}1. 普通模式运行程序${CYAN}           ║"
  echo -e "║ ${YELLOW}2. GDB调试模式${CYAN}                ║" 
  echo -e "║ ${YELLOW}3. 带参数运行程序${CYAN}             ║"
  echo -e "║ ${YELLOW}4. 查看运行日志${CYAN}               ║"
  echo -e "║ ${YELLOW}5. 内存泄漏检查${CYAN}               ║"
  echo -e "║ ${YELLOW}0. 退出${CYAN}                       ║"
  echo "╚══════════════════════════════════╝"
  echo -e "${RESET}"
}

# 普通运行模式
run_normal() {
  echo -e "\n${GREEN}▶ 开始执行程序...${RESET}"
  "$APP_PATH" | tee "$current_log" 2>&1
  echo -e "\n${GREEN}✓ 程序已退出，日志保存至: ${current_log}${RESET}"
}

# GDB调试模式 
run_gdb() {
  echo -e "\n${MAGENTA}▶ 准备GDB调试环境...${RESET}"
  echo -e "${YELLOW}请输入断点位置（默认：main）: ${RESET}"
  read -r bp
  bp=${bp:-$DEFAULT_BREAKPOINT}

  echo -e "${CYAN}➜ 启动GDB调试会话（输入q退出）...${RESET}"
  gdb -ex "b $bp" -ex "run" -ex "bt" --args "$APP_PATH"
}

# 带参数运行
run_with_args() {
  echo -e "\n${YELLOW}请输入运行参数（用空格分隔）: ${RESET}"
  read -r args
  echo -e "${GREEN}▶ 带参数执行程序: $args${RESET}"
  "$APP_PATH" $args | tee "$current_log" 2>&1
}

# 显示日志
show_logs() {
  echo -e "\n${CYAN}最近5个日志文件：${RESET}"
  ls -lt "$LOG_DIR" | head -n6 | awk '{print $9}'
  echo -e "\n${YELLOW}输入要查看的日志文件名：${RESET}"
  read -r logname
  less "${LOG_DIR}/${logname}"
}

# 内存检查
check_memory() {
  echo -e "\n${RED}▶ 开始内存泄漏检查...${RESET}"
  valgrind --leak-check=full \
           --show-leak-kinds=all \
           --track-origins=yes \
           --log-file="${current_log}_memcheck" \
           "$APP_PATH" >/dev/null 2>&1 &
  spinner
  echo -e "\n${GREEN}✓ 内存检查完成，查看日志：${current_log}_memcheck${RESET}"
}

# 主流程
main() {
  # 验证可执行文件
  if [ ! -x "$APP_PATH" ]; then
    echo -e "${RED}错误：可执行文件不存在或没有执行权限${RESET}"
    echo -e "请先运行编译脚本构建程序"
    exit 1
  fi

  init_env

  while true; do
    show_menu
    echo -e "${YELLOW}请选择操作（0-5）: ${RESET}"
    read -r choice

    case $choice in
      1) run_normal ;;
      2) run_gdb ;;
      3) run_with_args ;;
      4) show_logs ;;
      5) check_memory ;;
      0) echo -e "${CYAN}退出调试器${RESET}"; exit 0 ;;
      *) echo -e "${RED}无效选项${RESET}"; sleep 1 ;;
    esac

    echo -e "\n${YELLOW}按回车返回菜单...${RESET}"
    read -r
  done
}

main
