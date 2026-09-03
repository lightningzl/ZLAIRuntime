# Milestone 11：可配置角色与 NPC 技术验证

## 状态

- 状态：`已完成`
- 完成日期：2026-09-03
- 前置里程碑：Milestone 1 至 10
- 验收证据：[Milestone11Validation.md](../Validation/Milestone11Validation.md)

## 交付范围

Milestone 11 在既有社会沙盒上验证受控本地 JSON 预设能驱动玩家与 NPC 的生成、公开表现和既有个人 Decision Context；UE 保持世界、校验和执行权威，Python、协议和既有 Tool 不变。

- 版本化本地 JSON Schema、字段白名单、受控目录、完整候选校验、原子应用和公开字段导出。
- 两套示例预设，稳定 ID、显示名、颜色、出生 Transform、生命，以及 NPC 人物、表达风格、目标、初始 Relationship/Instant State 均由有效快照驱动。
- 玩家和 NPC 使用可由蓝图配置的 `ACharacter::Mesh`、动画蓝图和攻击蒙太奇；未配置资源时保留安全回退。
- NPC 使用 `ACharacter`、标准 AIController Possess 条件和可选 StateTree AIController；现有社会沙盒 Tool 和移动语义不变。
- 玩家 Input Mapping Context 由本地 PlayerController 添加，Pawn 保留 Action 绑定与既有轴映射/UI 回退。
- 普通、连招和蓄力攻击复用 Combat AnimNotify；命中只在 Notify 帧进入社会沙盒权威结算。
- NPC 实现 `ICombatDamageable`：非致命命中沿用击退边界，失能时进入 ragdoll，重置恢复角色碰撞、移动和非物理 Mesh。

## 明确不做

- 不做角色编辑器、运行时自由创建、持久化、背包、装备、职业树、资产导入、动画重定向、完整战斗、经济系统、多人同步或协议/Tool 扩展。
- 不由 AI 自动写入或应用配置，不读取任意文件路径、网络路径或隐藏运行时数据。
- 不把导航、行为树或 StateTree 配置扩展为新的 Decision Tool 或协议语义。

## 验收结论

`M11-A01` 至 `M11-A12` 已完成；实际编译、自动化、地图烟测和资源配置边界见 [Milestone11Validation.md](../Validation/Milestone11Validation.md)。需要用户配置的蓝图、网格、Physics Asset、动画、蒙太奇和 Input 资源均保持为显式配置项，未伪称为自动化验证结果。
