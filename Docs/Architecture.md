# Architecture

## 总体结构

```text
Player / UI
    |
    v
UE5 Runtime
  - ZL Gameplay/UI
    - Context Snapshot Source
  - ZLAIRuntime Plugin
    - AI Service Client
    - Protocol Types
    |
    | HTTP + versioned JSON
    v
Python AI Service
  - FastAPI Route
  - Dialogue Service
  - Context Builder
  - Dialogue Provider Interface
      |- Kimi Provider
      `- Stub Provider
```

## 模块边界

### UE5 Runtime

`ZL` Gameplay/UI 只通过 `ZLAIRuntime` 插件公开接口提交请求和消费结果。

- Gameplay/UI 负责从自身已知状态构造当前请求的 NPC 人格、世界状态和有限历史快照。
- 插件负责请求校验与发起、端到端 HTTP 管理、协议解析和结果交付。
- 插件不从 Actor、World、GameState、UI 或内容资产自动抓取上下文。
- UE 不负责 Prompt、模型 SDK、Provider 选择、密钥、Memory 或 Python 生成编排。
- 插件不得依赖 `ZL` 游戏模块、具体 UI 或 NPC Actor。
- Gameplay Tool 的最终校验和执行权仍属于 UE，但 Tool Use 不在当前里程碑范围。

### Python AI Service

Python Service 负责 AI 推理编排，不直接访问或修改 UE 世界。

- Route 负责 HTTP 输入输出适配。
- Dialogue Service 负责业务编排。
- Context Builder 负责将固定系统约束、NPC 人格、世界状态、有限历史和当前输入组装为供应商无关的生成上下文。
- Provider 接口隔离供应商 SDK 与上游数据格式。
- Provider 实现负责把内部生成上下文映射为供应商请求，并完成供应商异常分类。
- Service 不负责动画、移动、任务、战斗等 Gameplay 行为。
- Service、Context Builder 和 Provider 都不持久化会话或读取 UE 世界。

### Protocol

[Protocol.md](./Protocol.md) 是 UE 与 Python 的唯一共享边界。两端不得依赖对方的内部类型、SDK 或目录结构。

## 依赖方向

```text
ZL Gameplay/UI Context Snapshot -> ZLAIRuntime Plugin -> HTTP/JSON Protocol

FastAPI Route -> Dialogue Service
                     |-> Context Builder
                     `-> Dialogue Provider Interface -> Provider SDK
```

- Gameplay 层不处理 HTTP、JSON 或 Provider 细节。
- Route 不直接调用具体 Provider SDK。
- Dialogue Service 协调 Schema、Context Builder 和 Provider，不依赖 FastAPI HTTP 类型或具体 SDK。
- Context Builder 不依赖 FastAPI、Settings、网络、数据库或具体 Provider，并且只有它能把协议上下文转换为内部生成上下文。
- Provider 实现不得构造 UE/Python 协议响应。
- Provider 接口不得接收协议 Schema 或 UE 类型。

## 后续扩展边界

- **Memory Service**：负责存储与检索，不由 Route 或 UE Client 直接操作数据库。
- **Tool Planner**：生成结构化 Tool Call。
- **UE Tool Registry/Executor**：白名单校验、参数校验和 Gameplay 执行；最终执行权始终在 UE。

Context Builder 已进入当前里程碑；它只处理单次请求快照，不承担 Memory Service 的存储、检索、摘要或会话职责。

当前允许范围由 [CurrentMilestone.md](./CurrentMilestone.md) 定义；具体模块和类型见 [PythonModules.md](./PythonModules.md) 与 [UEClasses.md](./UEClasses.md)。
