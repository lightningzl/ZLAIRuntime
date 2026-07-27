# Project State

本文件只记录当前项目快照。范围、任务明细、验收标准和流程规则分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [DocumentationRules.md](./DocumentationRules.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-07-27 |
| 当前里程碑 | Milestone 3：NPC 上下文与人格 |
| 里程碑状态 | `已完成` |
| 当前活动任务 | 无 |
| 下一候选任务 | 无 |
| 已知阻塞 | 无 |
| 验收进度 | 10/10 项已验证 |

## 实现基线

- 已完成最小 UE 到 Python 通信闭环；历史范围见 [Milestone1.md](./Milestones/Milestone1.md)，验证记录见 [Milestone1Validation.md](./Validation/Milestone1Validation.md)。
- Python Service 已完成集中配置、Provider 接口与 Factory、确定性 Stub Provider、Kimi Chat Completions API 适配器、Provider 错误分类和协议映射。
- UE 已验证字符串形式的 `stub`、`kimi` 和未来 Provider 兼容性，以及全部新增 Provider HTTP 错误码；外层请求超时为 30 秒。
- 当前真实 LLM 运行时依赖仅为 Python Service 中用于 Kimi OpenAI 兼容接口的 SDK；不存在数据库、向量库、Memory 或 Tool Use 运行时依赖。
- Milestone 2 已使用 `kimi-k2.6` 完成真实 UE→Python Service→Kimi→UE 闭环、三请求同会话验证和真实 Provider 超时验证；完整证据见 [Milestone2Validation.md](./Validation/Milestone2Validation.md)。
- Milestone 2 范围定稿已归档到 [Milestone2.md](./Milestones/Milestone2.md)。
- Milestone 3 的可选 v1 `context` 协议、Context Builder 边界、工作包和验收模板已经定稿。
- M3-02 已完成：Python 与 UE 的上下文协议类型、边界校验和 UE JSON 序列化已实现；`M3-A02` 已验证。
- M3-03 已完成：无状态 Context Builder、固定系统约束和供应商无关内部生成类型已实现；`M3-A03` 已验证。
- M3-04 已完成：Dialogue Service、Stub/Fake 和 Kimi Provider 已统一接收内部生成上下文；`M3-A04`、`M3-A05` 已验证。
- M3-05 已完成：Python 上下文边界、离线保护和成功/错误日志脱敏回归已通过；`M3-A06` 已验证。
- M3-06 已完成：UE 兼容重载、上下文演示和本地 Stub 集成已通过编译及完整自动化；`M3-A01`、`M3-A07`、`M3-A08` 已验证。
- M3-07 已完成：真实 Kimi 三类受控上下文、Service 重启无状态检查和最终安全审计均通过；`M3-A09`、`M3-A10` 已验证，Milestone 3 验收完成。
