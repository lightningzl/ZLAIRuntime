# ZLAI Social Simulation Runtime 面试展示指南

## 文档职责

本文档只服务面试展示、讲解顺序和问答复习，不定义系统架构、协议、当前范围或完成状态。系统事实应链接到 [总规划](../Planning/SocialSimulationPlan.md)、[总设计](../Planning/SocialSimulationDesign.md) 和 [当前里程碑](../Current/CurrentMilestone.md)。

## 核心叙事

本项目不是 AI Chat NPC。展示重点是玩家行为如何进入一个可验证的社会模拟，并最终影响 NPC 的感知、状态、记忆、关系和行为。

一句话定位：

> 不确定的 AI 推理只能提出高层意图，确定性的 UE Runtime 始终掌握 Gameplay 事实与最终执行权。

## 10 分钟展示流程

以下内容是最终目标演示脚本，不代表 Milestone 5 已具备全部展示能力。当前已实现能力以 [ProjectState.md](../Current/ProjectState.md) 和 [Architecture.md](../Planning/Architecture.md) 为准；尚未实现的 Mass、事件二级传播、Relationship、AI Decision、ToolCall 和完整 Runtime Debugger 不得在当前演示中表述为已完成。

| 时间 | 展示内容 | 讲解重点 |
| --- | --- | --- |
| 0:00–0:45 | 总体架构图 | UE 权威；LLM 只做高层 Decision/Dialogue |
| 0:45–1:30 | 平静市场和 100+ NPC | Mass/LOD、人格分布、没有每帧全量事件扫描 |
| 1:30–3:20 | 玩家 Punch | 同一事件产生围观、逃跑、报警、协助和愤怒 |
| 3:20–4:20 | Event Graph | Root ID、空间过滤、传播深度、预算和去重 |
| 4:20–5:20 | Gunshot | 大范围反应、事件优先级和安全覆盖规则 |
| 5:20–6:20 | Help 或 Steal | Trust、Relationship、Memory 与后续行为变化 |
| 6:20–7:40 | Important/Core NPC | 规则候选、AI Decision、可选 Speech 与 ToolCall |
| 7:40–8:30 | UE Tool 校验 | Capability、参数、目标、距离、冷却和状态版本 |
| 8:30–9:15 | 非法/过期 Tool 或 Service 离线 | UE 拒绝执行，NPC 使用规则降级 |
| 9:15–10:00 | Debugger 与性能 | 事件查询量、决策来源、延迟、失败和可复现验证 |

## 展示时必须强调

- Event 到达后 UE 立即更新状态，不等待 LLM。
- Level 1 永远不调用 LLM。
- Important NPC 只有在高价值决策点才调用 AI。
- Python 不读取 UE World，也不能直接造成 Damage、Movement 或 Spawn。
- ToolCall 是建议，不是命令。
- Runtime Debugger 显示稳定 Reason Code，不展示 Chain-of-Thought。

## 面试问题与回答方向

| 问题 | 回答重点 |
| --- | --- |
| 为什么 UE 不直接调用 LLM？ | 隔离 SDK、密钥、Prompt 和网络失败；保持 Gameplay 稳定 |
| 为什么使用三级 NPC？ | 将成本和不确定性集中到少量有叙事价值的角色 |
| 为什么不让模型控制移动和战斗？ | 延迟、不可复现和安全；模型只选择高层 Intent |
| 100+ NPC 如何保证性能？ | 事件驱动、空间索引、Mass LOD、降频和有界预算 |
| 如何避免事件无限传播？ | Root ID、Depth、TTL、Budget、Fan-out 和去重 |
| Personality 如何保持一致？ | 固定 Trait、Utility、硬约束、迟滞和确定性平局 |
| Memory 为什么不会无限增长？ | 固定容量、Importance、Decay、Promotion 和 Top-K |
| Relationship 谁维护？ | UE 是唯一权威；Python只消费快照 |
| LLM 超时怎么办？ | 使用规则第一候选降级，社会状态更新不受影响 |
| 回复回来时世界变了怎么办？ | 状态版本、Expiry 和目标重新校验 |
| 如何防止 Prompt Injection？ | 输入和 Memory 都作为不可信数据；Tool 白名单由 UE 提供 |
| ToolCall 如何保证安全？ | Schema、Capability、Target、Range、Cooldown、Authority、幂等 |
| 为什么单独增加 Decision Endpoint？ | 不破坏已验证 Dialogue 路径，并隔离行为建议风险 |
| 为什么不用向量数据库？ | MVP 数据量小；结构化、有界检索更可预测、可测试 |
| StateTree 与 BehaviorTree 如何选择？ | 决策层不绑定；Mass/反应用 StateTree，复杂 Actor 可按需要选择 |
| 多人如何扩展？ | Event、State 和 Tool Execution 迁移到 Server Authority |
| 最大的工程取舍是什么？ | 用一个小场景证明完整闭环，而不是建设无边界开放世界 |

## 失败演示预案

- 真实 Provider 不稳定时切换到确定性 Stub，明确说明 Stub 只验证编排和执行。
- 若无法实时完成 LLM 请求，展示已录制的脱敏请求关联与 Debugger 数据，但不要伪装为实时结果。
- 性能演示记录固定硬件、地图、NPC 数量和采样方式，不使用无环境说明的单一 FPS 数字。

## 收尾表达

> 这个 Demo 的价值不在于给模型更多控制权，而在于证明生成式 AI 可以安全、可观察、可降级地参与 UE Gameplay Runtime。
