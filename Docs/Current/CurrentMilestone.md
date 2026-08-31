# Milestone 7：可操作交互舞台与定向感知

## 状态

- 状态：`已定义，待实施`
- 定义日期：2026-08-31
- 前置里程碑：Milestone 1 至 6 已完成
- 场景来源：[最终场景 1：开放式社会交互沙盒](../Planning/FinalScenarios/Scenario1OpenSocialSandbox.md)

历史里程碑：

- [Milestone 1：UE 到 Python Service 最小闭环](../Milestones/Milestone1.md)
- [Milestone 2：真实 LLM 自由对话](../Milestones/Milestone2.md)
- [Milestone 3：NPC 上下文与人格](../Milestones/Milestone3.md)
- [Milestone 4：持久化对话 Memory](../Milestones/Milestone4.md)
- [Milestone 5：确定性社会模拟基础](../Milestones/Milestone5.md)
- [Milestone 6：关系、长期记忆与重要 NPC](../Milestones/Milestone6.md)

长期路线见 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md)，目标系统设计见 [SocialSimulationDesign.md](../Planning/SocialSimulationDesign.md)，当前已实现边界见 [Architecture.md](../Planning/Architecture.md)。

## 目标

把 Milestone 5/6 的无界面社会模拟能力第一次落实为玩家可以直接进入、操作和观察的简单场景：

```text
玩家在平面场景中移动和转向
  -> 在 UI 中选择“说话”或“行为”并输入文本
  -> 选择小声、正常、大喊或耳边，可选目标
  -> UE 创建受控 Speech/Action Event
  -> 定向视觉与分级听觉为每个 NPC 生成独立 Observation
  -> 确定性占位规则产生可见反馈
  -> NPC 名称、朝向、对话/动作气泡和开发视图展示结果
```

本里程碑优先证明“玩家能操作、结果能显化、NPC 不获得上帝视角”。NPC 的最终自然语言质量、LLM 高层 Decision 和受控 NPC ToolCall 留到 Milestone 8。

## 玩家可操作成果

- 启动专用简单平面场景，控制玩家角色移动、停止和朝向。
- 场景中放置 3 至 5 个占位 NPC；玩家能够从名称和朝向标识区分角色。
- 打开文本交互 UI，在“说话”和“行为”模式之间切换。
- 说话模式允许输入 1 至 512 个 Unicode 字符，选择小声、正常、大喊或耳边，并选择可选目标。
- 行为模式至少识别并执行面向、靠近、远离和停止；需要目标的行为允许选择目标。
- 无法识别、目标缺失、距离不合法或无法执行的输入会得到明确 UI 反馈，不会假装世界已经改变。

## 屏幕可见成果

- NPC 头顶显示名称；场景中可辨认玩家和 NPC 的当前朝向。
- 玩家成功说话后显示玩家说话气泡；NPC 的确定性占位回应必须明确标识为本地规则反馈，不冒充 LLM 输出。
- 行为实际开始或完成后显示动作气泡或简短状态标识，文本描述必须与真实执行状态一致。
- 开发视图可以选择一个 NPC，显示其是否看见玩家、是否听见、听觉强度、说话模式、显式目标、目标判断和最近一次 Observation。
- 小声、正常、大喊、耳边、视野内、视野外、显式目标和无目标输入可以仅通过场景表现与开发视图区分，不要求查看原始日志。

## 本阶段范围

### 可操作场景与角色适配

- 创建仅用于最终场景 1 演进的简单平面测试场景；第一版使用占位几何体或胶囊体，不依赖正式美术。
- 提供 1 个玩家角色和 3 至 5 个稳定 ID 的 NPC，保存位置、朝向、显示名和基础感知配置。
- 玩家移动、朝向和停止由 UE Gameplay 权威执行；文本只表达请求，不能直接伪造位置或动作完成。
- 场景可以重复重置到确定初始状态，便于人工对照和自动化验证。

### 文本交互 UI

- 提供可聚焦的文本输入框、输入类型选择、说话模式选择、可选目标选择、提交和结果提示。
- 说话类型：`Whisper`、`Talk`、`Shout`、`InEar`；显示文本使用小声、正常、大喊、耳边。
- `InEar` 必须具有明确目标并满足近距离约束；其他说话模式可带目标，也可不带目标。
- 无目标说话表示“目标尚未解析”，不等于对所有 NPC 下达指令；NPC 可以听见内容，但目标判断必须单独记录。
- UI 只在本地显示本次玩家输入，不将完整输入写入长期日志、Validation 或调试文件。

### Speech 与 Action Event

- Speech Event 至少包含 Event ID、说话者、原始文本、模式、可选显式目标、位置、朝向、时间和生命周期。
- Action Event 至少包含 Event ID、行为类型、执行者、可选目标、开始/完成状态、位置和时间。
- 说话输入与行为输入必须严格区分：NPC 不会听见行为输入文本，只能通过视觉观察已经开始或完成的实际行为。
- 所有字符串、目标、模式、时间和生命周期均有边界校验；无效输入在产生 Gameplay 副作用前被拒绝。
- 新类型优先放在 `ZLASocialRuntime` 的纯数据与规则边界中；具体 Widget、Pawn、Actor 和表现适配属于 `ZL` 游戏模块。

### 定向视觉

- 在现有二维空间查询基础上加入观察者朝向、水平视野角和最大视觉距离过滤。
- 默认测试配置使用可调整的 120 度水平视野角和 1500 cm 最大视觉距离；最终数值由场景配置提供，不硬编码到通用规则。
- NPC 能区分玩家位于视野内、视野外、距离外，以及是否看见玩家开始或完成一个可见行为。
- 第一版场景是无复杂遮挡平面；不把真实三维遮挡、楼层或复杂 LOS 作为完成条件。

### 分级听觉与目标判断

- 默认测试基线：Whisper 200 cm、Talk 800 cm、Shout 2500 cm、InEar 150 cm；参数可由场景配置覆盖。
- Whisper、Talk、Shout 根据距离生成有界听觉强度；InEar 只向满足距离的显式目标产生定向听觉 Observation。
- 普通说话即使具有显式目标，范围内旁观者仍可听见；“听见内容”和“认为自己是目标”必须分离。
- Milestone 7 不要求使用 LLM 解析称呼和开放语义。无目标输入只通过显式目标缺失、玩家朝向、距离和规则标记为 `Unresolved` 或候选目标，不生成确定语义结论。
- 每个 NPC 只保存自己的 Observation；开发视图不得用完整接收者列表反向填充 NPC 的个人知识。

### 最小行为输入与执行

- 行为模式使用确定性白名单解析，至少覆盖 Face、Approach、MoveAway 和 Stop 的中文/英文受控别名。
- Face、Approach 和 MoveAway 在需要时校验目标存在；Stop 允许无目标。
- 行为只有在 Gameplay 执行器接受后才产生 Started/Completed 结果；拒绝、取消和失败不记录为已完成。
- NPC 可以通过视觉感知实际开始或完成的玩家行为，但不会读取玩家的行为输入原文。
- 本阶段行为输入只控制玩家角色，不允许 LLM 或 NPC 自主调用 Gameplay Tool。

### 气泡与开发视图

- 玩家和 NPC 使用有界时长、可覆盖或排队规则明确的头顶气泡；气泡销毁和角色注销不会留下悬挂引用。
- 对话气泡只显示真正说出口的文本；动作气泡只显示已经接受并开始执行的动作摘要。
- 确定性 NPC 占位回应使用稳定模板，并在开发模式中标记来源为 `RulePlaceholder`。
- Inspector 显示结构化 Observation、过滤原因和输入来源，不显示完整 Prompt、API Key、持久化 scope、模型隐式推理或未经裁剪的玩家历史。

### 自动化与人工验收

- 为说话模式校验、范围边界、InEar 目标约束、方向视野、个人 Observation 和行为白名单提供确定性自动化测试。
- 提供场景级自动化或 Functional Test，覆盖至少 1 个玩家和 3 个 NPC 的视野内、视野外、近距离与远距离组合。
- 提供一条不依赖 Python Service 的人工操作路径，能够在编辑器中复现全部可见验收点。
- 完成受影响 UE Target 编译、现有 `ZL.Social` 回归和适用 Dialogue/Protocol 回归。

## 数据与权威边界

- UE 是角色位置、朝向、移动、Speech/Action Event、感知结果和气泡展示状态的唯一事实来源。
- `ZLASocialRuntime` 不依赖 Widget、Pawn、Actor、AIController、HTTP 或 Python；`ZL` 负责把具体场景对象适配到纯数据接口。
- 当前 [Protocol.md](../Reference/Protocol.md) 和 `POST /v1/dialogue` 请求/响应保持不变；Milestone 7 不新增 Decision Endpoint、ToolCall 或协议字段。
- Python Service 不参与 Speech/Action Event 的空间传播、视觉/听觉过滤、玩家移动或占位反馈。
- 玩家文本是不可信数据，不得覆盖系统边界、生成未注册动作或作为日志中的完整原文长期保存。
- 所有输入长度、事件生命周期、气泡数量、Observation 数量和每次空间查询工作量必须有硬上限。

## 明确不做

- 不新增或修改 Decision Endpoint、ToolCall、结构化 AI Decision 或现有 Dialogue 协议。
- 不把 Relationship、Social Memory 或个人 Observation 自动注入现有 Dialogue 请求。
- 不实现 Milestone 8 的单 NPC LLM 具身反馈、NPC 自主动作或 Tool Executor。
- 不实现 Milestone 9 的连续冲突升级、基础战斗、受击、防卫、逃跑、求助或道歉后的 LLM 重新规划。
- 不正式集成 MassEntity、Representation LOD、World Partition 或 100+ NPC 可视化场景。
- 不实现复杂导航、三维楼层、室内声学、遮挡传播或正式美术与动画资产。
- 不实现语音识别、语音合成、流式 Token、唇形同步或完整表情系统。
- 不实现谣言、多源事实确认、承诺、任务、经济、犯罪、阵营政治或新的长期 Memory 语义。
- 不将固定威胁台词、固定 NPC 回复或固定剧情路线写成验收条件。

## 验收标准

| ID | 标准 |
| --- | --- |
| `M7-A01` | Milestone 1 至 6 的适用 Dialogue、Context、Memory、社会模拟、错误和超时回归保持通过；`Protocol.md` 与 Python Runtime 行为不改变。 |
| `M7-A02` | 专用平面场景可在编辑器中启动并重置，包含 1 个可移动/转向的玩家和 3 至 5 个具有稳定 ID、名称与可见朝向的 NPC。 |
| `M7-A03` | 玩家可以在 UI 中切换说话/行为、输入文本、选择 Whisper/Talk/Shout/InEar 和可选目标；空白、过长、非法模式、InEar 无目标或距离不合法会在产生副作用前得到可见拒绝。 |
| `M7-A04` | Whisper、Talk、Shout 和 InEar 使用可配置边界产生可重复的不同接收结果；普通说话的“听见”和“被指向”分离，InEar 只由合法显式目标接收。 |
| `M7-A05` | 定向视觉使用位置、朝向、视野角和距离过滤；同一玩家行为对视野内、视野外和距离外 NPC 产生不同且可见的 Observation 结果。 |
| `M7-A06` | 每个 NPC 只获得自己的视觉/听觉 Observation；无目标说话不会默认被解释为对所有 NPC 下达指令，开发视图可以逐 NPC 检查来源和过滤原因。 |
| `M7-A07` | 行为输入至少能实际执行 Face、Approach、MoveAway 和 Stop；未知、缺目标或无法执行的行为得到可见拒绝，NPC 只感知实际开始/完成的行为而不读取行为原文。 |
| `M7-A08` | 玩家说话、NPC 确定性占位回应和已接受行为分别通过对话/动作气泡或状态标识显化；气泡内容、来源和实际世界状态一致，不查看日志即可完成主要观察。 |
| `M7-A09` | Speech/Action Event、Observation、气泡和空间查询均有硬上限且事件驱动；自动化覆盖范围边界、方向边界、目标约束、个人视角、行为白名单、拒绝和场景重置。 |
| `M7-A10` | 受影响 UE Target 编译、适用 UE/Python 回归、场景级自动化和人工操作路径通过；Validation 记录可复查证据且不保存完整玩家输入、Prompt、scope 或凭据。 |

## 完成定义

1. [TaskBoard.md](./TaskBoard.md) 中 Milestone 7 工作包全部完成。
2. `M7-A01` 至 `M7-A10` 均有可复查验证证据。
3. 玩家能够从编辑器启动场景，独立完成“移动/转向 -> 说话或行为输入 -> 感知差异 -> 气泡/动作反馈”操作，不依赖命令行日志理解结果。
4. 现有 Dialogue 协议与 Python Runtime 保持不变，未提前实现 Milestone 8 至 10 能力。
5. 实际验证写入 `Docs/Validation/Milestone7Validation.md` 后，才允许将本文件定稿归档为 `Docs/Milestones/Milestone7.md`。
