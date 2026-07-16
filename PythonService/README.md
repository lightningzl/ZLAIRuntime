# Python AI Service

Milestone 1 使用的本地 FastAPI Stub Service。当前骨架只负责应用启动和模块分层；`POST /v1/dialogue` 将在 `M1-02` 中实现。

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

启动后，FastAPI 文档页面位于 `http://127.0.0.1:8000/docs`。当前尚未注册对话接口。
