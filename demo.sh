#!/bin/bash

# TCP服务器客户端演示脚本

echo "======================================"
echo "    TCP服务器客户端程序演示"
echo "======================================"

# 检查程序是否已编译
if [ ! -f "./servertcp" ] || [ ! -f "./clienttcp" ]; then
    echo "程序未编译，正在编译..."
    make
    echo ""
fi

echo "可用的演示选项："
echo "1. 查看程序帮助信息"
echo "2. 本地测试演示"
echo "3. 并发测试演示"
echo "4. 手动启动服务器"
echo "5. 手动启动客户端"
echo ""

read -p "请选择演示选项 (1-5): " choice

case $choice in
    1)
        echo ""
        echo "=== 服务器程序 ==="
        echo "服务器提供三种模式选择："
        ./servertcp <<< "0" 2>/dev/null || echo "服务器程序包含基础、多进程、多线程三种模式"
        echo ""
        echo "=== 客户端程序帮助 ==="
        ./clienttcp -h
        ;;
        
    2)
        echo ""
        echo "=== 本地测试演示 ==="
        echo "将启动多线程服务器进行演示..."
        echo "服务器将在后台运行，然后启动一个测试客户端"
        echo ""
        
        # 启动多线程服务器
        echo "正在启动多线程服务器..."
        (echo "3") | ./servertcp &
        SERVER_PID=$!
        
        # 等待服务器启动
        sleep 2
        
        echo "服务器已启动 (PID: $SERVER_PID)"
        echo "现在启动测试客户端..."
        echo ""
        
        # 启动测试客户端
        (
            echo "Hello, Server!"
            sleep 1
            echo "This is a test message."
            sleep 1
            echo "quit"
        ) | ./clienttcp
        
        # 关闭服务器
        echo ""
        echo "演示完成，正在关闭服务器..."
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
        echo "服务器已关闭"
        ;;
        
    3)
        echo ""
        echo "=== 并发测试演示 ==="
        echo "将启动多线程服务器，然后运行并发测试..."
        echo ""
        
        # 启动多线程服务器
        echo "正在启动多线程服务器..."
        (echo "3") | ./servertcp &
        SERVER_PID=$!
        
        # 等待服务器启动
        sleep 2
        
        echo "服务器已启动 (PID: $SERVER_PID)"
        echo "现在运行并发测试（3个客户端）..."
        echo ""
        
        # 运行并发测试
        ./test_concurrent.sh 127.0.0.1 8888 3
        
        # 关闭服务器
        echo ""
        echo "演示完成，正在关闭服务器..."
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
        echo "服务器已关闭"
        ;;
        
    4)
        echo ""
        echo "=== 手动启动服务器 ==="
        echo "正在启动服务器程序..."
        echo "请选择服务器类型（1-3）："
        ./servertcp
        ;;
        
    5)
        echo ""
        echo "=== 手动启动客户端 ==="
        echo "请输入服务器IP地址（回车使用默认 127.0.0.1）："
        read -r server_ip
        if [ -z "$server_ip" ]; then
            server_ip="127.0.0.1"
        fi
        
        echo "请输入服务器端口（回车使用默认 8888）："
        read -r server_port
        if [ -z "$server_port" ]; then
            server_port="8888"
        fi
        
        echo "正在连接到 $server_ip:$server_port..."
        ./clienttcp "$server_ip" "$server_port"
        ;;
        
    *)
        echo "无效选择"
        exit 1
        ;;
esac

echo ""
echo "演示完成！"