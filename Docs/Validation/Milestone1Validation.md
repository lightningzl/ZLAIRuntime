# Milestone 1 验收记录

## 验收环境

- 日期：2026-07-17
- Python：3.12.13
- Unreal Engine：5.8.0
- UE Target：`ZLEditor Win64 Development`
- 编译工具链：MSVC 14.44.35228、Windows SDK 10.0.22621.0

## 可复现演示

1. 在仓库根目录安装 Python 依赖：

   ```powershell
   PythonService\.venv\Scripts\python -m pip install -r PythonService\requirements-dev.txt
   ```

2. 启动本地 Stub Service：

   ```powershell
   Set-Location PythonService
   .venv\Scripts\python -m uvicorn app.main:app --host 127.0.0.1 --port 8000
   ```

3. 启动 `ZL/ZL.uproject` 的 Game 模式，在控制台执行：

   ```text
   ZL.AI.DialogueDemo npc_guard_01 What happened here?
   ```

4. 游戏模块将输入交给 `UZLAIServiceSubsystem`，后者向 `POST /v1/dialogue` 发送版本化 JSON；Service 返回 `provider: stub` 的协议响应；UE 在屏幕和 `LogZL` 中展示 `reply`。停止 Service 后重复命令，可见带请求 ID 和错误码的失败信息，游戏不崩溃。

## 自动化与运行结果

- Python：`PythonService/.venv/Scripts/python -m pytest -c PythonService/pyproject.toml`，10 项全部通过；`pip check` 无依赖冲突。
- UE 编译：UE 5.8 `ZLEditor Win64 Development` 编译成功。
- UE 自动化：`ZLAIRuntime` 共发现并通过 6 项测试，测试框架退出码为 0。
- 连续请求：在单次 Game 会话中依次执行 `stability_01` 至 `stability_10`；UE 记录 10 条请求、10 条回复、0 条失败，Service 记录 10 个 `POST /v1/dialogue` 200；游戏按退出码 0 结束，Service 可正常停止，未观察到崩溃、遗留进程或明显资源泄漏。

## 验收项

| 验收项 | 结果 | 证据 |
| --- | --- | --- |
| 单命令启动 Python Service | 通过 | Uvicorn 启动后 `/docs` 返回 200 |
| 合法 JSON 返回协议化 200 | 通过 | Python 测试与真实 HTTP 联调 |
| 请求与响应 ID 一致 | 通过 | Python 测试、UE 协议测试与联调 |
| `provider` 为 `stub` 且无 LLM Key | 通过 | Python 测试在无 API Key、禁外网条件下通过 |
| 非法字段返回结构化错误 | 通过 | Python 10 项测试覆盖 400、422 和 500 |
| UE 地址可配置并能发送请求 | 通过 | 配置测试与真实 Service 联调 |
| UE 可见展示 `reply` | 通过 | Game 模式屏幕与日志演示 |
| Service/超时/非 2xx 失败不崩溃 | 通过 | UE 失败分类测试与 Game 失败演示 |
| 至少 10 次连续请求稳定 | 通过 | 单次会话 10 请求、10 回复、10 个 HTTP 200 |
| 无超范围运行时依赖 | 通过 | 依赖声明、Python 应用、UE Runtime 源码与 Build.cs 审计 |

## 运行时依赖审计

- Python 运行依赖仅为 FastAPI 和 Uvicorn；`httpx2` 与 pytest 仅属于开发测试依赖。
- `ZLAIRuntime` 插件依赖 Core、CoreUObject、DeveloperSettings、Engine、HTTP 和 Json。
- 对 Python 应用、Python 依赖声明、UE Runtime 源码和 Build.cs 搜索数据库、向量库、真实 LLM SDK 及 Tool Call/Tool Use 相关依赖，无运行时命中。
- 当前实现不访问数据库、不要求外部网络或 API Key，也不生成、解析或执行 Tool Call。
