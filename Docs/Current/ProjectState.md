# Project State

本文件只记录当前项目快照。范围、任务明细和长期路线分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-09-02 |
| 当前里程碑 | Milestone 11：可配置角色与 NPC 技术验证 |
| 里程碑状态 | `进行中` |
| 当前活动任务 | `M11-T03` 配置驱动场景生成与个人 Context |
| 下一候选任务 | `M11-T04` 受控攻击展示适配器 |
| 已知阻塞 | 无 |
| 最近验收 | Milestone 10：11/11 项及 FS1 11/11 属性已验证 |

## 当前能力基线

- Milestone 1 至 9 已完成；范围与证据见 [Milestones](../Milestones/) 和 [Validation](../Validation/)。
- UE/Python 已具有真实 Kimi Dialogue、SQLite 对话 Memory、独立 Decision v1、Kimi/Stub Planner 和脱敏失败映射。
- 纯 UE 社会模拟已具有 Event、空间查询、个人感知、状态、关系、长期记忆、规则决策、5 Important NPC 无界面切片和 120+5 基准。
- 可操作社会沙盒稳定生成 4 个 NPC，并具有 Speech/Action 分离输入、定向视觉、分级听觉、逐 NPC Observation、气泡和个人 Inspector 基础。
- 当前只有 Guard 接入 LLM Decision、四个受控移动 Tool、连续调度、基础 Attack、生命/防卫/失能、冲突升级—缓和和本地失败降级。
- 当前场景 4 个 NPC 已固定为 Guard、Merchant、Rival 和 Civilian，并分别具有身份、人物、表达风格、目标、初始关系/状态和可见颜色；个人 Decision Context 已消费选定 NPC Profile。
- 当前已接入逐 NPC Scheduler、稳定轮转和全局最多 2 个 Decision 在途；实际听清 Speech 或看见已完成 Action 的 NPC 才进入自己的有界队列，回调按 NPC 与重置代次隔离。
- 当前逐 NPC 公开历史、冲突状态和本地失败已按稳定 ID 隔离；旁观者只有看见其他 NPC 已执行动作后才重新判断，未听清或未看见的 Event 不进入 Planner。
- 当前 Stub 已按 Guard、Merchant、Rival、Civilian Profile 与关系生成 4 种可复现表达；Kimi 固定约束要求保持当前 NPC 身份、人物和个人视角，并禁止推断其他 NPC 的感知与决定。
- 当前任意 NPC 已可独立接收和执行通过 UE 校验的四个 Tool，并产生可见移动/动作与个人公开历史；每个 NPC 的距离跨带、生命、防卫、冲突状态和 Inspector 结果彼此隔离。
- 当前完整验证基线为 Python 198/198、UE `ZL.Social` 28/28、UE `ZLAIRuntime` 13/13、最终 Target 编译、4 NPC Stub 默认地图烟测和服务离线降级烟测通过。
- 真实 Kimi 已对 Guard、Merchant、Rival、Civilian 的同一威胁输入分别返回 `provider=kimi`；Intent 分为 engage/disengage，动作建议分为 face_target/move_away，Speech 长度各异且结构合法。默认 UE 地图 Kimi 烟测也已接受 Speech，未建议 Tool 时保持位置不变。
- Milestone 9 最终基线为 Python 197/197、UE `ZL.Social` 27/27、受影响 UE Target 编译、Stub/离线烟测和默认地图真实 Kimi 验收通过。
- 社会交互舞台的固定界面、状态、气泡与 Stub Speech 已统一为简体中文，Kimi Speech 指令同步约束中文；UE Target 已固定使用 UTF-8 编译，避免无 BOM 源码被按本机代码页读取而乱码。
- 玩家与 NPC 头顶名称改为屏幕空间 UMG 文本组件，使用与气泡相同的中文回退字体路径，避免 `TextRenderComponent` 的有限字形和离线材质限制。
- 玩家点击场景中的 NPC 时，控制器按 Visibility 命中该角色，并同步左侧目标选择与个人 Inspector；点击非 NPC 保持当前选择不变。
- 行为输入已改为面向、靠近、远离、攻击、停止的下拉选择，并保持既有 Action Parser 白名单；下拉按钮、展开菜单与箭头统一为深色背景、浅色文字，避免白底白字。
- 社会沙盒源码已按 Actors、Decision、Domain、UI、World、Tests 六类职责拆分，跨目录包含路径均显式声明；UE 5.8 Target 编译通过。
- 屏幕右侧独立行动记录按钮可展开日志面板：最多显示最近 12 条玩家成功提交、NPC 对话及 NPC 已接受行动，重置场景时清空，不保留隐藏上下文。
- 玩家攻击不再重置控制器视角；攻击继续沿用既有目标、距离、冷却与伤害校验。
- 当前开始 M11 配置驱动技术验证：少量本地 JSON 预设将驱动玩家与 NPC 的公开属性及既有个人 Context；UE 在应用前校验 Schema、字段白名单、稳定 ID、角色数量和数值范围。蓝图、动画和其他资源配置由用户完成，未配置时 C++ 路径必须安全回退。

## 当前执行边界

- Milestone 10 已归档为 [Milestone10.md](../Milestones/Milestone10.md)；`M10-T09` 中文化修正已完成代码、Python 与 UE 5.8 场景验证。
- 沿用 2026-08-31 已确认的 Decision v1；不修改 `Protocol.md`，不新增 Tool、Intent、字段或批量请求。
- UE 继续权威维护逐 NPC 感知、关系输入、生命、冲突、状态版本、调度和动作执行；Python 每次只返回一个 NPC 的现有结构化建议。
- `M10-T01` 至 `M10-T08` 已按依赖顺序完成并形成独立逻辑提交；交付不包含未验证的未来能力。
- M11 已将 M10 范围归档，并建立独立当前范围、任务板和验收标准；不修改协议，也不新增 Tool、Intent、字段或批量请求。
