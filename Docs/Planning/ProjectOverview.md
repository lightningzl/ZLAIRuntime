# Project Overview

## 文档职责

本文档只维护项目定位、长期目标、技术栈和高层演进路线。详细路线见 [SocialSimulationPlan.md](./SocialSimulationPlan.md)，目标技术设计见 [SocialSimulationDesign.md](./SocialSimulationDesign.md)，当前执行状态见 [ProjectState.md](../Current/ProjectState.md)。

## 项目定位

**ZLAI Social Simulation Runtime** 是用于高级 UE Gameplay / AI 技术岗位展示的 UE5 + Python AI Runtime Demo。

它不是以聊天为中心的 AI NPC 项目。核心目标是展示玩家行为如何通过 Gameplay Event、空间传播、NPC 感知、状态、记忆、人格、关系和分层决策改变一个小型社会系统。

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
- 每个里程碑保持可运行、可验证，避免提前引入未被当前需求证明的复杂度。

## 演进路线

1. **已完成：通信与真实对话**——UE/Python 闭环、Kimi、错误与超时。
2. **已完成：上下文与对话 Memory**——NPC/World 快照和 SQLite 对话历史。
3. **已完成：确定性社会模拟基础**——Event、空间查询、感知、人格、Instant State、Short Memory 和规则决策。
4. **已完成：关系与重要 NPC**——Long-Term State、Relationship、Faction、Long Memory 和有界事件链。
5. **下一阶段：AI Decision 与 ToolCall**——经协议确认后接入 Level 2/3。
6. **交付：Core NPC、Debugger 与面试场景**。

## 文档入口

- 当前范围：[CurrentMilestone.md](../Current/CurrentMilestone.md)
- 当前任务：[TaskBoard.md](../Current/TaskBoard.md)
- 当前状态：[ProjectState.md](../Current/ProjectState.md)
- 当前架构：[Architecture.md](./Architecture.md)
- 通信协议：[Protocol.md](../Reference/Protocol.md)
- 数据库设计：[DatabaseDesign.md](../Reference/DatabaseDesign.md)
- 面试资料：[SocialSimulationInterviewGuide.md](../Interview/SocialSimulationInterviewGuide.md)
