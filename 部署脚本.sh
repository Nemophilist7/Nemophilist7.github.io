#!/bin/bash

# 个人博客部署脚本
# 使用前请确保已经正确配置了 GitHub 仓库

echo "🚀 开始部署博客..."

# 进入博客目录
cd my-new-blog

# 清理缓存
echo "🧹 清理缓存..."
hexo clean

# 生成静态文件
echo "📦 生成静态文件..."
hexo generate

# 启动本地服务器（用于测试）
echo "🌐 启动本地服务器..."
echo "📝 博客地址: http://localhost:4000"
echo "💡 按 Ctrl+C 停止服务器"

hexo server