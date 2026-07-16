# Python AI Service

Milestone 1 使用的本地 FastAPI Stub Service。当前实现提供版本化对话接口、严格请求校验、确定性 Stub 回复和统一错误结构。

## 环境要求

- Python 3.12
- 不需要 API Key、数据库或外部服务

## 安装依赖

在仓库根目录执行：

```powershell
python -m venv PythonService/.venv
PythonService/.venv/Scripts/python -m pip install -r PythonService/requirements.txt
```

## 启动

依赖安装完成后，可通过一条命令从仓库根目录启动 Service：

```powershell
PythonService/.venv/Scripts/python -m uvicorn app.main:app --app-dir PythonService --host 127.0.0.1 --port 8000
```

启动后，FastAPI 文档页面位于 `http://127.0.0.1:8000/docs`，对话接口为 `POST http://127.0.0.1:8000/v1/dialogue`。请求和响应格式见 [`Docs/Protocol.md`](../Docs/Protocol.md)。
