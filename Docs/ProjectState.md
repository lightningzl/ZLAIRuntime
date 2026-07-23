# Project State

本文件只记录当前项目快照。范围、任务明细、验收标准和流程规则分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [DocumentationRules.md](./DocumentationRules.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-07-23 |
| 当前里程碑 | Milestone 2：真实 LLM 自由对话 |
| 里程碑状态 | `进行中` |
| 当前活动任务 | 无 |
| 下一候选任务 | `M2-04` Dialogue Service 与错误映射 |
| 已知阻塞 | 无 |
| 验收进度 | 0/10 项已验证 |

## 实现基线

- 已完成最小 UE 到 Python 通信闭环；历史范围见 [Milestone1.md](./Milestones/Milestone1.md)，验证记录见 [Milestone1Validation.md](./Validation/Milestone1Validation.md)。
- Python Service 已完成集中配置、Provider 接口与 Factory、确定性 Stub Provider、OpenAI Responses API 非流式适配器和 Fake SDK 注入边界；完整 Provider 错误映射尚未添加。
- UE 已使用字符串承载 `provider` 和服务错误码，真实 Provider 兼容性尚未验证。
- 当前不存在真实 LLM、数据库、向量库、Memory 或 Tool Use 运行时依赖。
