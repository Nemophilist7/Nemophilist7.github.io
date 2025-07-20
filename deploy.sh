#!/bin/bash

# TCP服务器客户端一键部署脚本

VERSION="1.1"
PROJECT_NAME="TCP服务器客户端"

echo "🌐 $PROJECT_NAME 一键部署脚本 v$VERSION"
echo "=========================================="

# 检查系统类型
check_system() {
    if command -v apt-get >/dev/null 2>&1; then
        SYSTEM="debian"
        INSTALL_CMD="sudo apt-get install -y"
    elif command -v yum >/dev/null 2>&1; then
        SYSTEM="redhat"
        INSTALL_CMD="sudo yum install -y"
    elif command -v dnf >/dev/null 2>&1; then
        SYSTEM="fedora"
        INSTALL_CMD="sudo dnf install -y"
    else
        SYSTEM="unknown"
    fi
    
    echo "✅ 检测到系统类型: $SYSTEM"
}

# 安装依赖
install_dependencies() {
    echo "🔧 检查并安装依赖..."
    
    # 检查gcc
    if ! command -v gcc >/dev/null 2>&1; then
        echo "⚠️  未找到gcc编译器，正在安装..."
        case $SYSTEM in
            "debian")
                $INSTALL_CMD build-essential
                ;;
            "redhat"|"fedora")
                $INSTALL_CMD gcc make
                ;;
            *)
                echo "❌ 无法自动安装gcc，请手动安装"
                exit 1
                ;;
        esac
    else
        echo "✅ gcc编译器已安装"
    fi
    
    # 检查make
    if ! command -v make >/dev/null 2>&1; then
        echo "⚠️  未找到make工具，正在安装..."
        case $SYSTEM in
            "debian")
                $INSTALL_CMD make
                ;;
            "redhat"|"fedora")
                $INSTALL_CMD make
                ;;
            *)
                echo "❌ 无法自动安装make，请手动安装"
                exit 1
                ;;
        esac
    else
        echo "✅ make工具已安装"
    fi
    
    # 检查网络工具
    if ! command -v netstat >/dev/null 2>&1; then
        echo "⚠️  未找到网络工具，正在安装..."
        case $SYSTEM in
            "debian")
                $INSTALL_CMD net-tools
                ;;
            "redhat"|"fedora")
                $INSTALL_CMD net-tools
                ;;
        esac
    fi
}

# 编译程序
compile_programs() {
    echo "🔨 编译程序..."
    
    if [ -f "Makefile" ]; then
        make clean
        if make; then
            echo "✅ 程序编译成功"
        else
            echo "❌ 编译失败"
            exit 1
        fi
    else
        echo "⚠️  未找到Makefile，使用手动编译..."
        
        if [ -f "servertcp.c" ]; then
            gcc -Wall -Wextra -std=c99 -pthread -o servertcp servertcp.c
            echo "✅ 服务器程序编译完成"
        else
            echo "❌ 未找到servertcp.c文件"
        fi
        
        if [ -f "clienttcp.c" ]; then
            gcc -Wall -Wextra -std=c99 -pthread -o clienttcp clienttcp.c
            echo "✅ 客户端程序编译完成"
        else
            echo "❌ 未找到clienttcp.c文件"
        fi
    fi
}

# 检查网络环境
check_network() {
    echo "🔍 检查网络环境..."
    
    # 获取IP地址
    echo "本机IP地址:"
    if command -v hostname >/dev/null 2>&1; then
        hostname -I 2>/dev/null | awk '{print "  " $1}'
    fi
    
    if command -v ip >/dev/null 2>&1; then
        ip addr show | grep 'inet ' | grep -v '127.0.0.1' | awk '{print "  " $2}' | head -3
    fi
    
    # 检查端口8888
    if command -v netstat >/dev/null 2>&1; then
        if netstat -tlnp 2>/dev/null | grep -q ':8888 '; then
            echo "⚠️  端口8888已被占用"
            netstat -tlnp | grep ':8888 '
        else
            echo "✅ 端口8888可用"
        fi
    fi
}

# 配置防火墙
configure_firewall() {
    echo "🛡️  配置防火墙..."
    
    case $SYSTEM in
        "debian")
            if command -v ufw >/dev/null 2>&1; then
                echo "检测到UFW防火墙"
                read -p "是否配置UFW允许8888端口？(y/N): " answer
                if [ "$answer" = "y" ] || [ "$answer" = "Y" ]; then
                    sudo ufw allow 8888
                    sudo ufw status
                fi
            fi
            ;;
        "redhat"|"fedora")
            if command -v firewall-cmd >/dev/null 2>&1; then
                echo "检测到firewalld防火墙"
                read -p "是否配置firewalld允许8888端口？(y/N): " answer
                if [ "$answer" = "y" ] || [ "$answer" = "Y" ]; then
                    sudo firewall-cmd --permanent --add-port=8888/tcp
                    sudo firewall-cmd --reload
                    sudo firewall-cmd --list-ports
                fi
            fi
            ;;
    esac
}

# 创建启动脚本
create_scripts() {
    echo "📝 创建便捷启动脚本..."
    
    # 服务器启动脚本
    cat > start_server.sh << 'EOF'
#!/bin/bash
echo "🚀 启动TCP服务器"
echo "==============="

if [ ! -f "./servertcp" ]; then
    echo "❌ 未找到servertcp程序，请先运行 ./deploy.sh"
    exit 1
fi

echo "本机IP地址:"
hostname -I 2>/dev/null | awk '{print "  " $1}' || echo "  无法获取IP地址"

echo ""
echo "启动服务器程序..."
./servertcp
EOF

    # 客户端启动脚本
    cat > start_client.sh << 'EOF'
#!/bin/bash
echo "📱 启动TCP客户端"
echo "==============="

if [ ! -f "./clienttcp" ]; then
    echo "❌ 未找到clienttcp程序，请先运行 ./deploy.sh"
    exit 1
fi

if [ $# -eq 0 ]; then
    echo "使用交互式模式连接服务器"
    ./clienttcp -i
else
    echo "连接到服务器: $1"
    ./clienttcp "$@"
fi
EOF

    chmod +x start_server.sh start_client.sh
    echo "✅ 启动脚本创建完成"
}

# 显示使用说明
show_usage() {
    echo ""
    echo "🎉 部署完成！"
    echo "=============="
    echo ""
    echo "📋 可用文件:"
    echo "  servertcp        - 服务器程序"
    echo "  clienttcp        - 客户端程序"
    echo "  start_server.sh  - 服务器启动脚本"
    echo "  start_client.sh  - 客户端启动脚本"
    echo ""
    echo "🚀 快速开始:"
    echo "  启动服务器: ./start_server.sh"
    echo "  启动客户端: ./start_client.sh [服务器IP]"
    echo ""
    echo "💡 示例:"
    echo "  在服务器机器上: ./start_server.sh"
    echo "  在客户端机器上: ./start_client.sh 192.168.1.100"
    echo ""
    echo "📖 更多信息请查看:"
    echo "  README.md - 详细使用说明"
    echo "  跨机器部署指南.md - 跨机器部署指南"
}

# 主函数
main() {
    echo "开始部署..."
    
    check_system
    install_dependencies
    compile_programs
    check_network
    configure_firewall
    create_scripts
    show_usage
    
    echo ""
    echo "✅ 部署完成！祝您使用愉快！"
}

# 运行主函数
main