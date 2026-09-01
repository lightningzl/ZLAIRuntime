# Project Overview

## 文档职责

本文档只维护项目定位、长期目标、技术栈和高层演进路线。详细路线见 [SocialSimulationPlan.md](./SocialSimulationPlan.md)，目标技术设计见 [SocialSimulationDesign.md](./SocialSimulationDesign.md)，当前执行状态见 [ProjectState.md](../Current/ProjectState.md)。

## 项目定位

**ZLAI Social Simulation Runtime** 是用于高级 UE Gameplay / AI 技术岗位展示的 UE5 + Python AI Runtime Demo。

它不是以聊天为中心的 AI NPC 项目。核心目标是通过少量具身 NPC 的开放交互，展示玩家近乎无限的语言和有限但真实执行的行为，如何经过定向感知、人物上下文、记忆、关系和分层决策形成连续、可见的社会反馈。100+ NPC 的确定性模拟保留为独立性能证明，不替代最终场景的交互质量。

目标架构中，LLM 只负责少量关键 NPC 的 reasoning、dialogue 和 high-level decision；UE 始终负责世界事实、移动、动画、伤害、生成和行为执行。

## 技术栈

- Client：Unreal Engine 5、C++、Gameplay Tags、StateTree；目标社会模拟使用 MassEntity。
- AI Service：Python、FastAPI。
- Dialogue Data：SQLite。
- LLM：当前接入 Kimi Chat Completions；Provider 和模型由 Python 配置，不进入 UE 协议。
- 可选后续扩展：语义检索、AI Boss Director、多人 Server Authority。

## 核心原则

- UE 是 Gameplay 和社会模拟的唯一事实来源。
- Python Service 负责 AI 推理编排，不直接访问或修改 UE World。
- UE 与 Service 只通过版本化 JSON 协议通信。
- 目标系统允许 AI 返回 Decision、Dialogue 或受约束 ToolCall；当前已实现路径只返回 Dialogue，任何未来 Gameplay 建议都必须由 UE 验证后才可能执行。
- Level 1 NPC 不调用 LLM，所有层级都有确定性降级路径。
- 最终场景先定义玩家体验，再倒推领域模型、协议与实现；技术能力清单不能代替可玩的场景闭环。
- 每个里程碑必须交付玩家能实际操作、屏幕上能直接观察、可独立验收的纵向切片，避免只完成后台类型、接口或无界面测试。

## 演进路线

1. **已完成：通信与真实对话**——UE/Python 闭环、Kimi、错误与超时。
2. **已完成：上下文与对话 Memory**——NPC/World 快照和 SQLite 对话历史。
3. **已完成：确定性社会模拟基础**——Event、空间查询、感知、人格、Instant State、Short Memory 和规则决策。
4. **已完成：关系与重要 NPC**——Long-Term State、Relationship、Faction、Long Memory 和有界事件链。
5. **已完成：可操作交互舞台与定向感知**——玩家能移动、输入说话或行为，并直接观察 NPC 的个人感知与气泡反馈。
6. **已完成：单 NPC LLM 具身反馈**——一个关键 NPC 根据个人视角生成语言和受控动作。
7. **已完成：连续互动与冲突变化**——NPC 随距离、行为、道歉、威胁和冲突结果持续重新判断。
8. **已完成：最终场景 1 交付**——4 个差异化 NPC 完成开放式社会交互沙盒 `FS1-01` 至 `FS1-11` 验收。
9. **后续候选扩展**——NPC 间协作、跨时间后果、社会 Memory 持久化、语义检索或多人 Server Authority，进入新里程碑前另行定义范围。

## 文档入口

- 当前范围：[CurrentMilestone.md](../Current/CurrentMilestone.md)
- 当前任务：[TaskBoard.md](../Current/TaskBoard.md)
- 当前状态：[ProjectState.md](../Current/ProjectState.md)
- 当前架构：[Architecture.md](./Architecture.md)
- 通信协议：[Protocol.md](../Reference/Protocol.md)
- 数据库设计：[DatabaseDesign.md](../Reference/DatabaseDesign.md)
- 面试资料：[SocialSimulationInterviewGuide.md](../Interview/SocialSimulationInterviewGuide.md)
