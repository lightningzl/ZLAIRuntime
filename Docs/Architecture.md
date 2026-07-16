# Architecture

## 总体结构

```text
Player / UI
    |
    v
UE5 Runtime
  - NPC / Interaction
  - AI Service Client
  - Response Presentation
    |
    | HTTP + JSON
    v
Python AI Service
  - FastAPI Endpoint
  - Request Validation
  - Stub Response Provider
```

当前阶段没有数据库、向量库、真实 LLM 或 Tool 执行链路。

## 模块边界

### UE5 Runtime

职责：

- 发起 NPC 对话请求。
- 收集并序列化当前阶段所需的最小输入。
- 管理 HTTP 请求、超时和失败回调。
- 解析协议响应并将文本交给 UI 或 NPC 表现层。

不负责：

- 拼接模型 Prompt。
- 调用模型供应商 API。
- 保存 AI Memory。
- 信任并直接执行任意 AI 返回内容。

### Python AI Service

职责：

- 暴露版本化 HTTP API。
- 校验请求并返回稳定的 JSON 响应。
- 当前使用 Stub 或简单规则生成回复。
- 保持未来接入 LLM、Memory 和 Tool Planner 的扩展点。

不负责：

- 直接访问或修改 UE 世界对象。
- 控制动画、移动、任务等 Gameplay 行为。
- 在当前阶段访问数据库或真实 LLM。

### Protocol

协议是 UE 与 Python 之间唯一的共享边界，定义见 [Protocol.md](./Protocol.md)。两端不得依赖对方的内部类型或目录结构。

## 依赖方向

```text
UE Gameplay/UI -> UE AI Service Client -> Protocol
FastAPI Route  -> Service Logic        -> Protocol Schema
```

- Gameplay 层不直接处理 HTTP 细节。
- FastAPI Route 只做输入输出适配，回复逻辑放在独立 Service 层。
- 当前 Stub Provider 应可在后续被 LLM Provider 替换，不影响 UE 调用方。

## 后续扩展边界

- **LLM Provider**：封装不同模型供应商，不泄漏供应商格式到 UE。
- **Memory Service**：负责存储与检索，不由 Route 或 UE Client 直接操作数据库。
- **Tool Planner**：生成结构化 Tool Call。
- **UE Tool Registry/Executor**：白名单校验、参数校验和 Gameplay 执行；最终执行权始终在 UE。

以上模块均不属于当前里程碑。
