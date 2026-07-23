# Python AI Service

Milestone 2 的本地 FastAPI AI Service。当前已完成集中配置、Dialogue Provider 接口、显式 Provider 选择、可注入的离线 Stub/Fake 边界，以及通过 OpenAI 兼容 Python SDK 调用 Kimi Chat Completions API 的非流式适配器。

## 环境要求

- Python 3.12
- Stub 模式不需要 API Key、数据库或外部服务
- Kimi 模式只从进程环境读取 `MOONSHOT_API_KEY`

## 安装依赖

仅运行 Service 时，在仓库根目录执行：

```powershell
python -m venv PythonService/.venv
PythonService/.venv/Scripts/python -m pip install -r PythonService/requirements.txt
```

进行开发和测试时，使用测试依赖声明代替最后一条安装命令：

```powershell
PythonService/.venv/Scripts/python -m pip install -r PythonService/requirements-dev.txt
```

## 配置

| 环境变量 | 默认值 | 规则 |
| --- | --- | --- |
| `ZL_DIALOGUE_PROVIDER` | `kimi` | 只允许 `kimi` 或 `stub`；不会静默回退 |
| `MOONSHOT_API_KEY` | 无 | Kimi 模式必填；不得写入仓库、请求或日志 |
| `ZL_KIMI_MODEL` | `kimi-k3` | 必须是非空字符串 |
| `ZL_KIMI_TIMEOUT_SECONDS` | `20` | 必须大于 `0` 且小于 UE 外层基线 `30` 秒 |
| `ZL_KIMI_MAX_OUTPUT_TOKENS` | `256` | 必须为正整数，硬上限为 `4096` |

环境变量必须在启动进程前设置。本项目不要求 `.env` 文件，也不要把真实 Key 写入任何仓库文件。

## 启动

依赖安装完成后，可通过一条命令从仓库根目录启动 Service：

```powershell
$env:ZL_DIALOGUE_PROVIDER = "stub"
PythonService/.venv/Scripts/python -m uvicorn app.main:app --app-dir PythonService --host 127.0.0.1 --port 8000
```

上述命令是无需网络的离线启动方式。Kimi 是默认模式；配置示例如下，其中 Key 只能使用本地进程环境中的真实值：

```powershell
$env:ZL_DIALOGUE_PROVIDER = "kimi"
$env:MOONSHOT_API_KEY = "<your-local-api-key>"
PythonService/.venv/Scripts/python -m uvicorn app.main:app --app-dir PythonService --host 127.0.0.1 --port 8000
```

Kimi 模式使用 `openai>=2.46,<3.0` 调用 `https://api.moonshot.ai/v1`，从集中配置读取模型、超时和输出上限，并显式禁用 SDK 自动重试。单次请求不会启用流式输出、工具或托管会话状态，也不会在失败时回退到 Stub。

启动后，FastAPI 文档页面位于 `http://127.0.0.1:8000/docs`，对话接口为 `POST http://127.0.0.1:8000/v1/dialogue`。请求和响应格式见 [`Docs/Protocol.md`](../Docs/Protocol.md)。

## 测试

在仓库根目录执行完整 Python 测试：

```powershell
PythonService/.venv/Scripts/python -m pytest -c PythonService/pyproject.toml
```
