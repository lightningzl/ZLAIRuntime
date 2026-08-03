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
  - Memory Service
      `- SQLite Repository
  - Context Builder
  - Dialogue Provider Interface
      |- Kimi Provider
      `- Stub Provider
```

## 模块边界

### UE5 Runtime

`ZL` Gameplay/UI 只通过 `ZLAIRuntime` 插件公开接口提交请求和消费结果。

- Gameplay/UI 负责从自身已知状态构造当前请求的 NPC 人格、世界状态和有限历史快照，并显式决定是否提供稳定的 Memory 范围。
- 插件负责请求校验与发起、端到端 HTTP 管理、协议解析和结果交付。
- 插件不从 Actor、World、GameState、SaveGame、账号、UI 或内容资产自动抓取上下文或 Memory 标识。
- UE 不负责 Prompt、模型 SDK、Provider 选择、密钥、Memory 存储检索或 Python 生成编排。
- 插件不得依赖 `ZL` 游戏模块、具体 UI 或 NPC Actor。
- Gameplay Tool 的最终校验和执行权仍属于 UE，但 Tool Use 不在当前里程碑范围。

### Python AI Service

Python Service 负责 AI 推理编排，不直接访问或修改 UE 世界。

- Route 负责 HTTP 输入输出适配。
- Dialogue Service 负责协调可选 Memory 读取、Context Builder、单次 Provider 调用和成功后的 Memory 写入。
- Memory Service 负责范围隔离、检索预算、历史合并和幂等轮次语义，不执行 SQL。
- SQLite Repository 负责 Schema、索引、连接、查询和事务，不参与 Prompt 或 Provider 编排。
- Context Builder 负责将固定系统约束、NPC 人格、世界状态、持久化历史、有限客户端历史和当前输入组装为供应商无关的生成上下文。
- Provider 接口隔离供应商 SDK 与上游数据格式。
- Provider 实现负责把内部生成上下文映射为供应商请求，并完成供应商异常分类。
- Service 不负责动画、移动、任务、战斗等 Gameplay 行为。
- Memory Service 只持久化显式启用范围中的已完成对话轮次；Route、Context Builder 和 Provider 不直接访问数据库。
- Service、Memory Service、Context Builder 和 Provider 都不读取 UE 世界。

### Protocol

[Protocol.md](./Protocol.md) 是 UE 与 Python 的唯一共享边界。两端不得依赖对方的内部类型、SDK 或目录结构。

## 依赖方向

```text
ZL Gameplay/UI Context Snapshot -> ZLAIRuntime Plugin -> HTTP/JSON Protocol

FastAPI Route -> Dialogue Service
                     |-> Memory Service -> SQLite Repository
                     |-> Context Builder
                     `-> Dialogue Provider Interface -> Provider SDK
```

- Gameplay 层不处理 HTTP、JSON 或 Provider 细节。
- Route 不直接调用具体 Provider SDK。
- Dialogue Service 协调 Schema、Memory Service、Context Builder 和 Provider，不依赖 FastAPI HTTP 类型、SQL 或具体 SDK。
- Memory Service 依赖抽象 Repository 能力和内部对话类型，不依赖 FastAPI、Provider SDK 或 UE 类型。
- SQLite Repository 不依赖 Route、Context Builder 或 Provider。
- Context Builder 不依赖 FastAPI、Settings、网络、数据库或具体 Provider，并且只有它能把协议上下文转换为内部生成上下文。
- Provider 实现不得构造 UE/Python 协议响应。
- Provider 接口不得接收协议 Schema 或 UE 类型。

## 后续扩展边界

- **Memory Retrieval 扩展**：向量检索、摘要、事实抽取和相关性评分必须继续位于 Memory Service 边界内，不得扩散到 Route、UE Client 或 Provider。
- **Tool Planner**：生成结构化 Tool Call。
- **UE Tool Registry/Executor**：白名单校验、参数校验和 Gameplay 执行；最终执行权始终在 UE。

Milestone 4 只引入 SQLite 结构化对话轮次和最近历史检索。Context Builder 接收 Memory Service 已准备的内部历史，但不承担存储、查询、事务、摘要或向量检索职责。

当前允许范围由 [CurrentMilestone.md](./CurrentMilestone.md) 定义；具体模块和类型见 [PythonModules.md](./PythonModules.md) 与 [UEClasses.md](./UEClasses.md)，SQLite 的物理结构和事务设计见 [DatabaseDesign.md](./DatabaseDesign.md)。
