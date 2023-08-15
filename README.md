# Nameless-Bot-System

Nameless Bot System (by ofbwyx)

## 使用方法

0. 本项目运行在 [Mirai](https://github.com/mamoe/mirai) 框架下，并且依赖 [MiraiCP](https://github.com/Nambers/MiraiCP)。请在使用本项目前部署好二者。确认 python 已安装并添加到 path，且安装了 requests 库（命令行使用 `pip install requests` 进行安装）。
1. 在本项目 Releases 中下载最新版本的文件压缩包，按照部署 MiraiCP 插件的方式处理 Hi.dll 文件，注意所需 MiraiCP 的版本。**目前最新版本依赖 2.15.0 版本的 MiraiCP 运行。**
2. 将压缩包内的其余两个文件夹复制进 mcl 根目录并合并。注意检查 mcl 根目录下的 `data` 和 `config` 文件夹中是否分别有一个名为 `com.github.ofbwyx.hi` 的文件夹且其中都有一些文件。
3. 打开 `.\config\com.github.ofbwyx.hi\config.json`，修改 `Admin` 数组内容为 bot 管理者的 qq 号（可以有多个，用逗号隔开），修改 `NudgeG` 数组内容为开启戳一戳功能的群号（可以有多个，用逗号隔开），若填写 `-1`（默认如此）则所有群都开启戳一戳。
4. 若有使用 FlightRadar24 预警功能的需求，修改 `Fr24Bot` 参数为 bot 的 qq 号（用 mcl 登录哪个号就填哪个号），修改 `Fr24G` 数组内容为需要开启播报的群号（可以有多个，用逗号隔开）。无此需求则可以跳过本步。
6. 启动！
7. 在 mcl 控制台中输入命令 `perm add m群号.*` 以允许这个群的成员使用本插件。fr24 模块目前暂不支持权限设置。

## 开发方法

0. 安装 Visual Studio 和 Cmake（当然你也可以使用其他方法进行编辑和编译）。安装好 mcl 并保证能正常运行本项目 Releases 中的插件。
1. 按照 [MiraiCP 开发文档](https://github.com/Nambers/MiraiCP/blob/main/doc/startup.md) 下载好 MiraiCP-template 并配置。
2. 使用本项目 Code 中的 `main.cpp` 替换 MiraiCP-template 中的 `.\src\main.cpp`。
3. 在 `.\build` 文件夹（如果没有就新建一个）中打开 cmd，输入 `cmake ..`。
4. 打开 `.\build` 文件夹中的 `*.sln` 项目文件（名字取决于你在 `CMakeLists.txt` 中填写的，默认是 `MiraiCPPlugin`）。
5. 进行编辑，完成后点击生成解决方案（默认 F7），将生成 `.\build\Debug\*.dll` 或 `.\build\Release\*.dll`（取决于编译时采用的配置）。
6. 在 mcl 目录下的 `.\data\tech.eritquearcus.miraicp\miraicp.json` 中修改 `"pluginConfig"` 项的 `"path"` 为编译出的 dll 文件的路径。
7. 启动 mcl！
8. 若在 mcl 运行状态下重新编译了插件，可在 mcl 控制台使用 `reload com.github.ofbwyx.hi` 命令重新加载插件（具体以代码中的插件名为准）。

## 开发时的重要事项

代码第一行的 `#define RELEASE` 在开发时请注释掉，因为这会使本插件覆盖 MiraiCP 的所有命令（包括 `reload` 等）。
