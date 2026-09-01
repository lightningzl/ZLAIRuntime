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

### `M9-T03` 玩家基础攻击与权威冲突状态

| 验证项 | 实际结果 |
| --- | --- |
| UE Target 编译 | `ZLEditor Win64 Development` 编译成功；新增 Attack 行为、纯数据攻击校验、NPC 生命/防卫/失能和场景反馈均通过编译链接 |
| UE 社会回归 | NullRHI 执行 `ZL.Social`，收集 26 项并全部 `Success`，退出码 0 |
| Attack 边界 | `AttackValidation` 覆盖合法命中、超距、冷却、玩家忙碌和目标失能拒绝 |
| 伤害权威与零副作用 | `NpcDecisionAction` 覆盖防卫减伤、合法伤害状态版本推进、受击无敌拒绝的生命/版本不变、失能和失能后拒绝 |
| 既有回归 | Action Parser、Observation、Tool Registry、连续调度、个人上下文和其余社会模拟测试同批通过 |

本工作包尚未完成 `M9-A03`、`M9-A04` 的最终场景操作验收；以上结果作为对应实现基础证据，最终状态在集成验证后更新。

### `M9-T05` Python 连续 Planner 行为

| 验证项 | 实际结果 |
| --- | --- |
| 定向 Planner 回归 | 项目虚拟环境执行 Stub、Decision Context Builder 与 Kimi Planner 测试，11 项全部通过，退出码 0 |
| 离线连续路径 | Stub 覆盖可感知 Attack 的 `engage + move_away`、道歉的 `respond + stop`、近期 Attack 后的 `disengage + move_away`；均仅使用既有允许 Tool |
| Kimi 约束 | 系统指令明确要求依据最新 Trigger 与有限个人历史保持连续性；禁止虚构事件、关系、生命、命中或伤害事实 |
| 协议边界 | 未修改 Decision Schema、HTTP 路由、Tool 枚举或响应字段 |

本工作包为 `M9-A05`、`M9-A06`、`M9-A10` 提供服务端基础证据；真实 Kimi 的行为属性和最终端到端状态将在收口验证中记录。

### `M9-T04` 升级、缓和与本地安全行为

| 验证项 | 实际结果 |
| --- | --- |
| UE Target 编译 | `ZLEditor Win64 Development` 编译成功，冲突状态机、GameMode 映射和本地降级反馈均完成链接 |
| UE 社会回归 | NullRHI 执行 `ZL.Social`，收集 27 项并全部 `Success`，退出码 0 |
| 纯状态边界 | `ConflictState` 覆盖初始平静、攻击升级/开启防卫、距离远离进入缓和、停止回到平静，以及 Planner 的升级/缓和意图 |
| 本地安全行为 | 请求失败会进入 `Escalated`、停止当前 Guard Decision 动作、开启防卫并显示 `LocalFallback`；不调用新 Tool 或改变协议 |

本工作包尚未完成 Stub 场景纵向路径与离线端到端验收；其自动化和可见反馈基础将在后续集成工作包完成。

### `M9-T06` 场景反馈与连续集成

| 验证项 | 实际结果 |
| --- | --- |
| Stub 默认地图烟测 | 显式 Stub Service + `/Game/SocialSandbox/Lvl_SocialSandbox` + `-ZLSandboxDecisionSmoke` 通过；Speech 接受，至少一次 `move_away` 被接受并造成 Guard 位置/版本变化。后续自动重规划被 `AutomaticReplanLimit` 截断，未形成请求风暴。 |
| 离线默认地图烟测 | 停止 Service 后以 `-ZLSandboxDecisionFallbackSmoke` 通过；来源为 `local`、结果为 `network_error`，Guard 版本推进进入防卫，但位置未变且未执行 Tool。 |
| 烟测语义 | Stub 烟测记录首次合法 Tool，避免把后续的正常自动重规划上限误判为首次行为失败；离线烟测明确允许本地防卫导致的权威状态变化。 |

以上场景证据覆盖 `M9-A07` 至 `M9-A09` 的端到端基础；升级—缓和的完整 Stub 场景和最终全量回归仍在 `M9-T07` 记录。
