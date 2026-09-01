# Milestone 10：最终场景 1 多 NPC 交付

## 历史声明

本文档定稿 Milestone 10 的已完成范围。当前状态与后续任务以 [ProjectState.md](../Current/ProjectState.md) 和 [TaskBoard.md](../Current/TaskBoard.md) 为准；实际证据见 [Milestone10Validation.md](../Validation/Milestone10Validation.md)。

## 完成范围

- 将社会沙盒固定为 Guard、Merchant、Rival、Civilian 4 个差异化 NPC，分别配置身份、人物、表达风格、目标、初始关系/状态和可见标识。
- 每个 NPC 独立维护 Observation、公开历史、Authority State Version、生命、防卫、冲突状态、Decision 调度、Tool 执行窗口和 Inspector 快照。
- 玩家有目标或无目标的 Speech、已完成 Action、NPC 动作和距离跨带只在对应 NPC 实际听见/看见后进入个人 Decision。
- UE 组合逐 NPC 单在途/单 Pending Scheduler，并把全局 Decision 并发固定为 2；回调按 NPC、请求代次、状态版本和 TTL 关联。
- Stub 根据 4 种人物和关系产生可复现差异；Kimi 只使用当前 NPC 的个人 Context，禁止跨 NPC 推断。
- 现有 FaceTarget、MoveToward、MoveAway、Stop 经统一 UE Registry 校验后可由任意 NPC 独立执行；攻击、命中、伤害和防卫继续由 UE 权威维护。
- 默认地图提供 4 NPC 气泡、移动、生命/立场、目标切换和个人 Inspector；新增多 NPC Stub 烟测并保留离线与真实 Kimi 纵向验收。

## 协议与取舍

- 沿用 Decision v1，没有修改 [Protocol.md](../Reference/Protocol.md)、Endpoint、字段、Intent、Tool 或兼容规则。
- 不包含 NPC 间自由对话、无限自主循环、完整战斗/导航/动画、MassEntity 正式场景集成、Embedding、多人或 Server Authority。
- 100+ NPC 确定性性能继续由独立基准证明，不与 4 NPC 真实 LLM 场景混测。

## 验收结论

- `M10-A01` 至 `M10-A11`：11/11 通过。
- 最终场景 1 `FS1-01` 至 `FS1-11`：11/11 通过。
- Python 198/198、UE Social 28/28、UE AI Runtime 13/13、ZLEditor 编译、4 NPC Stub、离线降级和真实 Kimi 均通过。
