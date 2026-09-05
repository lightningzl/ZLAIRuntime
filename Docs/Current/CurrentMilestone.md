# Milestone 13：NPC 内容配置、批量管理与场景投放

## 状态

- 状态：`已完成`
- 开始日期：2026-09-04
- 前置里程碑：[Milestone12.md](../Milestones/Milestone12.md)
- 验收证据：实施后创建 `Milestone13Validation.md`

## 目标

把当前项目级 JSON 预设升级为可复用的 NPC 内容生产管线。策划应能通过 Asset、DataTable/DataRegistry、JSON 和场景 Spawner 配置 NPC 的身份、性格、背景、目标、初始关系与出生位置，而无需修改 C++。配置驱动 LLM 的人物 Context，但不能直接创建世界事实或绕过 UE 的 Decision/执行校验。

## 玩家可操作成果

- 在编辑器中为 NPC 选择 Persona Asset 或 DataRegistry Persona ID；场景开始后看到正确 NPC 类、名称、出生位置和人物反馈。
- 在 NPC 配置编辑界面导入单条 JSON 自动填充 Asset，或导出当前 Asset 为 JSON。
- 使用批量 JSON 导入 Persona 行：同 ID 覆盖、不同 ID 新增；可导出整个已选 DataTable 的 Persona 行。
- Spawner 的 Persona ID 从已配置 DataRegistry 的有效 ID 中可搜索选择；单条与批量 JSON 均可直接粘贴到编辑器导入。
- 在场景中放置 NPC Spawner，通过 NPC 类 + Asset 或 NPC 类 + Persona ID 在固定位置生成 NPC。

## 屏幕可见成果

- NPC 的身份、性格、表达风格、目标和初始立场会进入 Inspector 与 Decision Context；不同配置产生可解释的差异，但不预写固定台词或行动链。
- Spawner 在解析不到 ID、Asset 或 NPC 类时显示明确的配置失败反馈，不生成半初始化 NPC。

## 本阶段范围

### Persona Asset 与单条 JSON

- 在插件定义可复用 `NPC Persona` 配置模型：稳定 ID、显示名、背景摘要、身份、性格、表达风格、目标、初始关系和即时状态；字段须有上限与校验。
- 提供 Editor 可用的单条 JSON 导入/导出操作；导入先校验完整 Schema，失败不改变已有 Asset。
- Persona Asset 可被角色/NPC 组件直接引用；项目层只负责将该 Persona 与具体 Actor、网格、动画和世界事件绑定。

### DataTable、DataRegistry 与批量 JSON

- Persona 的扁平行结构可用作 DataTable Row；插件 Setting 显式配置一个或多个 Persona DataRegistry 来源。
- 支持从当前 Setting 指向的 DataRegistry 查询稳定 ID，用于详情面板下拉选择；不扫描任意资产路径。
- 提供批量 JSON 导入/导出：相同 ID 覆盖对应行、不同 ID 新增；导入在写入前完成 Schema、重复 ID、字段白名单和数值校验，并报告逐行结果。
- 不把运行时个人经历、Prompt、服务回复、密钥或 DataRegistry 内部路径写入 JSON。

### NPC Spawner

- 提供可放置的 `NPC Spawner`，保存固定 Transform、NPC 类，以及互斥的 Persona Asset 或 Persona ID 引用。
- ID 方式在运行时通过配置的 DataRegistry 解析 Persona；Asset 方式不依赖 Registry。两者都必须在生成前验证 NPC 类、Persona 与稳定 ID。
- Spawner 只负责生成和初始化，不负责导航、行为树、战斗、网络同步或让 LLM 创建角色。

### 配置与现有 Decision 的接入

- Persona 的身份、背景摘要、性格、说话风格、目标、初始关系和即时状态进入现有单 NPC Decision Context；运行时社会后果仍由 UE 维护，不能被导入配置覆盖为“已经发生”。
- 维持当前 `/v1/decision`、`/v2/decision` 字段和执行边界。本里程碑不修改 UE/Python 协议；如实施时确需新增字段，必须另行说明并取得确认。

## 明确不做

- 不实现完整角色编辑器、资产导入/生成、捏脸、装备、背包、经济、任务或多人同步。
- 不实现动态世界事件、谣言、个人世界知识、可信度传播或世界背景变化反馈；这些属于 Milestone 14。
- 不让批量导入写入运行中的 NPC、自动替换已放置 Actor，或绕过编辑器事务/撤销边界。
- 不让 DataRegistry、Asset 或 JSON 直接定义任意 Tool、动画、伤害、导航或模型 Prompt。

## 验收标准

| ID | 标准 |
| --- | --- |
| `M13-A01` | Persona Asset 可完整保存并校验身份、背景、性格、表达、目标、关系和即时状态；单条 JSON 的有效导入/导出往返保持等价，无效 JSON 不改变原 Asset。 |
| `M13-A02` | DataTable 行与 DataRegistry Setting 能提供稳定 Persona ID；详情下拉只显示已配置 Registry 的有效 ID。 |
| `M13-A03` | 批量 JSON 导入按 ID 完成覆盖/新增，拒绝重复、未知字段和越界数据，导出不含路径、密钥、Prompt 或运行时私人事实。 |
| `M13-A04` | Spawner 能以 Asset 或 ID 两种方式在固定位置生成指定 NPC 类；无效配置可见失败且不生成半初始化对象。 |
| `M13-A05` | 由 Asset、DataTable 或 Registry 提供的 Persona 一致进入 NPC Inspector 和 Decision Context；运行时个人社会事实不跨 NPC 串线。 |
| `M13-A06` | 既有 M12 Decision v2、Stub/Kimi、离线降级及社会沙盒自动化回归保持通过；本里程碑不修改线上协议。 |

## 完成定义

1. TaskBoard 的 M13 工作包全部完成，且 `M13-A01` 至 `M13-A06` 有可复查证据。
2. 单条/批量 JSON、Asset、DataTable/DataRegistry 与 Spawner 形成同一 Persona 数据模型，不产生多份互相漂移的角色事实。
3. 玩家可在场景中观察到配置驱动的 NPC 差异，且任何配置失败都有安全、可见的反馈。
