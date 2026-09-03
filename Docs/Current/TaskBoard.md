# Task Board

## 文档职责

本文档维护当前 [Milestone 12](./CurrentMilestone.md) 的工作包、依赖、状态和完成条件。稳定验收标准只在 Current Milestone 定义；实际证据只写入实施后创建的 [Milestone12Validation.md](../Validation/Milestone12Validation.md)。

## 当前状态

- 当前里程碑：Milestone 12——社会后果与角色行为打磨
- 里程碑状态：`验收中`
- 当前活动工作包：`M12-T06` 集成回归、场景验收与文档收口
- 下一工作包：无（等待 `M12-T06` 验收证据收口）
- 协议状态：用户已确认新增独立 Decision v2；`/v1/decision` 保持不变，v2 的计划、能力和校验字段以 [Protocol.md](../Reference/Protocol.md) 为准。

## 工作包

| ID | 工作包 | 主要成果 | 依赖 | 状态 | 完成条件 |
| --- | --- | --- | --- | --- | --- |
| `M12-T01` | 范围、v2 契约与体验探针 | 明确个人感知隔离、社会后果状态、最小交易边界、Decision v2、验收探针和无界执行约束 | Milestone 11 | `已完成` | `M12-A01` 至 `M12-A07` 及 v2 字段/校验可审查 |
| `M12-T02` | Python v2 决策质量验证 | 使用受控个人社会处境快照实现/测试 v2 Schema、Context Builder、Kimi Planner 与 Stub；验证首次/重复受击、缓和、交易、目击和求助情境的计划差异 | `M12-T01` | `已完成` | Python 先满足 `M12-A01`、`M12-A05`、`M12-A07` 的 Planner 部分 |
| `M12-T03` | UE 个人后果与 v2 Context | 攻击、威胁、缓和、报告与交易尝试形成按 NPC 隔离、有界且可检视的 UE 权威事实和 `social_situation` | `M12-T02` | `已完成` | 满足 `M12-A02`、`M12-A04` |
| `M12-T04` | UE v2 计划执行器与表现适配 | UE 验证并执行有限 `available_capabilities`，回流步骤结果；表现建议仅映射已注册资源 | `M12-T03` | `已完成` | 满足 `M12-A01`、`M12-A03` |
| `M12-T05` | 最小交易立场入口与场景反馈 | 玩家可尝试交易并看到既往行为造成的互动结果，不引入经济系统 | `M12-T04` | `已完成` | 满足 `M12-A03`、`M12-A06` |
| `M12-T06` | 集成回归、场景验收与文档收口 | Python 情境抽样、UE 自动化、Kimi/离线地图验证、Inspector 证据和受影响 Reference/Architecture 文档完成 | `M12-T05` | `进行中` | 完成 `M12-A07` 的端到端部分 |

## 推荐实施顺序

```text
M12-T01 -> M12-T02 (Python) -> M12-T03 (UE Context) -> M12-T04 (UE 执行/表现) -> M12-T05 -> M12-T06
```

## 工作规则

- 社会后果事实只由 UE 根据个人已感知事件或已确认报告更新；模型回复、未执行 Tool、隐藏推理和其他 NPC 私有 Observation 不得写入。LLM 基于这些事实自主选择高层策略，不使用固定角色反应表。
- 每个新事实必须有稳定来源、目标、时间、上限、去重、衰减和 Reset 语义；不得跨 NPC 串线或无界增长。
- 商人交易入口只表达互动立场，不能添加货币、库存、物品、价格或经济结算事实。
- `/v2/decision` 的字段、能力实例、计划预算和校验规则已获用户确认；后续实现必须逐字遵守 [Protocol.md](../Reference/Protocol.md)，v1 不得被破坏或混用。
- Task 状态变化时同步 [ProjectState.md](./ProjectState.md)；实际命令、环境和结果只写入 Validation 文档。
