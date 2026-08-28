# Task Board

## 文档职责

本文档只维护 [CurrentMilestone.md](./CurrentMilestone.md) 的当前工作包、依赖、状态和完成条件。验收标准正文只存在于 Current Milestone，实际证据只写入对应 Validation 文档。

## 状态定义

| 状态 | 含义 |
| --- | --- |
| `待开始` | 范围和前置条件明确，尚未实施 |
| `进行中` | 已开始实施；同一任务只有一个主要执行者 |
| `受阻` | 无法继续，已记录原因和解除条件 |
| `待验收` | 实施完成，正在执行规定验证 |
| `已完成` | 产物、适用验证和文档均已完成 |

## Milestone 6 工作包

| ID | 状态 | 工作包 | 主要产物 | 依赖 | 完成条件 |
| --- | --- | --- | --- | --- | --- |
| `M6-01` | `已完成` | M6 范围与文档准备 | Current Milestone、Task Board、Project State 与长期路线状态同步 | Milestone 5 | M6 范围、不做项、验收 ID 和依赖明确；文档检查通过 |
| `M6-02` | `已完成` | Event Chain 数据模型 | Root/Parent、Depth、Budget、Causation、Social Channel 与派生 Event 基础类型 | `M6-01` | 自动化覆盖 `M6-A02` 的字段、生命周期和非法输入边界 |
| `M6-03` | `已完成` | 有界 Social Propagation | 显式 Report 确认 API、Confidence 衰减、Fan-out/Budget/TTL 与 Root 去重 | `M6-02` | 自动化覆盖 `M6-A02`、`M6-A03`，决策本身不产生成功副作用 |
| `M6-04` | `已完成` | 稀疏 Relationship 与 Faction Authority | Personal Relationship、Reputation、Faction Standing、确定性 Delta 与 Authority Capability | `M6-02` | 自动化覆盖 `M6-A04`、`M6-A05`，不预建 N×N 图 |
| `M6-05` | `已完成` | Important NPC Long Memory | Level 2 Profile、Short/Long 容量、Promotion、Decay、Eviction 和结构化 Top-K 检索 | `M6-02` | 自动化覆盖 `M6-A06`，不访问 Dialogue Memory 或 SQLite |
| `M6-06` | `已完成` | 关系感知 Rule Decision | Investigate、Relationship/Memory/Faction/Occupation 评分、Reason Code 和重复报告约束 | `M6-03`、`M6-04`、`M6-05` | 自动化覆盖 `M6-A07`，验证规则路径不产生 Decision HTTP 请求，且相同输入可复现 |
| `M6-07` | `进行中` | Important NPC 报告纵向切片 | 5 个 Important NPC、Witness→Guard 数据传播、Authority 更新和 Intent 回调 | `M6-06` | 无界面场景覆盖 `M6-A08`、`M6-A09`，不声称具体行为执行已实现 |
| `M6-08` | `待开始` | Inspector 与有界性能指标 | Event Chain、关系、Faction、Long Memory 快照及 120+5 聚合基准 | `M6-07` | 满足 `M6-A10` 的可观察性、性能记录和安全输出要求 |
| `M6-09` | `待开始` | 最终回归与验收记录 | UE/Python 回归、Target 编译、自动化、无界面集成和 Validation 文档 | `M6-08` | `M6-A01` 至 `M6-A10` 均有实际、可复查证据 |

## 推荐执行顺序

```text
M6-01 -> M6-02 -> M6-03 --+
                  M6-04 --+-> M6-06 -> M6-07 -> M6-08 -> M6-09
                  M6-05 --+
```

`M6-03`、`M6-04` 与 `M6-05` 在 Event Chain 基础类型稳定后可以并行设计，但代码合入顺序必须保持模块可编译。任何协议、ToolCall、LLM Decision、Mass 正式集成、Level 3、完整 StateTree 执行或语义 Memory 需求必须返回总规划，不能加入本里程碑任务板。
