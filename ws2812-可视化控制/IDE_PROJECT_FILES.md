# IDE 项目文件说明

## 已恢复的文件

以下文件已从项目备份中恢复，用于在 WCH Studio 中打开和开发项目：

| 文件 | 用途 | 必需 |
|------|------|------|
| `第二讲.wvproj` | WCH Studio 项目配置文件 | ✅ 必需 |
| `.project` | Eclipse/CDT 项目配置 | ✅ 必需 |
| `.cproject` | Eclipse/CDT C 项目配置 | ✅ 必需 |

## 如何打开项目

### Windows

1. **打开 WCH Studio**
2. **File** → **Open Projects from File System**
3. 选择本目录（`C:\Users\weijiahong\Desktop\1位WS2812\第二讲——测试`）
4. 点击 **Finish**

或者直接：
1. **File** → **Open File**
2. 选择 `第二讲.wvproj`

### Linux/Mac

- 使用 WCH Studio 的对应版本
- 或在命令行使用 riscv-none-embed-gcc 编译

## 编译和烧录步骤

1. **编译**
   - 右键点击项目 → **Build Project**
   - 或 **Project** → **Build Project**
   - 或按 `Ctrl+B`

2. **烧录**
   - **Run** → **Download and Run**
   - 或按 `Ctrl+F11`
   - 确保 WCH Link 已连接

## .gitignore 更新

由于 IDE 文件对于项目打开和烧录是必需的，以下文件已从 `.gitignore` 中移除：

```
# 现在 .gitignore 会保留这些文件
# .project
# .cproject  
# *.wvproj
# *.launch
```

只忽略编译输出和临时文件：
```
obj/           # 编译输出
Debug/         # 调试文件
Release/       # 发布文件
.settings/     # IDE 设置缓存
.mrs/          # MRS 备份
```

## 注意事项

- 这些文件是基于 CH32V307 项目模板改编
- 如果在 WCH Studio 中打开后提示配置错误，请：
  1. 右键项目 → **Properties**
  2. 检查 MCU 是否为 CH32V30X
  3. 检查编译路径是否正确

- 如遇到编译错误，确保：
  1. Peripheral 库文件完整
  2. User/main.c 存在
  3. HardWare/ws2812.c/h 存在

## 常见问题

**Q: 能否删除这些 IDE 文件？**  
A: 不建议。如果删除，需要在 WCH Studio 中重新创建项目配置，比较麻烦。

**Q: 这些文件能否上传到 GitHub？**  
A: 可以。这些是项目必需文件，应该版本控制。

**Q: 如何在其他电脑上打开项目？**  
A: 直接复制整个目录，然后在 WCH Studio 中 Open，会自动识别这些配置文件。

---

**恢复日期**: 2026-04-29  
**恢复原因**: 重新创建 IDE 配置文件（删除错误导致的恢复）
