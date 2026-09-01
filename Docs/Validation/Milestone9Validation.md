# Milestone 9 Validation

## 文档职责

本文档只记录 [Milestone 9](../Current/CurrentMilestone.md) 已实际执行的验证环境、命令、结果和验收证据，不重新定义范围或标准。

## 当前状态

- 验证状态：`进行中`
- 验收进度：0/10
- 当前范围整理不包含代码验证；实现与验证结果将在对应工作包完成后追加。

## 验收映射

| 验收 ID | 状态 | 证据 |
| --- | --- | --- |
| `M9-A01` | `未验证` | 待记录 |
| `M9-A02` | `未验证` | 待记录 |
| `M9-A03` | `未验证` | 待记录 |
| `M9-A04` | `未验证` | 待记录 |
| `M9-A05` | `未验证` | 待记录 |
| `M9-A06` | `未验证` | 待记录 |
| `M9-A07` | `未验证` | 待记录 |
| `M9-A08` | `未验证` | 待记录 |
| `M9-A09` | `未验证` | 待记录 |
| `M9-A10` | `未验证` | 待记录 |

## 验证记录

### `M9-T02` 连续 Decision 调度与个人历史

| 验证项 | 实际结果 |
| --- | --- |
| UE Target 编译 | `ZLEditor Win64 Development` 编译成功；新增调度器、Observation 来源、Context 合并与 GameMode 接入均通过编译链接 |
| 社会沙盒自动化 | NullRHI 执行 `ZL.Social.Sandbox`，收集 6 项并全部 `Success`，退出码 0 |
| 连续调度边界 | `ContinuousDecisionScheduler` 覆盖单在途、最新 Pending 合并、冷却、3 次自动重规划硬上限和外部输入预算重置 |
| 个人上下文 | `PersonalDecisionContext` 覆盖玩家 Observation 来源、Guard 已公开 Speech 来源、其他 Observer 过滤和协议校验 |
| 既有沙盒回归 | Stage、Motion、Per-NPC Observation 和 NPC Decision Action 同批通过 |

本工作包尚未完成 `M9-A02`、`M9-A06`、`M9-A07` 的最终场景验收；以上结果作为对应实现基础证据，最终状态在集成验证后更新。
