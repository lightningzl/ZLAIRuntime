# Python Modules

## 当前状态

Python 链路 `M1-01` 至 `M1-03` 已完成：AI Service 可启动，并已实现对话 Route、严格协议 Schema、确定性 Stub Service、统一错误映射和自动化回归测试。

## 计划目录

```text
PythonService/
  .python-version
  pyproject.toml
  app/
    __init__.py
    main.py
    api/
      __init__.py
      dialogue.py
    schemas/
      __init__.py
      dialogue.py
    services/
      __init__.py
      dialogue_service.py
  tests/
    test_dialogue_api.py
  requirements-dev.txt
  requirements.txt
  README.md
```

## 模块职责

| 模块 | 状态 | 职责 |
| --- | --- | --- |
| `app.main` | 已实现（M1-02） | 通过 `create_app()` 创建 FastAPI App、注册 Dialogue Router，并统一映射业务校验、Schema 校验和未预期错误 |
| `app.api.dialogue` | 已实现（M1-02） | 提供 `POST /v1/dialogue` 的 HTTP 输入输出适配，并将业务处理委托给 Service |
| `app.schemas.dialogue` | 已实现（M1-02） | 定义严格的请求、成功响应和统一错误响应 Schema |
| `app.services.dialogue_service` | 已实现（M1-02） | 校验空输入并返回确定性 Stub 回复，不依赖 FastAPI HTTP 类型 |
| `tests.test_dialogue_api` | 已实现（M1-03） | 验证成功字段透传、确定性回复、业务与 Schema 错误、内部错误脱敏，以及无 API Key/外部网络运行 |

## 依赖方向

```text
app.main -> app.api.dialogue -> app.services.dialogue_service
                         \----> app.schemas.dialogue
```

- Route 可以依赖 Schema 和 Service。
- Service 不依赖 FastAPI Request/Response，也不直接构造 HTTP 状态码。
- Schema 不依赖 Route 或 Service。
- 当前阶段任何模块都不得依赖 LLM SDK、SQLite、Chroma、FAISS 或 Tool 框架。

## 运行与配置边界

- Service 必须能通过一条明确命令本地启动。
- 当前无需 API Key、数据库连接或外部网络。
- Host/Port 可由启动参数或环境配置提供，但不引入生产级配置系统。
- `provider` 固定返回 `stub`。
- 错误格式和状态码严格遵守 [Protocol.md](./Protocol.md)。

## 更新规则

新增、删除或改变 Python 模块职责时，更新目录、职责表和依赖方向；架构变化同时记录到 [DecisionLog.md](./DecisionLog.md)。
