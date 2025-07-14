# GitHub Pages 部署指南

## 准备工作

### 1. 创建 GitHub 仓库
1. 登录 GitHub 账号
2. 创建一个新的仓库，命名为 `你的用户名.github.io`
   - 例如：如果你的用户名是 `john123`，则仓库名应该是 `john123.github.io`
3. 仓库必须设置为 **Public**（公开）
4. 勾选 "Add a README file" 选项

### 2. 配置本地 Git
```bash
# 配置用户信息（如果还没有配置）
git config --global user.name "你的用户名"
git config --global user.email "your.email@example.com"

# 进入博客目录
cd my-new-blog

# 初始化 Git 仓库
git init

# 添加远程仓库
git remote add origin https://github.com/你的用户名/你的用户名.github.io.git
```

## 部署方法

### 方法一：使用 Hexo 部署插件

#### 1. 安装部署插件
```bash
npm install hexo-deployer-git --save
```

#### 2. 配置 _config.yml
编辑 `_config.yml` 文件，在最后添加：
```yaml
# Deployment
deploy:
  type: git
  repo: https://github.com/你的用户名/你的用户名.github.io.git
  branch: main
```

#### 3. 部署到 GitHub Pages
```bash
# 清理缓存
hexo clean

# 生成并部署
hexo deploy -g
```

### 方法二：手动部署

#### 1. 生成静态文件
```bash
hexo generate
```

#### 2. 复制文件到 GitHub 仓库
```bash
# 克隆你的 GitHub Pages 仓库到另一个目录
git clone https://github.com/你的用户名/你的用户名.github.io.git

# 复制生成的文件
cp -r public/* 你的用户名.github.io/

# 提交并推送
cd 你的用户名.github.io
git add .
git commit -m "Deploy blog"
git push origin main
```

### 方法三：使用 GitHub Actions（推荐）

#### 1. 创建源码仓库
创建一个名为 `blog-source` 的仓库来存储源代码

#### 2. 配置 GitHub Actions
在博客项目根目录创建 `.github/workflows/deploy.yml`：
```yaml
name: Deploy Blog

on:
  push:
    branches: [ main ]

jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
    - name: Checkout
      uses: actions/checkout@v3
      with:
        submodules: false
        
    - name: Setup Node.js
      uses: actions/setup-node@v3
      with:
        node-version: '18'
        
    - name: Cache dependencies
      uses: actions/cache@v3
      with:
        path: ~/.npm
        key: ${{ runner.os }}-node-${{ hashFiles('**/package-lock.json') }}
        
    - name: Install dependencies
      run: npm ci
      
    - name: Generate static files
      run: npm run build
      
    - name: Deploy to GitHub Pages
      uses: peaceiris/actions-gh-pages@v3
      with:
        github_token: ${{ secrets.GITHUB_TOKEN }}
        publish_dir: ./public
        publish_branch: main
```

#### 3. 配置 package.json
添加构建脚本：
```json
{
  "scripts": {
    "build": "hexo generate",
    "clean": "hexo clean",
    "deploy": "hexo deploy",
    "server": "hexo server"
  }
}
```

## 自定义域名（可选）

### 1. 购买域名
在域名服务商购买域名，如：
- 阿里云域名服务
- 腾讯云域名服务
- GoDaddy
- Namecheap

### 2. 配置 DNS
在域名服务商的控制面板中添加以下 DNS 记录：
```
类型: CNAME
主机记录: www
记录值: 你的用户名.github.io

类型: A
主机记录: @
记录值: 185.199.108.153
记录值: 185.199.109.153
记录值: 185.199.110.153
记录值: 185.199.111.153
```

### 3. 配置 GitHub Pages
1. 在博客源码的 `source` 目录下创建 `CNAME` 文件
2. 文件内容为你的域名，例如：`blog.yourdomain.com`
3. 重新部署博客

### 4. 在 GitHub 仓库设置中配置
1. 进入 GitHub 仓库的 Settings
2. 找到 Pages 设置
3. 在 Custom domain 中输入你的域名
4. 勾选 "Enforce HTTPS"

## 常见问题解决

### 1. 部署失败
**问题**：`fatal: repository 'https://github.com/...' not found`
**解决**：
- 检查仓库名是否正确
- 确认仓库是 Public 的
- 检查用户名和密码是否正确

### 2. 页面显示 404
**问题**：访问 `你的用户名.github.io` 显示 404
**解决**：
- 确认 `index.html` 文件在仓库根目录
- 检查 GitHub Pages 是否已启用
- 等待 5-10 分钟让 GitHub Pages 生效

### 3. 样式丢失
**问题**：网站显示但没有样式
**解决**：
- 检查 `_config.yml` 中的 `url` 和 `root` 配置
- 确认 CSS 文件路径正确

### 4. 中文文章链接问题
**问题**：中文文章链接无法访问
**解决**：
在 `_config.yml` 中修改：
```yaml
permalink: :year/:month/:day/:id/
```

## 维护建议

### 1. 定期备份
- 将源代码备份到私有仓库
- 定期导出重要文章

### 2. 监控性能
- 使用 Google Analytics 监控访问量
- 优化图片大小和加载速度

### 3. 更新维护
- 定期更新 Hexo 版本
- 更新主题和插件

### 4. 安全措施
- 不要在源代码中暴露敏感信息
- 使用 GitHub Token 而不是密码进行部署

## 下一步

完成部署后，你可以：
1. 📝 开始写作你的第一篇博客
2. 🎨 选择和配置博客主题
3. 🔧 添加评论系统和数据统计
4. 🚀 优化 SEO 设置
5. 📱 确保移动端适配

祝你博客搭建成功！🎉