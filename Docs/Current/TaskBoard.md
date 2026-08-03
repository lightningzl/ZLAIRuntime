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

## Milestone 5 工作包

| ID | 状态 | 工作包 | 主要产物 | 依赖 | 完成条件 |
| --- | --- | --- | --- | --- | --- |
| `M5-01` | `已完成` | 总规划、总设计与文档治理准备 | M4 归档、规划/设计/面试文档、目录和职责规则、M5 范围 | Milestone 4 | 文档职责明确，当前范围不修改协议，链接和术语检查通过 |
| `M5-02` | `已完成` | UE 社会模拟模块与基础类型 | `ZLASocialRuntime`、Gameplay Tags、Event/Agent/Profile 基础类型 | `M5-01` | 受影响 Target 编译；模块依赖符合 `M5-A02` |
| `M5-03` | `已完成` | Event Router 与空间索引 | 注册/移动更新、Cell 查询、Event 生命周期和去重 | `M5-02` | 自动化覆盖 `M5-A03`、`M5-A04` |
| `M5-04` | `已完成` | Perception Filter | Direct/Visual/Auditory、距离衰减、过期和阈值 | `M5-03` | 自动化覆盖 `M5-A05` |
| `M5-05` | `已完成` | Personality、Instant State 与 Short Memory | DataAsset/DataTable 边界、六 Trait、四 State、固定容量 Memory | `M5-02` | 自动化覆盖 `M5-A06` |
| `M5-06` | `已完成` | Rule/Utility Decision | 候选 Intent、硬约束、优先级、迟滞、冷却和确定性平局 | `M5-04`、`M5-05` | 自动化覆盖 `M5-A07`、`M5-A08` |
| `M5-07` | `已完成` | Gameplay Intent 集成 | `ZL` Event Producer、Intent Adapter、StateTree/AIController 演示 | `M5-06` | 一个无界面纵向切片闭环；不修改 Dialogue 协议 |
| `M5-08` | `已完成` | 最小 Debug 与 100+ Agent 基准 | NPC Inspector/命令、聚合指标、固定场景性能记录 | `M5-07` | 满足 `M5-A10`，无敏感数据输出 |
| `M5-09` | `已完成` | 最终回归与验收记录 | Python/UE 回归、编译、自动化、无界面集成、Validation 文档 | `M5-08` | `M5-A01` 至 `M5-A10` 均有可复查证据 |

## 推荐执行顺序

```text
M5-01 -> M5-02 -> M5-03 -> M5-04 --+
                  `-> M5-05 --------+-> M5-06 -> M5-07 -> M5-08 -> M5-09
```

`M5-03` 与 `M5-05` 在基础类型稳定后可以并行设计，但代码合入顺序必须保持模块可编译。任何 ToolCall、Decision Endpoint、Relationship 或 NPC→NPC 传播需求都应返回总规划，不能加入本里程碑任务板。
