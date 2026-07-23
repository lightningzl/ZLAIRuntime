# Project State

本文件只记录当前项目快照。范围、任务明细、验收标准和流程规则分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [DocumentationRules.md](./DocumentationRules.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-07-23 |
| 当前里程碑 | Milestone 2：真实 LLM 自由对话 |
| 里程碑状态 | `验收中` |
| 当前活动任务 | `M2-08` 真实端到端验收与交付记录 |
| 下一候选任务 | 无 |
| 已知阻塞 | 当前进程和用户环境均未配置 `MOONSHOT_API_KEY`；配置有效但不入库的本地密钥后可继续真实请求验收 |
| 验收进度 | 5/10 项已验证 |

## 实现基线

- 已完成最小 UE 到 Python 通信闭环；历史范围见 [Milestone1.md](./Milestones/Milestone1.md)，验证记录见 [Milestone1Validation.md](./Validation/Milestone1Validation.md)。
- Python Service 已完成集中配置、Provider 接口与 Factory、确定性 Stub Provider、Kimi Chat Completions API 适配器、Provider 错误分类和协议映射。
- UE 已验证字符串形式的 `stub`、`kimi` 和未来 Provider 兼容性，以及全部新增 Provider HTTP 错误码；外层请求超时为 30 秒。
- 当前真实 LLM 运行时依赖仅为 Python Service 中用于 Kimi OpenAI 兼容接口的 SDK；不存在数据库、向量库、Memory 或 Tool Use 运行时依赖。
