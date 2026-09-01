# Milestone 8 Validation

## 状态

- 里程碑：Milestone 8——单 NPC LLM 具身反馈与受控动作
- 当前阶段：本地实现与回归完成；真实 Kimi 外发验收待明确授权
- 最后更新：2026-09-01
- 结论：本地实现、Stub 纵向闭环和全量回归完成；`M8-A05` 的真实 Kimi 场景调用因外部数据发送未获专项授权而未执行，其余验收均有实际证据

验收标准正文见 [CurrentMilestone.md](../Current/CurrentMilestone.md)。本文只记录实际执行过的验证，不保存完整玩家输入、Prompt、Memory scope、回复正文、凭据或原始 Provider 异常。

## 基线环境

- 分支：`codex/milestone-8`
- Python：3.12.13；pytest 9.1.1
- 当前协议：`POST /v1/dialogue` 保持纯文本兼容；`POST /v1/decision` 已确认并同步两端契约类型
- UE：Unreal Engine 5.8.1，Win64 Development Editor，NullRHI 自动化

## 已执行验证

| 范围 | 方法 | 结果 | 证据 |
| --- | --- | --- | --- |
| Python 全量基线 | 在 `PythonService` 执行 `.venv/Scripts/python -m pytest` | 通过 | 收集 166 项，166 项通过，耗时 1.76 秒 |
| 当前分支与工作区 | 检查分支、提交和工作区状态 | 通过 | 位于 `codex/milestone-8`；范围文档提交为 `2905c67`；检查时无未提交实现改动 |
| 协议门禁 | 检查 `Protocol.md` 与当前实现入口 | 通过 | 当前分支未修改协议；Python 只注册 `/v1/dialogue`，UE 只构造 `/v1/dialogue` Endpoint |
| Python 实现边界 | 检查 Route、Schema、Service 与 Provider 注入入口 | 通过 | 当前为 Dialogue 专用路径；Decision 需要并列 Route/Schema/Service/Planner，不应让 Route 直接依赖 Provider SDK |
| UE 实现边界 | 检查 Service Subsystem、协议类型、Sandbox GameMode/NPC 与移动实现 | 通过 | Dialogue 请求已有一次完成和错误解析基线；现有 Sandbox 移动只控制玩家，NPC 具身动作需要独立权威 Handler |
| Python Decision Schema | 执行 Decision Schema、Dialogue API 与 Provider 错误相关测试 | 通过 | 收集 31 项，31 项通过；其中新增 Decision 契约测试 14 项 |
| Python 全量协议回归 | 在 `PythonService` 执行 `.venv/Scripts/python -m pytest` | 通过 | 收集 180 项，180 项通过，耗时 1.48 秒 |
| Python Decision Planner 与全量回归 | 在 `PythonService` 执行 `.venv/Scripts/python -m pytest` | 通过 | 收集 193 项，193 项通过，耗时 1.58 秒；覆盖 Route、Context Builder、Service、Stub、Kimi JSON 映射、非法 Tool/目标、空白业务数据和超时分类 |
| UE Target 编译 | 执行 `Build.bat ZLEditor Win64 Development` | 通过 | UHT、Decision 协议类型、序列化/解析和新增测试编译链接成功 |
| UE Decision/Dialogue 契约 | NullRHI 执行 `Automation RunTests ZLAIRuntime.Protocol` | 通过 | 收集 9 项，9 项全部 `Success`；包含 3 项 Decision 与 6 项 Dialogue 回归，退出码 0 |
| UE Decision Client 编译与完成语义 | 编译 Target 后 NullRHI 执行 `Automation RunTests ZLAIRuntime.DecisionClient` | 通过 | 1 项通过；覆盖成功关联、30 秒本地 TTL 过期和状态版本不匹配，均只走一次完成路径 |
| UE 单 NPC 个人上下文 | NullRHI 执行 `Automation RunTests ZL.Social.Sandbox.PersonalDecisionContext` | 通过 | 1 项通过；只保留选定 NPC 的已感知 Trigger/历史，过滤其他 Observer，未听见 Speech 不能构造请求 |
| UE Tool Registry 编译与规则验收 | 编译 Target 后 NullRHI 执行 `Automation RunTests ZL.Social.ToolRegistry.Validation` | 通过 | 1 项通过，退出码 0；覆盖四 Tool 白名单、Capability、目标、状态版本、TTL、距离、导航、可执行状态、冷却、速率、幂等、拒绝不提交和 128 条有界 Call ID 缓存 |
| NPC Decision Handler | NullRHI 执行 `Automation RunTests ZL.Social.Sandbox.NpcDecisionAction` | 通过 | 1 项通过，退出码 0；覆盖无效 Face 零状态副作用、合法 Face 立即完成、Authority Version 推进、MoveAway 真实 Transform 变化和 Stop 清理 |
| Stub 场景纵向闭环 | Stub Service + 默认沙盒地图 + `-ZLSandboxDecisionSmoke` | 通过 | 正常 Demo 提交路径返回 `provider=stub`，Speech 接受，`move_away` 校验接受，Guard 版本与位置改变，耗时 10 ms |
| 请求期间状态变化 | Stub Service + 默认沙盒地图 + `-ZLSandboxDecisionStaleSmoke` | 通过 | Speech 接受；Tool 以 `StateVersionMismatch` 拒绝；版本按测试推进但位置不变，证明表达与 Tool 拒绝独立且零 Tool Transform 副作用 |
| 服务离线降级 | 端口 8000 无监听时运行默认沙盒地图 + `-ZLSandboxDecisionFallbackSmoke` | 通过 | 连接失败映射为 `provider=local` / `network_error`，Guard 版本与位置均不变，耗时 2042 ms |
| 真实 Kimi 场景闭环 | 检测本机配置后尝试启动 Kimi Service 并运行默认沙盒地图 | 未执行 | 外部数据发送安全门禁拒绝执行；未发出请求、未泄露凭据或 Decision 内容。需要用户另行明确授权该外发后才能补充人工证据 |
| Python 最终全量回归 | 在 `PythonService` 执行 `.venv/Scripts/python -m pytest -c pyproject.toml` | 通过 | 收集 193 项，193 项通过，耗时 1.58 秒 |
| UE 最终 Target 编译 | 执行 `Build.bat ZLEditor Win64 Development` | 通过 | UHT、ZLAIRuntime、ZLASocialRuntime、ZL 全部编译链接成功，UnrealBuildTool `Result: Succeeded` |
| UE Social 最终全量回归 | NullRHI + 内存 DDC 执行 `Automation RunTests ZL.Social` | 通过 | 收集 24 项，24 项全部 `Success`，包含 Tool Registry、个人上下文和 NPC Decision Action，退出码 0 |
| UE AI Runtime 最终全量回归 | Stub Service 在线时，NullRHI + 内存 DDC 执行 `Automation RunTests ZLAIRuntime` | 通过 | 收集 13 项，13 项全部 `Success`，包含 Dialogue/Decision 协议、Client、失败分类和真实 HTTP 回调，退出码 0 |

## 基线结论

- Python 最终回归已覆盖 Dialogue、Memory、Decision Schema、Context Builder、Service、Stub/Kimi Planner 映射与错误分类；真实 Kimi 网络调用不属于离线测试证据。
- `UZLAIServiceSubsystem` 的 Dialogue/Decision 路径均已验证 HTTP 回调、超时分类、Game Thread 交付、请求关联和一次完成语义；既有 Dialogue API 保持不变。
- `AZLSocialSandboxGameMode::SubmitSpeech` 已逐 NPC 生成独立 Observation；里程碑 8 只能从选定 NPC 的 Observation 构造请求，不得把全体接收结果作为其个人知识。
- `AZLSocialSandboxPawn` 当前实现 Face、Approach、MoveAway 和 Stop，但它只代表玩家。NPC Tool Handler 必须在 `AZLSocialSandboxNpc` 或 `ZL` 的独立适配器中执行，并由通用 Registry 校验后调用。
- Decision 协议已在用户明确确认后同步；现有 Dialogue 字段和纯文本语义未改变，也没有从 Dialogue 文本推断 Gameplay 动作。
- `FZLSocialToolRegistry` 已完成纯规则权威校验并只在接受后提交幂等与冷却状态；具体 NPC 世界副作用仍由下一工作包接入和验证。

## 验收进度

| ID | 当前证据 | 状态 |
| --- | --- | --- |
| `M8-A01` | Python 193/193、UE Social 24/24、UE AI Runtime 13/13 全部通过；ZLEditor Win64 Development 最终编译成功 | `通过` |
| `M8-A02` | 用户已明确确认；`Protocol.md`、Python Schema、UE 类型及两端契约测试一致，Python 14 项与 UE 3 项 Decision 契约测试通过 | `通过` |
| `M8-A03` | Python Context Builder 与 UE Gameplay Builder 只包含 Guard 已感知 Trigger、人物/关系/即时状态/个人历史和允许 Tool；Stub 场景实际发送通过 | `通过` |
| `M8-A04` | Stub 经默认 Demo/UI 提交路径返回 Speech 与 `move_away`；Guard 真实移动、状态版本推进并产生动作结果 Observation | `通过` |
| `M8-A05` | Kimi Planner 的离线 JSON 映射、结构校验和异常分类通过；真实模型人工闭环待执行 | `部分` |
| `M8-A08` | Python 覆盖无效 Planner/Provider 超时；UE 离线场景显示 `LocalFallback`，错误码脱敏且 Authority State/位置不变 | `通过` |
| `M8-A07` | UE Client 覆盖 TTL/响应关联；场景状态变化证明 Speech 保留、Tool 以 `StateVersionMismatch` 拒绝且位置不变 | `通过` |
| `M8-A06` | Registry 全规则测试和场景执行共同证明合法动作才改变 Transform；拒绝有稳定 Reason Code，Inspector 可见 | `通过` |
| `M8-A09` | Context/协议/历史/字符串/Tool/Call ID 均有硬上限；场景固定单在途请求、4 次执行窗口和单项安全调试快照 | `通过` |
| `M8-A10` | 模块依赖保持 `ZL -> ZLAIRuntime/ZLASocialRuntime`；命名、边界、公开 Reason Code、文档与最终编译回归一致 | `通过` |

后续每次工作包验证只追加实际命令、环境、结果和与验收 ID 的对应关系；未执行项目不得写成通过。
