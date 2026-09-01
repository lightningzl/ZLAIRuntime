# ZLAI Social Simulation Runtime 面试展示指南

## 文档职责

本文档只服务面试展示、讲解顺序和问答复习，不定义系统架构、协议、当前范围或完成状态。系统事实应链接到 [总规划](../Planning/SocialSimulationPlan.md)、[总设计](../Planning/SocialSimulationDesign.md) 和 [当前里程碑](../Current/CurrentMilestone.md)。

## 核心叙事

本项目不是 AI Chat NPC。展示重点是玩家行为如何进入一个可验证的社会模拟，并最终影响 NPC 的感知、状态、记忆、关系和行为。

一句话定位：

> 不确定的 AI 推理只能提出高层意图，确定性的 UE Runtime 始终掌握 Gameplay 事实与最终执行权。

## 10 分钟展示流程

以下流程以已完成的 Milestone 10 为主线；100+ NPC 确定性基准与 4 NPC 真实 LLM 场景分别展示，不声称它们在同一地图同时运行。当前事实见 [ProjectState.md](../Current/ProjectState.md)、[Architecture.md](../Planning/Architecture.md) 和 [Milestone10Validation.md](../Validation/Milestone10Validation.md)。

| 时间 | 展示内容 | 讲解重点 |
| --- | --- | --- |
| 0:00–0:45 | 总体架构图 | UE 权威；Kimi 每次只消费一个 NPC 的个人 Context |
| 0:45–1:30 | 默认社会沙盒与 4 NPC | Guard、Merchant、Rival、Civilian 的人物、关系和可见差异 |
| 1:30–2:30 | Whisper、Talk、Shout、InEar | 定向视觉、分级听觉、明确目标与无目标判断 |
| 2:30–3:40 | 无目标开放语言 | 只有实际听清的 NPC 进入个人 Decision；旁观者不获得上帝视角 |
| 3:40–5:00 | 同一句威胁分别指向 4 NPC | Kimi/Stub 的人物差异、Rival 既有矛盾与 Civilian 回避倾向 |
| 5:00–6:10 | NPC Speech 与移动 | 结构化 Intent、可选 Speech、单 Tool 建议和真实动作落地 |
| 6:10–7:15 | 靠近、退让与 Attack | 连续重新判断、生命/防卫、冲突升级和缓和都由 UE 维护 |
| 7:15–8:15 | 个人 Inspector | Trigger、感知来源、状态版本、Provider、Intent、Tool 结果与延迟 |
| 8:15–9:10 | 非法/过期 Tool 或 Service 离线 | UE 零副作用拒绝，NPC 显示有界本地降级 |
| 9:10–10:00 | 120+5 独立基准与回归证据 | 空间查询、事件预算、全局 LLM 并发 2、198+28+13 项回归 |

## 展示时必须强调

- Event 到达后 UE 立即更新状态，不等待 LLM。
- Level 1 永远不调用 LLM。
- Important NPC 只有在高价值决策点才调用 AI。
- 4 NPC 共享协议和执行器，但不共享 Observation、隐藏历史或请求；全局最多 2 个 Decision 在途。
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
