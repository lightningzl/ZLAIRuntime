# Python Modules

## 当前状态

`M1-01` 已建立可启动的 Python AI Service 骨架。FastAPI App 与 Route、Schema、Service 目录边界已经就位；对话协议、Stub 回复和错误映射留待 `M1-02` 实现。

## 计划目录

```text
PythonService/
  .python-version
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
  requirements.txt
  README.md
```

`tests/` 及其测试文件将在 `M1-03` 创建。

## 模块职责

| 模块 | 状态 | 职责 |
| --- | --- | --- |
| `app.main` | 已实现（M1-01） | 通过 `create_app()` 创建 FastAPI App 并注册 Dialogue Router；统一异常处理留待 `M1-02` |
| `app.api.dialogue` | 骨架（M1-01） | 提供 `/v1` Router；`POST /v1/dialogue` 输入输出适配留待 `M1-02` |
| `app.schemas.dialogue` | 占位（M1-01） | 请求、成功响应和错误响应 Schema 留待 `M1-02` |
| `app.services.dialogue_service` | 占位（M1-01） | 确定性 Stub 回复留待 `M1-02`，且不依赖 HTTP 类型 |
| `tests.test_dialogue_api` | 计划 | 验证成功请求和协议规定的错误路径 |

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
