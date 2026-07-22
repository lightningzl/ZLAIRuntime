# Architecture

## 总体结构

```text
Player / UI
    |
    v
UE5 Runtime
  - ZL Gameplay/UI
  - ZLAIRuntime Plugin
    - AI Service Client
    - Protocol Types
    |
    | HTTP + versioned JSON
    v
Python AI Service
  - FastAPI Route
  - Dialogue Service
  - Dialogue Provider Interface
      |- OpenAI Provider
      `- Stub Provider
```

## 模块边界

### UE5 Runtime

`ZL` Gameplay/UI 只通过 `ZLAIRuntime` 插件公开接口提交请求和消费结果。

- 负责请求发起、端到端 HTTP 管理、协议解析和结果展示。
- 不负责 Prompt、模型 SDK、Provider 选择、密钥、Memory 或 Gameplay Tool 执行。
- 插件不得依赖 `ZL` 游戏模块、具体 UI 或 NPC Actor。

### Python AI Service

Python Service 负责 AI 推理编排，不直接访问或修改 UE 世界。

- Route 负责 HTTP 输入输出适配。
- Dialogue Service 负责业务编排。
- Provider 接口隔离供应商 SDK 与上游数据格式。
- Provider 实现负责生成请求和供应商异常分类。
- Service 不负责动画、移动、任务、战斗等 Gameplay 行为。

### Protocol

[Protocol.md](./Protocol.md) 是 UE 与 Python 的唯一共享边界。两端不得依赖对方的内部类型、SDK 或目录结构。

## 依赖方向

```text
ZL Gameplay/UI -> ZLAIRuntime Plugin -> HTTP/JSON Protocol

FastAPI Route -> Dialogue Service -> Dialogue Provider Interface -> Provider SDK
```

- Gameplay 层不处理 HTTP、JSON 或 Provider 细节。
- Route 不直接调用具体 Provider SDK。
- Dialogue Service 只依赖 Provider 接口，不依赖 FastAPI HTTP 类型或具体 SDK。
- Provider 实现不得构造 UE/Python 协议响应。

## 后续扩展边界

- **Context Builder**：组装人格、世界状态和有限对话上下文。
- **Memory Service**：负责存储与检索，不由 Route 或 UE Client 直接操作数据库。
- **Tool Planner**：生成结构化 Tool Call。
- **UE Tool Registry/Executor**：白名单校验、参数校验和 Gameplay 执行；最终执行权始终在 UE。

当前允许范围由 [CurrentMilestone.md](./CurrentMilestone.md) 定义；具体模块和类型见 [PythonModules.md](./PythonModules.md) 与 [UEClasses.md](./UEClasses.md)。
