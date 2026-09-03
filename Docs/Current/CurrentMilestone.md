# Milestone 11：可配置角色与 NPC 技术验证

## 状态

- 状态：`已完成`
- 开始日期：2026-09-02
- 前置里程碑：Milestone 1 至 10 已完成
- 历史范围：[Milestone10.md](../Milestones/Milestone10.md)
- 验收证据：[Milestone11Validation.md](../Validation/Milestone11Validation.md)

长期路线见 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md)，当前模块边界见 [Architecture.md](../Planning/Architecture.md)。

## 目标

在既有社会沙盒上验证受控本地 JSON 预设能可靠改变角色/NPC 的生成、可见表现与个人 Decision Context。预设只替换当前写死的初始数据；UE 继续作为世界、校验和执行权威，Python、协议和现有 Tool 均不改变。

```text
选择受控预设
  -> UE 读取本地 JSON 并先完成 Schema/字段/范围/稳定 ID 校验
  -> 校验成功时替换下一次场景生成快照；失败时保留当前有效场景
  -> GameMode 生成一个玩家与至少两个 NPC
  -> 名称、颜色、出生 Transform、生命和 NPC 人物/关系进入既有表现与个人 Context
  -> 原有说话、行为、攻击、Stub 与离线降级路径继续工作
```

## 协议边界

- 不修改 [Protocol.md](../Reference/Protocol.md)、两端协议类型、Endpoint、Intent、Tool 或错误码。
- JSON 仅由 UE 本地读取，绝不发送给 Python；Decision 请求仍由既有个人 Context Builder 产生。
- AI 可以在未来生成候选文本，但本里程碑不实现 AI 写文件或自动应用；所有加载都必须通过 UE 校验。

## 玩家可操作成果

- 进入测试场景时可在少量受控玩家角色预设中选择；下一次重置/进入场景后使用该角色的名称、颜色和出生位置。
- 每套预设生成一个玩家和至少两个 NPC；玩家仍可使用既有说话、行为和攻击入口。
- 修改公开 JSON 预设后重新加载场景，可观察名称、颜色、出生位置、生命、NPC 人物或初始关系变化。
- 导出当前有效预设的公开字段；导出不含 Prompt、服务响应、密钥、运行时 Observation、历史、请求或调试状态。

## 屏幕可见成果

- 玩家和每个 NPC 显示预设中的名称与颜色，并从预设出生 Transform 生成。
- Inspector 显示选中 NPC 的个人感知与既有 Decision 信息；预设改变 NPC 人物/关系时，Stub/Decision Context 同步使用新值。
- JSON 校验失败时，以简体中文明确显示原因，且当前有效场景不被半更新或清空。

## 本阶段范围

### 受控 JSON 预设

- 定义版本化 JSON Schema 和字段白名单：稳定 ID、角色类型、显示名、颜色、出生 Transform、初始生命，以及 NPC 的角色、人格、表达风格、目标、初始 Relationship/Instant State。
- 提供至少两套仓库内的安全示例预设；稳定 ID 在单份文件内唯一，角色数固定为 1 个玩家与 2 至 4 个 NPC。
- UE 在解析后、应用前校验版本、对象层级、未知字段、字符串边界、颜色、数值范围、生命、Transform、唯一 ID、角色数量和必需玩家；任一失败都不得替换当前有效快照。
- 导入、选择、导出和重置只使用有效的内存快照；路径限定在项目受控预设目录，不接收玩家任意文件路径。

### 场景接入

- 用有效预设驱动 `AZLSocialSandboxGameMode` 的玩家和 NPC 生成，移除本阶段触及的固定出生位置和固定 Profile 初始数据依赖。
- 将玩家公开显示名、颜色、出生 Transform、初始生命接入既有 Pawn；将 NPC 配置接入既有 Profile、生命、名称、颜色和个人 Context 路径。
- 保留当前 Pawn、NPC 类、UI 和四个受控 Tool；移除 `BodyMesh` 与 `FacingArrow` 占位组件，改由角色蓝图配置骨骼网格与动画蓝图；不要求新增蓝图或资产来完成代码验证。

### 攻击表现边界

- 只有现有攻击目标、距离、冷却、伤害和受击校验成功后，才允许由可选展示适配器请求一次攻击/受击表现；拒绝攻击不得请求命中表现，且不改变玩家镜头朝向。
- `Variant_Combat` 的动画蒙太奇、AnimNotify、蓝图引用和资源绑定由用户配置；本里程碑的 C++ 只提供无资源依赖的可选接口与安全回退。

## 明确不做

- 不做角色编辑器、运行时自由创建、持久化、背包、装备、职业树、资产导入、动画重定向、完整 `Variant_Combat` 接入、连招、蓄力、敌人刷新、复杂战斗、多人同步或任何协议/Tool 扩展。
- 不由 AI 自动写入、自动应用或绕过校验加载 JSON；不读取用户任意路径、网络路径或隐藏运行时数据。
- 不创建、修改或提交蓝图、动画、蒙太奇、材质、地图或其他二进制资源；此类资源配置由用户完成。
- 不改变现有感知、调度、Tool、伤害、镜头或 Python Provider 语义。

## 验收标准

| ID | 标准 |
| --- | --- |
| `M11-A01` | `Protocol.md`、两端协议 Schema、Tool 与 Intent 均无变更；现有社会沙盒说话、行为、攻击、Stub 与离线降级路径保持可用。 |
| `M11-A02` | 至少两套受控 JSON 预设可由同一受限入口加载，每套恰有 1 个玩家与 2 至 4 个唯一稳定 ID NPC。 |
| `M11-A03` | 预设的公开名称、颜色、出生 Transform 与初始生命可靠驱动玩家/NPC 生成和可见表现。 |
| `M11-A04` | NPC 的角色、人格、表达风格、目标、初始 Relationship/Instant State 从预设进入既有 Profile 与个人 Decision Context，不混入其他 NPC 数据。 |
| `M11-A05` | Schema 版本、字段白名单、类型、稳定 ID、角色数量、数值和路径均被校验；无效文件不会改变当前有效场景。 |
| `M11-A06` | 导出仅包含允许公开配置字段；不包含 Prompt、服务响应、密钥、运行时历史、Observation 或调试信息。 |
| `M11-A07` | 合法攻击才触发一次可选攻击/受击展示请求；拒绝攻击不触发命中表现，镜头朝向保持不变；未配置资源时稳定回退。 |
| `M11-A08` | 受影响 UE Target 编译、配置解析自动化、预设切换场景烟测、Stub 与离线降级烟测完成；需要用户在蓝图/资源编辑器完成的步骤单列且不伪称已验证。 |
| `M11-A09` | `AZLSocialSandboxNpc` 继承 `ACharacter`，使用内置 Capsule/Mesh/CharacterMovement，生成时可被 AIController Possess；既有社会沙盒决策、伤害、受控移动和协议语义不变。 |

## 完成定义

1. [TaskBoard.md](./TaskBoard.md) 的 M11 工作包全部完成，且 `M11-A01` 至 `M11-A09` 有可复查证据。
2. 两套预设在同一地图切换时，玩家/NPC 可见属性及每个 NPC 的个人 Context 均随配置变化。
3. 无效 JSON、非法字段或越界值不会改变当前有效场景；导出保持脱敏、有限且可再次导入。
4. 所有必须由用户完成的蓝图或资源绑定均以明确、可执行的清单交付，未配置时 C++ 路径保持可运行。

## 追加范围：角色攻击与输入配置

- 角色攻击表现采用与 `ACombatCharacter` 相同的“角色持有蒙太奇配置、运行时只负责播放”的边界，但不复用其连招、蓄力、Trace 或 AnimNotify 命中逻辑。
- 玩家和 NPC 分别暴露攻击/受击蒙太奇、Section 与播放速率；现有社会沙盒攻击校验和伤害先完成，才允许播放对应表现。
- 玩家暴露 Enhanced Input Mapping Context 与移动、视角、攻击 Input Action 配置；未配置时保留已有轴映射输入和 UI 攻击入口。
- 不修改用户已有蓝图、蒙太奇、Input Action 或 Mapping Context 资源；不将资源路径写入 JSON 或协议。

## 追加范围：NPC Character 与 AIController 基础

- `AZLSocialSandboxNpc` 改为继承 `ACharacter`，使用内置 Capsule、Mesh 与 CharacterMovement；生成的 NPC 自动具备标准 AIController 的可 Possess 条件。
- 本次只建立 Character/Controller 基础，不新增 Behavior Tree、StateTree、导航 Tool、寻路语义或协议字段；既有受控移动、决策、伤害和攻击校验保持不变。

## 用户资源配置清单

1. 创建玩家蓝图，父类为 `AZLSocialSandboxPawn`；在其内置 `Mesh` 设置骨骼网格、动画蓝图、相对位置与旋转。
2. 创建 NPC 蓝图，父类为 `AZLSocialSandboxNpc`；在其内置 `Mesh` 设置骨骼网格、动画蓝图、相对位置与旋转。该角色可由标准 AIController Possess。
3. 在两类蓝图的 `Sandbox|Combat Presentation` 中分别设置 Attack/Hit Montage、可选 Section 和播放速率；未设置时攻击、伤害和 UI 仍按既有权威路径工作，只不播放动画。
4. 在玩家蓝图的 `Sandbox|Input` 中设置 Mapping Context 与 Move/Look/Attack Input Action。Move/Look 使用 `Axis2D`，Attack 使用数字/布尔触发；未设置时仍可用既有轴映射和 UI 攻击。
5. 创建或配置 GameMode 蓝图（父类 `AZLSocialSandboxGameMode`），将 `SandboxPlayerClass` 和 `SandboxNpcClass` 指向上述角色蓝图，并在地图 World Settings 选用该 GameMode。资源路径不进入 JSON 或网络协议。
