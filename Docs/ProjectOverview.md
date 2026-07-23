# Project Overview

## 项目定位

本项目是一个用于高级 UE Gameplay / AI 技术岗位展示的 **UE5 + LLM AI NPC Runtime Demo**。

长期目标是让 NPC 基于以下信息生成符合情境的对话，并通过 Tool Call 驱动 UE Gameplay 行为：

- 玩家历史行为
- 当前世界状态
- NPC 人格与目标
- 对话上下文

## 技术栈

- Client：Unreal Engine 5、C++
- AI Service：Python、FastAPI
- Data：SQLite
- 可选扩展：Chroma 或 FAISS
- LLM：首先接入 Kimi 开放平台 Chat Completions API；模型由 Python Service 配置，UE 与协议不绑定具体模型

## 核心原则

- UE 负责 Gameplay 状态、表现和行为执行。
- Python Service 负责 AI 推理编排，不直接修改 UE 世界。
- UE 与 Service 只通过版本化 JSON 协议通信。
- 每个里程碑保持可运行、可验证，避免提前引入未被当前需求证明的复杂度。
- 新增模块不得绕过既定边界直接依赖其他模块内部实现。

## 演进路线

1. **最小通信闭环**：UE 请求 Python Service，并展示固定或规则生成的回复。
2. **真实 LLM 对话**：接入模型，加入超时、错误处理和配置管理。
3. **NPC 上下文与人格**：传递世界状态、人格和有限对话上下文。
4. **Memory**：使用 SQLite 保存结构化历史，按需评估向量检索。
5. **Tool Use**：模型返回受约束的 Tool Call，由 UE 校验并执行 Gameplay 行为。

## 当前状态

当前执行状态见 [ProjectState.md](./ProjectState.md)。当前范围、验收标准和任务拆分分别见 [CurrentMilestone.md](./CurrentMilestone.md) 与 [TaskBoard.md](./TaskBoard.md)。
