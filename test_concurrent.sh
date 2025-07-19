#!/bin/bash

# TCP服务器并发测试脚本

SERVER_IP=${1:-"127.0.0.1"}
SERVER_PORT=${2:-"8888"}
NUM_CLIENTS=${3:-5}

echo "=== TCP服务器并发测试 ==="
echo "服务器地址: $SERVER_IP:$SERVER_PORT"
echo "并发客户端数量: $NUM_CLIENTS"
echo "================================"

# 检查服务器是否可达
echo "正在检查服务器连通性..."
if ! nc -z "$SERVER_IP" "$SERVER_PORT" 2>/dev/null; then
    echo "错误: 无法连接到服务器 $SERVER_IP:$SERVER_PORT"
    echo "请确保服务器正在运行"
    exit 1
fi

echo "服务器连通性检查通过"

# 创建临时目录存储测试结果
TEST_DIR="/tmp/tcp_test_$$"
mkdir -p "$TEST_DIR"

echo "开始并发测试..."

# 启动多个客户端进程
for i in $(seq 1 $NUM_CLIENTS); do
    {
        echo "=== 客户端 $i 开始测试 ===" > "$TEST_DIR/client_$i.log"
        
        # 发送测试消息
        (
            echo "你好，我是客户端 $i"
            sleep 1
            echo "这是客户端 $i 的第二条消息"
            sleep 1
            echo "客户端 $i 测试完成"
            sleep 1
            echo "quit"
        ) | ./clienttcp "$SERVER_IP" "$SERVER_PORT" >> "$TEST_DIR/client_$i.log" 2>&1
        
        echo "=== 客户端 $i 测试完成 ===" >> "$TEST_DIR/client_$i.log"
    } &
    
    echo "客户端 $i 已启动 (PID: $!)"
    
    # 稍微延迟避免同时连接
    sleep 0.2
done

echo "等待所有客户端完成..."
wait

echo "测试完成！查看结果："
echo "================================"

# 显示测试结果
for i in $(seq 1 $NUM_CLIENTS); do
    echo "--- 客户端 $i 的输出 ---"
    if [ -f "$TEST_DIR/client_$i.log" ]; then
        cat "$TEST_DIR/client_$i.log"
    else
        echo "日志文件不存在"
    fi
    echo ""
done

# 清理临时文件
echo "清理临时文件..."
rm -rf "$TEST_DIR"

echo "测试脚本执行完毕"