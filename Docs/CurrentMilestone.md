# Current Milestone

## Milestone 1：UE 到 Python Service 最小闭环

目标：证明 UE5 Runtime 可以稳定调用本地 FastAPI Service，取得符合协议的 Stub 回复，并在 UE 中展示结果。

## 本阶段范围

### UE5

- 提供独立的 AI Service Client。
- 按 [Protocol.md](./Protocol.md) 构造 `POST /v1/dialogue` 请求。
- 支持可配置的 Service Base URL。
- 处理成功、网络失败、超时和非 `2xx` 响应。
- 将成功回复输出到最小可见载体，例如 UMG 文本或明确的屏幕/日志输出。

### Python

- 提供可启动的 FastAPI Service。
- 实现 `POST /v1/dialogue`。
- 校验输入并返回确定性的 Stub 回复。
- 返回统一错误格式。
- 提供最小启动说明和依赖声明。

## 明确不做

- 不接入 OpenAI 或其他真实 LLM。
- 不设计 Prompt、流式输出或 Token 管理。
- 不做玩家历史、NPC 长短期 Memory 或 SQLite 持久化。
- 不接入 Chroma、FAISS 或任何向量检索。
- 不传递完整世界状态或 NPC 人格。
- 不生成、解析或执行 Tool Call。
- 不驱动 NPC 移动、动画、任务、战斗等 Gameplay 行为。
- 不做多人同步、服务部署、鉴权、限流或生产级运维。

## 验收标准

- [ ] Python Service 可通过单条明确命令在本地启动。
- [ ] 使用合法 JSON 调用 `/v1/dialogue`，返回 `200` 且结构符合协议。
- [ ] `request_id` 和 `npc_id` 在请求与响应间保持一致。
- [ ] 响应的 `provider` 为 `stub`，运行过程不需要任何 LLM API Key。
- [ ] 缺少必填字段或字段类型错误时，返回结构化错误且 Service 不崩溃。
- [ ] UE 可配置 Service 地址并成功发出请求。
- [ ] UE 能解析并可见地展示 `reply`。
- [ ] Service 未启动、请求超时或返回非 `2xx` 时，UE 不崩溃并输出可定位的错误信息。
- [ ] 连续完成至少 10 次请求，UE 和 Service 均无崩溃或明显资源泄漏。
- [ ] 当前代码中不存在数据库、向量库、真实 LLM 或 Tool Use 的运行时依赖。

## 完成定义

以上验收项全部通过，并记录一条从 UE 输入、HTTP 请求、Service 响应到 UE 展示结果的可复现演示路径后，本里程碑完成。任何超出范围的能力进入后续 Milestone，不阻塞本阶段交付。
