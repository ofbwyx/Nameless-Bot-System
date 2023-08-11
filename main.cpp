#define RELEASE // 是否正式发布

#include <MiraiCP.hpp>
#include <fstream>
#include <random>
using json = nlohmann::json;
using namespace MiraiCP;
using namespace std;

const PluginConfig CPPPlugin::config{
		"com.github.ofbwyx.hi",
		"Hi",
		"3.0",
		"ofbwyx",
		"Bot System",
		"2023.8"
};

mt19937 myrand(std::chrono::system_clock::now().time_since_epoch().count()); // 随机数
ifstream f_read;
ofstream f_write; // 文件读写
string himg; // 帮助图片

json money; // 数据数
json cnt; // 计数器
json set; // 设置
json drift; // 漂流瓶

int NudgeCD; // 戳一戳冷却

// 保存文件
void SaveFile(bool d = 0) {
	// 只保存漂流瓶
	if (d) {
		f_write.open((const string)".\\data\\" + CPPPlugin::config.id + (const string)"\\drift.json");
		f_write << setw(4) << drift << endl;
		f_write.close();
	}
	// 只保存其他
	else {
		f_write.open((const string)".\\data\\" + CPPPlugin::config.id + (const string)"\\money.json");
		f_write << setw(4) << money << endl;
		f_write.close();
		f_write.open((const string)".\\data\\" + CPPPlugin::config.id + (const string)"\\cnt.json");
		f_write << setw(4) << cnt << endl;
		f_write.close();
	}
	Logger::logger.info("Save");
}

// 读取文件
void ReadFile() {
	f_read.open((const string)".\\data\\" + CPPPlugin::config.id + (const string)"\\money.json");
	money = json::parse(f_read);
	f_read.close();
	f_read.open((const string)".\\data\\" + CPPPlugin::config.id + (const string)"\\cnt.json");
	cnt = json::parse(f_read);
	f_read.close();
	f_read.open((const string)".\\data\\" + CPPPlugin::config.id + (const string)"\\drift.json");
	drift = json::parse(f_read);
	f_read.close();
	f_read.open((const string)".\\config\\" + CPPPlugin::config.id + (const string)"\\config.json");
	set = json::parse(f_read);
	f_read.close();
	Logger::logger.info("Load");
}

// 由时间戳获得日期
int getday() {
	return (time(0) + 28800) / 86400;
}

// 初始化
void DayF(string id) {
	if (money[id].is_null())money[id] = 0;
	if (cnt[id]["Bit"].is_null())cnt[id]["Bit"][0] = cnt[id]["Bit"][1] = cnt[id]["Bit"][2] = 0;
	if (cnt[id]["BitCnt"].is_null())cnt[id]["BitCnt"] = 0;
	if (cnt[id]["Draw"].is_null())cnt[id]["Draw"] = 0;
	if (cnt[id]["Sign"].is_null())cnt[id]["Sign"] = 0;
	if (cnt[id]["SignC"].is_null())cnt[id]["SignC"] = 0;
	SaveFile();
}

// python
json python(string url) {
	f_write.open("Temp.ofbwyx");
	f_write << url;
	f_write.close();
	string tmp = "python " + (string)set["Path"] + "\\python.py";
	system(tmp.c_str());
	f_read.open("Temp.ofbwyx");
	json rsp = "";
	f_read >> rsp;
	f_read.close();
	system("del .\\Temp.ofbwyx");
	return rsp;
}

bool ckAdmin(unsigned long long id) {
	for (int i = 0; i < set["Admin"].size(); i++) if ((unsigned long long)set["Admin"][i] == id)return false;
	return true;
}

namespace Command {
	// 帮助
	class help :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "help",{"帮助","?","？"},"","BS 帮助",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return; // 禁止匿名群成员使用
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			// 没有发送过则上传图片
			if (himg.size() <= 1) {
				Image a = g.uploadImg((std::string)set["Path"] + "\\help.png");
				himg = a.id;
				g.sendMessage(a);
			}
			// 发送过则直接使用旧的图
			else {
				Image a(himg);
				a.getKey(himg);
				g.sendMessage(a);
			}
		}
		help() = default;
	};

	// 签到
	class sign :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "sign",{"签到"},"","BS 签到",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id());
			DayF(id);
			if (cnt[id]["Sign"] != getday()) {
				if (getday() - cnt[id]["Sign"] > 1)cnt[id]["SignC"] = 1;
				if (cnt[id]["Sign"] == getday() - 1)cnt[id]["SignC"] = cnt[id]["SignC"] + 1;
				int rd = myrand() % ((int)set["SignMost"] - (int)set["SignLeast"] + 1) + set["SignLeast"];
				int ex = min((cnt[id]["SignC"] * 20), 200); // 连续签到
				cnt[id]["Sign"] = getday();
				money[id] = money[id] + rd + ex;
				g.sendMessage(m.nickOrNameCard() + " 签到成功！连续签到 " + to_string(cnt[id]["SignC"]) + " 天\n获得数据 " + to_string(rd) + " Bit\n连续签到获得数据 " + to_string(ex) + " Bit\n拥有数据 " + to_string(money[id]) + " Bit");
				SaveFile();
			}
			else g.sendMessage(m.nickOrNameCard() + " 你今天已经签到过了。\n拥有数据 " + to_string(money[id]) + " Bit");
		}
		sign() = default;
	};

	// 抽奖
	class draw :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "draw",{"抽奖"},"","BS 抽奖",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id());
			DayF(id);
			if (cnt[id]["Draw"] != getday()) {
				if (money[id] < set["DrawT"]) g.sendMessage(m.nickOrNameCard() + " 你没有足够的数据进行抽奖。");
				else {
					money[id] = money[id] - (int)set["DrawT"], money["Jackpot"] = money["Jackpot"] + (int)set["DrawT"];
					cnt[id]["Draw"] = getday();
					if (myrand() % set["DrawOdds"] == 0) {
						money[id] = (int)money[id] + (int)money["Jackpot"];
						g.sendMessage(m.nickOrNameCard() + " 中奖了！\n获得奖池内所有数据 " + to_string((int)money["Jackpot"]) + " Bit\n拥有数据 " + to_string((int)money[id]) + " Bit");
						money["Jackpot"] = set["Jackpot"];
					}
					else g.sendMessage(m.nickOrNameCard() + " 没有中奖捏。\n当前奖池 " + to_string((int)money["Jackpot"]) + " Bit\n剩余数据 " + to_string((int)money[id]) + " Bit");
				}
				SaveFile();
			}
			else g.sendMessage(m.nickOrNameCard() + " 你今天已经抽奖过了。");
		}
		draw() = default;
	};

	// 信息
	class info :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "info",{"信息"},"","BS 信息",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id());
			DayF(id);
			g.sendMessage(m.nickOrNameCard() + " (" + to_string(m.id()) + ")\n连续签到 " + to_string(cnt[id]["SignC"]) + " 天\n拥有数据 " + to_string((int)money[id]) + " Bit");
		}
		info() = default;
	};

	// 赌一赌
	class bit :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "bit",{},"","BS 赌一赌",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id());
			if (a.size() == 0) info().onCommand(c, b, a);
			else if (a.size() != 1)m.sendMessage("参数不匹配，请输入 #help 查看帮助。");
			else {
				int num = atoi(a[0].toMiraiCode().c_str());
				DayF(id);
				if (cnt[id]["Bit"][(int)cnt[id]["BitCnt"]] > time(0))g.sendMessage(m.nickOrNameCard() + " 你玩太多次了，过会再玩吧。\n剩余冷却时间：" + to_string(cnt[id]["Bit"][(int)cnt[id]["BitCnt"]] - time(0)) + " s");
				else if (num <= 0) g.sendMessage(m.nickOrNameCard() + " 请输入正整数！");
				else {
					if ((int)money[id] < num) g.sendMessage(m.nickOrNameCard() + " 数据不足哦");
					else {
						int rd = myrand() % (num * 2 + 1) - num;
						money[id] = money[id] + rd;
						g.sendMessage(m.nickOrNameCard() + " 数据 " + (rd >= 0 ? "+" : "-") + to_string(abs(rd)) + " Bit\n拥有数据 " + to_string((int)money[id]) + " Bit");
						cnt[id]["Bit"][(int)cnt[id]["BitCnt"]] = time(0) + set["BitCD"];
						cnt[id]["BitCnt"] = (cnt[id]["BitCnt"] + 1) % 3;
						SaveFile();
					}
				}
			}
		}
		bit() = default;
	};

	// 转账
	class pay :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "pay",{"转账"},"","BS 转账",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id());
			if (a.size() != 2) g.sendMessage("参数不匹配！");
			else {
				Logger::logger.info(a[1].toMiraiCode());
				// 解析 QQ 号
				unsigned long long toid = atoll(a[0].toMiraiCode().substr(10).c_str());
				if (!toid)toid = atoll(a[0].toMiraiCode().c_str());
				int pay = atoi(a[1].toMiraiCode().c_str());
				DayF(id);
				if (pay <= 0) g.sendMessage("参数不匹配！");
				else if (money[id] < pay) g.sendMessage("数据不足哦。");
				else {
					try {
						Member to((QQID)toid, g.id(), g.botid());
						std::string toname = to.nickOrNameCard(); // 无此人则抛出异常不转账
						DayF(to_string(toid));
						money[id] = money[id] - pay, money[to_string(toid)] = money[to_string(toid)] + pay;
						g.sendMessage("数据传输完成！\n" + m.nickOrNameCard() + " 拥有数据：" + to_string((int)money[id]) + " Bit\n" + toname + " 拥有数据：" + to_string((int)money[to_string(toid)]) + " Bit");
						SaveFile();
					}
					catch (MiraiCPException) {
						g.sendMessage("找不到群成员！");
					}
				}
			}
		}
		pay() = default;
	};

	// 排行榜 
	class rank :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "rank",{"排行榜"},"","BS 排行榜",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);

			// 获取群成员，排序
			auto gm = g.getMemberList();
			vector<pair<int, long long>>p;
			for (auto i : gm) {
				if (money[to_string(i)].is_null())continue;
				p.push_back({ -(int)money[to_string(i)],i }); // 传入负数实现从大到小排序
			}
			sort(p.begin(), p.end());

			// 构造转发消息
			vector<ForwardedNode>msg;
			int cnt = 0;
			for (auto i : p) {
				if (i.first == 0)continue;
				Member mb((QQID)atoll(to_string(i.second).c_str()), c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
				msg.push_back(ForwardedNode((QQID)g.botid(), "数据排行榜", MessageChain(PlainText(to_string(++cnt) + "：" + mb.nickOrNameCard() + "\n拥有数据：" + to_string(0 - i.first) + " Bit")), 0)); 
			}
			auto style = ForwardedMessageDisplayStrategy();
			style.title = "数据排行榜", style.brief = "数据排行榜", style.summary = "点击查看排行榜";
			style.preview.clear(), style.preview.push_back("群 " + to_string(g.groupId()) + " 数据排行榜");
			ForwardedMessage(msg,style).sendTo(&g);
		}
		rank() = default;
	};

	// 丢漂流瓶
	class dthrow :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "throw",{"丢漂流瓶","扔漂流瓶"},"","BS 丢漂流瓶",1}; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id());
			if (a.size()==0) g.sendMessage("请问你要丢什么捏，一起发出来哦。");
			else {
				drift["Cnt"] = drift["Cnt"] + 1;
				drift["Drift"][(int)drift["Cnt"]]["Text"] = a.toMiraiCode();
				drift["Drift"][(int)drift["Cnt"]]["Sender"] = m.nickOrNameCard();
				drift["Drift"][(int)drift["Cnt"]]["SenderID"] = m.id();
				drift["Drift"][(int)drift["Cnt"]]["Time"] = time(0);
				g.sendMessage(m.nickOrNameCard() + " 投掷成功！\n目前漂流瓶总数：" + to_string(drift["Cnt"]));
				SaveFile(1); // 只保存漂流瓶部分
			}
		}
		dthrow() = default;
	};

	// 捡漂流瓶 
	class dpick :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "pick",{"捡漂流瓶"},"","BS 捡漂流瓶",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id());
			int rd = myrand() % drift["Cnt"] + 1;
			json tmp = drift["Drift"][rd];
			MessageChain msg = msg.deserializationFromMiraiCode(tmp["Text"]); // 解析漂流瓶

			// 构造转发信息
			auto style = ForwardedMessageDisplayStrategy();
			style.title = "漂流瓶", style.brief = "漂流瓶", style.summary = "点击打开漂流瓶";
			style.preview.clear(),style.preview.push_back("漂流瓶 id：" + to_string(rd));
			ForwardedMessage({ ForwardedNode((QQID)tmp["SenderID"],(std::string)tmp["Sender"],msg,(int)tmp["Time"]) },style).sendTo(&g);
		}
		dpick() = default;
	};

	// 头衔
	class myhonor :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "honor",{"头衔"},"","BS 头衔",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id());
			if (a.size() == 0) g.sendMessage(m.nickOrNameCard() + " 群头衔为 " + m.specialTitle()); // 获取群头衔

			// 更改群头衔
			else if(a.size()==1){
				if (g.getOwner().id() != g.botid()) {
					g.sendMessage("权限不足！");
					return;
				}
				if ((int)money[id] < (int)set["ChangeH"])g.sendMessage("需要花费 " + to_string((int)set["ChangeH"]) + " Bit 才能更改群头衔哦。");
				else {
					m.changeSpecialTitle(a[0].toMiraiCode());
					money[id] = (int)money[id] - (int)set["ChangeH"];
					g.sendMessage(m.nickOrNameCard() + " 更改群头衔为 " + m.specialTitle()+"\n剩余数据 "+to_string((int)money[id])+" Bit");
				}
			}
			else g.sendMessage("参数不匹配！");
		}
		myhonor() = default;
	};

	// 天气
	class weather :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "weather",{"天气"},"","BS 天气",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id()),city = a[0].toMiraiCode(),msg;
			
			// 当前天气
			json rsp = python("https://api.seniverse.com/v3/weather/now.json?key=" + (std::string)set["WeatherKey"] + "&location=" + city);
			// 处理异常
			if (rsp["results"].is_null()) {
				Logger::logger.warning(rsp);
				if ((std::string)rsp["status_code"] == "AP010010" || (std::string)rsp["status_code"] == "AP010001")g.sendMessage("错误：找不到此地点。");
				else if ((std::string)rsp["status_code"] == "AP010003")g.sendMessage("错误 " + (std::string)rsp["status_code"] + "：API 密匙错误。请联系 bot 维护者。");
				else if ((std::string)rsp["status_code"] == "AP010006")g.sendMessage("错误 " + (std::string)rsp["status_code"] + "：免费套餐无权访问该地点。");
				else g.sendMessage("未知的错误 " + (std::string)rsp["status_code"] + "：请联系 bot 维护者。");
				return;
			}
			msg = city + "天气预报\n当前天气" + (std::string)(rsp["results"][0]["now"]["text"]) + "，温度 " + (std::string)(rsp["results"][0]["now"]["temperature"]) + " 摄氏度。";

			// 三天天气
			rsp = python("https://api.seniverse.com/v3/weather/daily.json?key=" + (std::string)set["WeatherKey"] + "&location=" + city);
			for (int i = 0; i < 3; i++)msg += "\n" + (std::string)rsp["results"][0]["daily"][i]["date"] + " 天气" + (std::string)rsp["results"][0]["daily"][i]["text_day"] + ((std::string)rsp["results"][0]["daily"][i]["text_day"] == (std::string)rsp["results"][0]["daily"][i]["text_night"] ? "" : "转" + (std::string)rsp["results"][0]["daily"][i]["text_night"]) + "，气温 " + (std::string)rsp["results"][0]["daily"][i]["low"] + "~" + (std::string)rsp["results"][0]["daily"][i]["high"] + " 摄氏度，相对湿度 " + (std::string)rsp["results"][0]["daily"][i]["humidity"] + "%，降水量 " + (std::string)rsp["results"][0]["daily"][i]["rainfall"] + " 毫米。";
			g.sendMessage(msg);
		}
		weather() = default;
	};

	// 管理 加载
	class load :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "load",{"加载"},"","BS 管理 加载",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id());
			if (ckAdmin(m.id())) {
				g.sendMessage("非 bot 管理无权执行该命令！");
				return;
			}
			ReadFile();
			g.sendMessage("OK");
		}
		load() = default;
	};

	// 管理 保存 
	class save :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "save",{"保存"},"","BS 管理 保存",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id());
			if (ckAdmin(m.id())) {
				g.sendMessage("非 bot 管理无权执行该命令！");
				return;
			}
			SaveFile();
			g.sendMessage("OK");
		}
		save() = default;
	};

	// 管理 发布 
	class publish :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "publish",{"发布"},"","BS 管理 发布",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id());
			if (ckAdmin(m.id())) {
				g.sendMessage("非 bot 管理无权执行该命令！");
				return;
			}
			if (set["Code"].is_null()) g.sendMessage("找不到文件！");
			else {
				std::string ver = CPPPlugin::config.version;
				g.sendFile("/main(ver " + ver + ").cpp", set["Code"]);
			}
		}
		publish() = default;
	};

	// 管理 给予
	class give :public IRawCommand {
	public:
		IRawCommand::Config config() override { return { "give",{"给予"},"","BS 管理 给予",1 }; }
		void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
			if (c.get()->toJson()["id"] == 80000000)return;
			Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
			std::string id = to_string(m.id());
			if (ckAdmin(m.id())) {
				g.sendMessage("非 bot 管理无权执行该命令！");
				return;
			}
			if (a.size() != 2) g.sendMessage("参数不匹配！");
			else {
				Logger::logger.info(a[1].toMiraiCode());
				// 解析 QQ 号
				unsigned long long toid = atoll(a[0].toMiraiCode().substr(10).c_str());
				if (!toid)toid = atoll(a[0].toMiraiCode().c_str());
				int pay = atoi(a[1].toMiraiCode().c_str());
				if (pay <= 0) g.sendMessage("参数不匹配！");
				else {
					try {
						Member to((QQID)toid, g.id(), g.botid());
						std::string toname = to.nickOrNameCard(); // 无此人则抛出异常不给予
						DayF(to_string(toid));
						money[to_string(toid)] = money[to_string(toid)] + pay;
						g.sendMessage("数据传输完成！\n" +  toname + " 拥有数据：" + to_string((int)money[to_string(toid)]) + " Bit");
						SaveFile();
					}
					catch (MiraiCPException) {
						g.sendMessage("找不到群成员！");
					}
				}
			}
		}
		give() = default;
	};

	// 模板
	/*
		class Command :public IRawCommand {
		public:
			IRawCommand::Config config() override { return { "Command",{"指令"},"","BS 指令",1 }; }
			void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
				if (c.get()->toJson()["id"] == 80000000)return;
				Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
				Member m(c.get()->toJson()["id"], c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
				std::string id = to_string(m.id());

			}
			Command() = default;
		};
	*/

	//// 测试
	//class Test :public IRawCommand {
	//public:
	//	IRawCommand::Config config() override { return { "Test",{"test"}}; }
	//	void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override {
	//		//if (c.get()->toJson()["id"] == 80000000)return;
	//		//Group g(c.get()->toJson()["groupId"], c.get()->toJson()["botId"]);
	//		//g.sendMessage("咕");
	//		Logger::logger.warning("yee");
	//	}
	//	Test() = default;
	//};

	// 正式发布时覆盖 Mirai 管理指令
#ifdef RELEASE
	class disP1 :public IRawCommand { public:IRawCommand::Config config() override { return { "disableplugin",{"disable"},"","",1 }; }void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override { ; }disP1() = default; };
	class disP2 :public IRawCommand { public:IRawCommand::Config config() override { return { "enableplugin",{"enable"},"","",1 }; }void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override { ; }disP2() = default; };
	class disP3 :public IRawCommand { public:IRawCommand::Config config() override { return { "disablepluginlist",{"dList"},"","",1 }; }void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override { ; }disP3() = default; };
	class disP4 :public IRawCommand { public:IRawCommand::Config config() override { return { "loadplugin",{"load"},"","",1 }; }void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override { ; }disP4() = default; };
	class disP5 :public IRawCommand { public:IRawCommand::Config config() override { return { "reloadplugin",{"reload"},"","",1 }; }void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override { ; }disP5() = default; };
	class disP6 :public IRawCommand { public:IRawCommand::Config config() override { return { "unloadplugin",{"unload"},"","",1 }; }void onCommand(std::shared_ptr<Contact> c, const Bot& b, const MessageChain& a) override { ; }disP6() = default; };
#endif
}

class PluginMain : public CPPPlugin {
public:
	PluginMain() : CPPPlugin() {}
	~PluginMain() override = default;

	void onEnable() override {

		ReadFile();

#ifdef RELEASE
		CommandManager::registerCommand<Command::disP1>();
		CommandManager::registerCommand<Command::disP2>();
		CommandManager::registerCommand<Command::disP3>();
		CommandManager::registerCommand<Command::disP4>();
		CommandManager::registerCommand<Command::disP5>();
		CommandManager::registerCommand<Command::disP6>();
#endif

		
		//Command::help h;
		
		CommandManager::registerCommand<Command::help>();
		CommandManager::registerCommand<Command::sign>();
		CommandManager::registerCommand<Command::draw>();
		CommandManager::registerCommand<Command::bit>();
		CommandManager::registerCommand<Command::info>();
		CommandManager::registerCommand<Command::pay>();
		CommandManager::registerCommand<Command::rank>();
		CommandManager::registerCommand<Command::dthrow>();
		CommandManager::registerCommand<Command::dpick>();
		CommandManager::registerCommand<Command::weather>();
		CommandManager::registerCommand<Command::save>();
		CommandManager::registerCommand<Command::load>();
		CommandManager::registerCommand<Command::publish>();
		CommandManager::registerCommand<Command::give>();
		CommandManager::registerCommand<Command::myhonor>();

		// 戳一戳
		Event::registerEvent<NudgeEvent>([](NudgeEvent e) {
			if (e.target.get()->id() == e.target.get()->botid() && time(0) >= NudgeCD) {
				for (int i = 0; i < set["NudgeG"].size(); i++)
					if ((i == 0 && set["NudgeG"][0] == -1) || set["NudgeG"][i] == e.subject.get()->id()) { // 找到开启了戳一戳的群
						Group g(e.subject.get()->id(), e.subject.get()->botid());
						int rd = myrand() % (set["NudgeR"].size()+1);
						if (rd == set["NudgeR"].size()) Member(e.from.get()->id(), e.subject.get()->id(), e.subject.get()->botid()).sendNudge(); // 戳回去
						else {
							MessageChain msg = msg.deserializationFromMiraiCode(set["NudgeR"][rd]);
							g.sendMessage(msg); // 发送消息
						}
						NudgeCD = time(0) + 10;
						break;
					}
			}
		});

		schedule(1, to_string(set["Fr24Bot"])); // 注册 30 min 计时器

		Event::registerEvent<TimeOutEvent>([](TimeOutEvent e) {

			Logger::logger.info("Time Out!");
			schedule(1800, e.msg); // 下一轮计时

			json rsp = python("https://www.flightradar24.com/flights/most-tracked"); // Fr24 爬取
			Logger::logger.info(rsp);

			if ((!rsp["data"][0]["squawk"].is_null() && rsp["data"][0]["squawk"] != "7600") || (int)rsp["data"][0]["clicks"] >= 10000)
				for (int i = 0; i < set["Fr24G"].size(); i++) {
					if ((int)rsp["data"][0]["clicks"] >= 10000)Group(set["Fr24G"][i], atoll(e.msg.c_str())).sendMessage("有超过一万人正在关注的航班：\n" + rsp["data"][0].dump() + "\n数据来源：https://www.flightradar24.com");
					else Group(set["Fr24G"][i], atoll(e.msg.c_str())).sendMessage("可能出现异常的航班：\n" + rsp["data"][0].dump() + "\n数据来源：https://www.flightradar24.com");
				}
		});

		Event::registerEvent<GroupMessageEvent>([](GroupMessageEvent e) {

			if (e.message.toMiraiCode() == (string)"Fr24" || e.message.toMiraiCode() == (string)"fr24") {
				json rsp = python("https://www.flightradar24.com/flights/most-tracked"); // Fr24 爬取
				e.group.sendMessage("当前关注最多的航班：\n" + rsp["data"][0].dump() + "\n数据来源：https://www.flightradar24.com");
			}

		});
	}

	void onDisable() override {
		SaveFile();
	}
};

void MiraiCP::enrollPlugin() {
	MiraiCP::enrollPlugin<PluginMain>();
}