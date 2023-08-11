# Nameless-Bot-System

Nameless Bot System (by ofbwyx)

## 使用方法

0. 本项目运行在 [Mirai](https://github.com/mamoe/mirai) 框架下，并且依赖 [MiraiCP](https://github.com/Nambers/MiraiCP)。请在使用本项目前部署好二者。确认 python 已安装并添加到 path，且安装了 requests 库（命令行使用 `pip install requests` 进行安装）。
1. 在本项目 Releases 中下载最新版本的文件压缩包，按照部署 MiraiCP 插件的方式处理 Hi.dll 文件，注意所需 MiraiCP 的版本。**目前最新版本依赖 2.15.0 版本的 MiraiCP 运行。**
2. 将压缩包内的其余两个文件夹复制进 mcl 根目录并合并。注意检查 mcl 根目录下的 `data` 和 `config` 文件夹中是否分别有一个名为 `com.github.ofbwyx.hi` 的文件夹且其中都有一些文件。
3. 打开 `.\config\com.github.ofbwyx.hi\config.json`，修改 `Admin` 数组内容为 bot 管理者的 qq 号（可以有多个，用逗号隔开），修改 `NudgeG` 数组内容为开启戳一戳功能的群号（可以有多个，用逗号隔开），若填写 `-1`（默认如此）则所有群都开启戳一戳。
4. 若有使用 FlightRadar24 预警功能的需求，修改 `Fr24Bot` 参数为 bot 的 qq 号（用 mcl 登录哪个号就填哪个号），修改 `Fr24G` 数组内容为需要开启播报的群号（可以有多个，用逗号隔开）。无此需求则可以跳过本步。
5. 启动！

## 开发方法

咕，以后再补。
