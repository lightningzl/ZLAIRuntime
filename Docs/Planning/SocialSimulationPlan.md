# ZLAI Social Simulation Runtime 总规划

## 文档职责

本文档是项目长期目标、交付范围、里程碑顺序和取舍原则的唯一规划来源。后续里程碑必须从本文裁剪，但只有 [CurrentMilestone.md](../Current/CurrentMilestone.md) 能定义当前允许实施的范围。

本文档不维护当前任务状态、实际验证结果、协议字段正文或实现文件清单。目标系统细节见 [SocialSimulationDesign.md](./SocialSimulationDesign.md)，当前实现边界见 [Architecture.md](./Architecture.md)。

## 项目目标

项目名称：**ZLAI Social Simulation Runtime**。

目标是制作一个小规模但系统深度高、可以直接游玩的 UE AI Gameplay Runtime。玩家可以输入开放自然语言并尝试带目标或无目标的行为；少量 NPC 只依据自己实际看见、听见、记住和相信的内容，由 LLM 产生符合人物与上下文的语言和高层行动建议，再由 UE 将合法结果落实为可见行为。项目不是单纯的 AI Chat NPC，也不以后台社会数值或无界面测试代替玩家体验。

最终 Demo 需要同时证明：

- UE Gameplay、AI、StateTree/Mass、动画与运行时工具能力。
- LLM 决策编排、结构化输出、失败降级和成本控制能力。
- 模块边界、协议、安全执行、性能预算和可验证架构能力。
- 每个未来里程碑都有玩家可操作入口、屏幕可见结果和失败时可观察的反馈。

## 实现状态入口

本文档不复制实时完成状态。当前已完成能力、活动里程碑和下一候选任务分别以 [ProjectState.md](../Current/ProjectState.md)、[CurrentMilestone.md](../Current/CurrentMilestone.md) 和 [TaskBoard.md](../Current/TaskBoard.md) 为准；已经接受并实现的模块边界见 [Architecture.md](./Architecture.md)。

## 核心原则

1. **UE 权威**：位置、状态、关系、伤害、移动、动画、生成和行为执行由 UE 决定。
2. **LLM 只做高层推理**：模型输出 Decision、Dialogue 和受约束 ToolCall，不直接控制世界。
3. **先确定性模拟，后接入 LLM**：Level 1 和所有降级路径不依赖外部服务。
4. **事件驱动**：Event 产生时进行空间查询和调度，不每帧扫描全部 NPC。
5. **有限预算**：传播深度、Fan-out、Memory、LLM 并发、Token 和 Tool 执行都必须有硬上限。
6. **可观察、可复现**：关键状态、事件链、决策来源和 Tool 校验可以在 Runtime Debugger 中检查。
7. **纵向切片优先**：每个里程碑都必须形成可运行、可测试的闭环。
8. **场景倒推**：先由最终场景确定体验和观察属性，再决定底座、协议与任务；不因已有模块而强行塑造场景。
9. **显化与可操作**：未来里程碑不得只交付类型、接口、日志或无界面测试；玩家必须能主动触发能力并从场景、角色移动、气泡或调试面板直接观察结果。

## 最终 Demo 范围

最终 Demo 可以由多个相互独立的最终场景组成，并按场景逐个设计、实现和验收。场景文档负责描述玩家体验、开放交互边界和可观察结果，不以固定剧情或固定模型输出代替验收；具体实现仍必须进入 Current Milestone 后才获得授权。

当前第一份场景设想：

- [最终场景 1：开放式社会交互沙盒](./FinalScenarios/Scenario1OpenSocialSandbox.md)

### 最终场景 1 的体验中心

- 一个简单平面和 1 个玩家角色。
- 3 至 5 个具有不同人物、关系与经历的具身 NPC。
- 玩家通过文本输入自由说话或描述行为，输入可带目标也可没有目标。
- NPC 具有定向视觉和分级听觉，只能使用个人实际感知的信息。
- NPC 的语言、动作气泡、朝向和移动构成主要反馈。
- 语言威胁、靠近、攻击、退让、道歉、逃跑和求助属于同一开放互动连续谱，不拆成重复的“战斗沙盒”。

### 独立规模与性能证明

以下数量作为独立性能场景和架构能力证明，不要求在最终场景 1 中同时出现：

- 100 至 120 个 Level 1 Mass NPC。
- 5 个 Level 2 Important NPC。
- 1 个完整 Level 3 Core NPC。
- Civilian、Merchant、Guard 三个基础阵营。

### 开放输入与有限执行

- 玩家语言内容原则上不预先枚举；说话模式至少包含小声、正常、大喊和针对单人的耳边说话。
- 玩家可以输入带目标或无目标的行为描述；语义输入可以开放，但只有映射到已注册能力的动作才能执行。
- 第一阶段可执行动作以面向、靠近、远离、停止、跟随、逃跑、求助和基础冲突动作为主。
- NPC 生成语言可以开放；NPC 动作必须通过 UE 白名单、目标、距离、状态版本、能力和冷却校验。

### 关键展示闭环

1. 玩家在场景中移动、转向，使用说话或行为输入，并能选择说话方式与可选目标。
2. 玩家能直接观察哪些 NPC 看见或听见了输入；未感知信息不会进入对应 NPC 的反馈。
3. 一个关键 NPC 能根据人物、关系、当前状态和最近交互生成自然语言与受控动作，并通过气泡和移动呈现。
4. 同一句输入在既有矛盾对象与陌生人之间产生合理但不固定的差异。
5. NPC 能随玩家靠近、远离、攻击、停止、解释或道歉更新判断，语言反馈和实际行为保持连续。
6. 冲突可以升级为防卫、逃跑或求助，也可以因玩家行为缓和；LLM 决定高层目标，UE 执行即时动作。
7. 3 至 5 个 NPC 各自使用个人视角作出差异化反馈，目标为空时不会自动把话语广播为对所有人说。
8. UE 拒绝未知、过期、越权或上下文失效的动作建议，并把拒绝结果显化在调试视图中。
9. Python 离线、超时或输出无效时，NPC 仍通过可见气泡和基础动作完成确定性降级。
10. 100+ Level 1 NPC 的事件处理、空间查询和规则性能通过独立场景展示，不挤占最终场景 1 的交互质量。

## MVP 取舍

MVP 先深度打磨第一个最终场景；后续最终场景在前一场景形成可运行、可验证闭环后逐个加入。MVP 优先保证 3 至 5 个 NPC 的感知一致、人物一致、多轮连续与行为落地，只实现支撑场景所需的最小 Intent 和 Tool。明确不做：

- 开放世界经济、任务生成和完整城市政治。
- 向量数据库、Embedding、知识图谱和自动事实抽取。
- 多人网络同步、生产级账号、云存档和数据合规系统。
- 任意模型动态创建新 Tool。
- 超过一个完整 Core NPC。
- AI Boss Director 的正式实现。

## 里程碑路线

### Milestone 5：确定性社会模拟基础

构建纯 UE 的 Event → Spatial Query → Perception → Instant State → Short Memory → Rule Decision → Gameplay Intent 闭环，并验证 100+ Level 1 NPC。不得修改现有 Dialogue 协议或接入 ToolCall。

### Milestone 6：关系与重要 NPC

增加 Long-Term State、Relationship、Faction Standing、Long Memory、事件二级传播和 5 个 Important NPC。所有行为仍可由规则完成，Python 只保留现有对话能力。

### Milestone 7：可操作交互舞台与定向感知

本里程碑不修改 Dialogue 协议，不实现结构化 ToolCall。

- **玩家可操作成果**：进入简单平面场景，控制玩家移动和朝向；在 UI 中选择说话或行为模式，输入文本，选择小声、正常、大喊或耳边，并可选目标；行为模式至少能识别并执行面向、靠近、远离和停止，无法识别时明确反馈。
- **屏幕可见成果**：3 至 5 个占位 NPC 显示名称、朝向、对话/动作气泡；开发视图能显示每个 NPC 是否看见、听见、听清以及是否认为自己是目标。
- **最小闭环**：玩家输入 -> UE 产生 Speech/Action Event -> 定向视觉/分级听觉 -> NPC 专属 Observation -> 确定性占位反馈 -> 气泡或移动结果。
- **完成标志**：不看日志也能在场景中操作并验证小声、正常、大喊、耳边、视野方向、显式目标和无目标输入的差异。

### Milestone 8：单 NPC LLM 具身反馈与受控动作

在单独获得协议确认后，为一个 Important/Core NPC 增加个人视角 Decision 请求、Python Planner、结构化表达、UE Tool Registry/Executor、过期检测和失败降级。

- **玩家可操作成果**：玩家可以对一个具名 NPC 自由说话或描述基础行为，并继续移动改变双方距离和朝向。
- **屏幕可见成果**：NPC 通过自然语言气泡回应，并实际执行面向、靠近、远离或停止等受控动作；非法或过期动作不会执行，调试视图显示拒绝原因。
- **最小闭环**：个人 Observation + 人物/关系/最近历史 -> LLM Speech/Intent/Tool 建议 -> UE 校验 -> 气泡与实际移动 -> 执行结果 Event。
- **完成标志**：真实模型与 Stub 都能完成一次自由输入到可见语言和实际动作的闭环；Python 不可用时场景仍可操作。

### Milestone 9：连续互动、冲突升级与缓和

让同一个 NPC 在多轮输入和连续世界变化中保持人物与行为一致。冲突和基础战斗属于开放互动的升级阶段，不建设另一套 LLM 战斗沙盒。

- **玩家可操作成果**：玩家可以威胁、靠近、停止、退让、道歉、求助或发起基础攻击，随时改变原有路线。
- **屏幕可见成果**：NPC 能随距离、可见行为、受击、玩家停止或语言变化更新表达和动作；允许出现嘴硬但后退、警告后逃跑、求助、防卫或接受缓和等非固定结果。
- **最小闭环**：显著变化触发重新判断 -> LLM 更新高层目标与表达 -> UE 继续执行即时移动、防卫和基础冲突动作 -> 新结果再次进入感知。
- **完成标志**：至少一段多轮互动可以从普通交流自然升级为冲突并再次缓和，NPC 不遗忘自己刚刚的立场，也不引用未感知事实。

### Milestone 10：最终场景 1 多 NPC 交付

将前述能力扩展到 3 至 5 个差异化 NPC，完成 [最终场景 1](./FinalScenarios/Scenario1OpenSocialSandbox.md) 的正式验收与展示。

- **玩家可操作成果**：玩家可以自由选择对象或不选对象，连续与不同 NPC 交谈、接近、离开或发生冲突，不需要按固定剧本操作。
- **屏幕可见成果**：不同 NPC 根据各自视觉、听觉、人物、关系和历史产生不同语言与动作；旁观者只对实际感知内容作出反应。
- **交付成果**：场景 UI、角色与气泡表现、个人视角 Inspector、Decision/Tool 校验记录、真实模型演示、Stub 回归和失败降级全部可现场操作。
- **完成标志**：`FS1-01` 至 `FS1-11` 全部通过，并形成演示指南、录屏路径、性能与真实模型验收记录。

### Milestone 11：可配置角色与 NPC 技术验证

在现有社会沙盒之上验证“配置改变场景角色”的最小闭环。它是技术 Demo，不建设角色编辑器或完整战斗玩法：使用受控配置替代当前固定写死的角色/NPC 初始数据，并证明配置会可靠驱动生成、展示与既有个人 Context。

- **玩家可操作成果**：进入测试场景时可从少量预设玩家角色中选择一个；选择后以该角色的名称、颜色和出生位置进入场景，并继续使用已有说话、行为和攻击入口。
- **屏幕可见成果**：场景按一个玩家配置与最少两个 NPC 配置生成；修改预设的名称、颜色、出生位置或 NPC 人物/初始关系后，重启场景即可看到对应变化。NPC 仍只使用自己的配置和个人 Observation。
- **配置形式**：使用本地 JSON 作为受控预设的导入/导出格式。AI 可以生成候选 JSON，但 UE 必须在加载前校验 Schema 版本、字段白名单、稳定 ID 唯一性、角色数量和数值范围；校验失败不得改变当前场景。导出只包含可配置的公开字段，不包含 Prompt、服务响应、密钥或运行时个人历史。
- **最小实现边界**：配置只覆盖稳定 ID、类型、显示名、颜色、出生 Transform、初始生命，以及 NPC 已有的人物、表达风格、目标和初始关系/即时状态；继续使用当前 Pawn、NPC 类和占位表现资产。
- **战斗表现验证**：选择性复用 `Variant_Combat` 的攻击/受击动画通知或展示组件。只有既有社会沙盒攻击校验和伤害入口成功后才播放一次攻击与受击表现；拒绝攻击不播放命中表现，且攻击不改变玩家镜头朝向。
- **完成标志**：至少两套受控预设可在同一地图切换，玩家与 NPC 的可见属性和 NPC Decision Context 均随配置变化；Stub 路径、离线降级和 UE Target 编译保持通过。
- **明确不做**：不做角色捏脸、运行时自由创建、AI 自动写入或自动应用配置、持久化、背包、装备、职业树、资产导入、动画重定向、完整 Variant_Combat 接入、连招/蓄力/敌人刷新、复杂战斗、多人同步或任何协议/Tool 扩展。

### 后续扩展

- AI Boss Director。
- 谣言可信度、多源确认和事件失真。
- 社会 Memory 的 SaveGame 持久化。
- 语义检索和摘要。
- Multiplayer Server Authority。
- 玩家离场后的信息传播场景。
- 跨时间承诺与延迟后果场景。
- 多 NPC 共同目标与群体协作场景。

## 后续实施节奏

后续不再按后台模块或固定周数切分，而按可操作纵向切片推进：

| 顺序 | 里程碑 | 玩家当场能做什么 | 玩家当场能看到什么 |
| --- | --- | --- | --- |
| 1 | Milestone 7 | 移动、转向、输入说话/行为、选择音量与目标 | NPC 朝向、感知结果和气泡差异 |
| 2 | Milestone 8 | 与一个关键 NPC 自由互动 | LLM 自然语言、受控移动和拒绝反馈 |
| 3 | Milestone 9 | 连续改变距离、态度和冲突程度 | NPC 多轮重新判断、升级与缓和 |
| 4 | Milestone 10 | 与 3 至 5 个 NPC 开放互动 | 个人视角、人物差异、旁观反应和完整调试证据 |
| 5 | Milestone 11 | 从少量预设选择玩家角色，体验配置驱动 NPC | 名称、颜色、出生位置、人物与初始关系随配置变化 |

如果必须压缩范围，优先减少 NPC 数量、动作种类和表现资产，不删除玩家输入、可见反馈、个人视角约束或真实 LLM 纵向闭环。

## Git 变更规划

每个提交只表达一个逻辑变更，并包含适用测试和文档。只有用户明确要求后才创建 commit。

```text
docs(docs): define social simulation roadmap
feat(ue): add gameplay event routing
feat(ue): add spatial event propagation
feat(ue): add personality and instant state
feat(ue): add bounded social memory
feat(ue): add mass npc rule decisions
feat(ue): add relationship propagation
docs(protocol): define decision contract
feat(python): add structured decision planner
feat(ue): add interactive social sandbox shell
feat(ue): add directional sight and speech perception
feat(ue): add speech and action bubbles
feat(ue): add validated embodied tool execution
feat(ue): integrate continuous core npc decisions
feat(ue): add runtime decision debugger
test(ue): validate open social sandbox scenarios
docs(docs): record final validation and demo guide
```

`docs(protocol)` 及后续 ToolCall 实现必须等待用户对协议方案的明确确认。

## 规划执行规则

- 同一时间只有一个 `CurrentMilestone.md`。
- 未来能力可以保留在本文，但不得提前加入当前 Task Board。
- 切换里程碑前必须归档已完成的当前范围。
- 每项验收标准使用稳定 ID，证据只写入对应 Validation 文档。
- 如果实现证明目标不合理，应先更新本文的取舍或顺序，再修改后续里程碑；不得用 ProjectState 反向改写规划。
