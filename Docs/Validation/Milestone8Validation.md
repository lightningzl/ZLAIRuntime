# Milestone 8 Validation

## 状态

- 里程碑：Milestone 8——单 NPC LLM 具身反馈与受控动作
- 当前阶段：协议确认前基线
- 最后更新：2026-08-31
- 结论：未完成；当前只记录已实际执行的基线检查，不声明 `M8-A01` 至 `M8-A10` 通过

验收标准正文见 [CurrentMilestone.md](../Current/CurrentMilestone.md)。本文只记录实际执行过的验证，不保存完整玩家输入、Prompt、Memory scope、回复正文、凭据或原始 Provider 异常。

## 基线环境

- 分支：`codex/milestone-8`
- Python：3.12.13；pytest 9.1.1
- 当前协议：仅有 `POST /v1/dialogue` 纯文本契约；Decision 协议尚未确认或实现
- UE 基线：沿用 Milestone 7 已归档的 Unreal Engine 5.8.1 Win64 Development Editor 环境，本阶段尚未重新编译或运行 UE 自动化

## 已执行验证

| 范围 | 方法 | 结果 | 证据 |
| --- | --- | --- | --- |
| Python 全量基线 | 在 `PythonService` 执行 `.venv/Scripts/python -m pytest` | 通过 | 收集 166 项，166 项通过，耗时 1.76 秒 |
| 当前分支与工作区 | 检查分支、提交和工作区状态 | 通过 | 位于 `codex/milestone-8`；范围文档提交为 `2905c67`；检查时无未提交实现改动 |
| 协议门禁 | 检查 `Protocol.md` 与当前实现入口 | 通过 | 当前分支未修改协议；Python 只注册 `/v1/dialogue`，UE 只构造 `/v1/dialogue` Endpoint |
| Python 实现边界 | 检查 Route、Schema、Service 与 Provider 注入入口 | 通过 | 当前为 Dialogue 专用路径；Decision 需要并列 Route/Schema/Service/Planner，不应让 Route 直接依赖 Provider SDK |
| UE 实现边界 | 检查 Service Subsystem、协议类型、Sandbox GameMode/NPC 与移动实现 | 通过 | Dialogue 请求已有一次完成和错误解析基线；现有 Sandbox 移动只控制玩家，NPC 具身动作需要独立权威 Handler |

## 基线结论

- Python 现有回归可作为后续 `M8-A01` 的对照基线，但尚未覆盖 Decision Schema、Planner 或错误映射。
- `UZLAIServiceSubsystem` 已提供 HTTP 超时、Game Thread 回调、请求/响应 ID 对照和一次完成语义；Decision 应复用行为模式而不改变既有 Dialogue API。
- `AZLSocialSandboxGameMode::SubmitSpeech` 已逐 NPC 生成独立 Observation；里程碑 8 只能从选定 NPC 的 Observation 构造请求，不得把全体接收结果作为其个人知识。
- `AZLSocialSandboxPawn` 当前实现 Face、Approach、MoveAway 和 Stop，但它只代表玩家。NPC Tool Handler 必须在 `AZLSocialSandboxNpc` 或 `ZL` 的独立适配器中执行，并由通用 Registry 校验后调用。
- 协议确认前没有修改 `Protocol.md`、没有新增 Decision JSON 类型、没有从 Dialogue 文本推断 Gameplay 动作。

## 验收进度

| ID | 当前证据 | 状态 |
| --- | --- | --- |
| `M8-A01` | Python 基线 166 项通过；UE 编译与自动化尚待执行 | `部分` |
| `M8-A02` | 已验证现有协议未改变；Decision 方案仍待用户明确确认 | `待确认` |
| `M8-A03` 至 `M8-A10` | 尚无实现或完整验证证据 | `未开始` |

后续每次工作包验证只追加实际命令、环境、结果和与验收 ID 的对应关系；未执行项目不得写成通过。
