# Milestone 10 Validation

## 文档职责

本文档只记录 Milestone 10 已实际执行的验证环境、命令、结果和验收证据。范围与验收标准见 [CurrentMilestone.md](../Current/CurrentMilestone.md)，任务状态见 [TaskBoard.md](../Current/TaskBoard.md)。

## 当前状态

- 状态：`已完成`
- 已完成：范围、协议边界、工作包与 `FS1-01` 至 `FS1-11` 验收映射审查
- 待记录：实现提交、Python/UE 自动化、构建、Stub/失败/性能场景、真实 Kimi 与现场演示证据

## 文档基线

- 2026-09-01：确认 Milestone 9 已归档，工作区开始时无未提交改动。
- 2026-09-01：确认 Milestone 10 沿用 Decision v1，不修改 `Protocol.md` 或两端协议类型。
- 2026-09-01：确认固定交付 Guard、Merchant、Rival、Civilian 4 个 NPC；逐 NPC 请求和全局最多 2 个并发均位于 UE 编排边界。
- 2026-09-01：`ZLEditor Win64 Development` 编译成功；4 NPC Profile、NPC 持有配置和个人 Context 接入通过 UHT、编译与链接。
- 2026-09-01：NullRHI 执行 `ZL.Social.Sandbox.PersonalDecisionContext`，收集 1 项并通过；验证 Rival/Guard 人物与关系不同、Observer 隔离和协议请求仍合法。
- 2026-09-01：`ZLEditor Win64 Development` 在多 NPC Scheduler、GameMode 分派和回调关联接入后编译成功。
- 2026-09-01：NullRHI 执行 `ZL.Social.Sandbox.MultiNpcDecisionBounds`，收集 1 项并通过；验证 4 NPC 注册上限、逐 NPC Pending 合并、稳定轮转、全局并发 2 和 Reset 清理。
- 2026-09-01：`ZLEditor Win64 Development` 在个人 Speech/Action 路由、旁观动作触发、逐 NPC 公开历史和冲突状态隔离接入后编译成功；全量场景回归留待 `M10-T07` 统一执行。
- 2026-09-01：Python 定向执行 Stub、Decision Context Builder 和 Kimi Planner 相关测试，12/12 通过；包含 4 NPC 相同输入产生 4 种稳定表达、Rival/Civilian Confidence 差异和 Kimi 单 NPC/旁观者约束断言。
- 2026-09-01：`ZLEditor Win64 Development` 在任意 NPC Tool Registry/Handler、动作结果、个人执行窗口、距离跨带和通用 Inspector 接入后编译成功；完整运行时回归与烟测进入 `M10-T07`。
- 2026-09-01：Python 全量回归 198/198 通过，用时 1.92 秒。
- 2026-09-01：NullRHI + 内存 DDC 执行 `Automation RunTests ZL.Social`，收集 28 项，28/28 `Success`；包含既有社会模拟、感知、冲突、Tool、个人 Context 和新增多 NPC 并发边界。
- 2026-09-01：本机隔离 Stub Service 在线时执行 `Automation RunTests ZLAIRuntime`，收集 13 项，13/13 `Success`；Dialogue/Decision 协议、Client、失败分类和真实 HTTP 回调保持通过。
- 2026-09-01：最终 `ZLEditor Win64 Development` 编译成功；UHT、ZLASocialRuntime、ZLAIRuntime、ZL 和新增烟测入口完成链接。
- 2026-09-01：默认地图运行 `-ZLSandboxMultiNpcSmoke`，结果 `Success`；生成 4 个稳定 NPC，4/4 接受 `provider=stub` Speech，采样时在途 0 且全程由硬上限 2 约束。
- 2026-09-01：本地 Service 离线且限帧运行 `-ZLSandboxDecisionFallbackSmoke`，结果 `Success`；约 2.02 秒获得 `network_error`，Guard 显示 `provider=local`，推进防卫状态版本且位置不变。

## 真实 Kimi 验收

- 本机只确认 `MOONSHOT_API_KEY` 环境变量存在；密钥值未读取、未输出、未写入文件或 Git。
- 本地 Service 在 `provider=kimi` 下对 Guard、Merchant、Rival、Civilian 使用同一句直接威胁和各自 Profile 发起 4 次独立 Decision，请求全部返回 HTTP 200 与 `provider=kimi`。
- 脱敏属性分别为：Guard `engage` + `face_target`，Speech 长度 35；Merchant `disengage` + `move_away`，长度 96；Rival `engage` + `face_target`，长度 75；Civilian `disengage` + `move_away`，长度 56；四项 Confidence 均在协议范围内。
- 结果体现身份/关系造成的高层差异，且所有 Tool 均属于请求白名单；没有记录模型正文、Prompt、密钥或原始供应商响应。
- 默认地图限帧运行 `-ZLSandboxDecisionKimiSmoke`，结果 `Success`：`provider=kimi`、Speech 接受、无 Tool 时 `NoTool`、延迟约 2500 ms、Guard 位置不变。该结果证明真实模型经过 UE 正常感知、HTTP、解析和 Gameplay 校验入口，而非只测试 Python SDK。

## 验收映射

| 验收 | 证据结论 |
| --- | --- |
| `M10-A01` | 协议文件和两端 Schema 未修改；Python 198/198、UE Social 28/28、UE AI Runtime 13/13 通过。 |
| `M10-A02` | 默认地图稳定生成 4 个 Profile 不同的 NPC；状态、历史、冲突、调试和重置按 ID 隔离。 |
| `M10-A03` | 既有 Speech/Action 感知回归通过；明确目标可按 Direct 听见触发，旁观者必须听清或看见才调度。 |
| `M10-A04` | PersonalDecisionContext 与 MultiNpcDecisionBounds 通过；逐 NPC 单在途/单 Pending，全局并发 2。 |
| `M10-A05` | Stub 对同一输入产生 4 种稳定表达；真实 Kimi 产生两类 Intent/Tool 组合与不同长度表达。 |
| `M10-A06` | 4 NPC 共用 UE Tool Registry 硬校验但各自执行；非法、过期、失能和速率边界由回归覆盖。 |
| `M10-A07` | 任意 NPC Attack/生命/防卫/冲突隔离完成；既有升级—缓和自动化保持通过。 |
| `M10-A08` | 任意 NPC Inspector、气泡、生命/立场和动作状态已接入默认地图。 |
| `M10-A09` | 4 NPC Stub 烟测与离线降级烟测通过，请求并发和自动重规划有硬上限。 |
| `M10-A10` | 下方 `FS1-01` 至 `FS1-11` 全部通过；真实模型只比较行为属性。 |
| `M10-A11` | Python/UE 全量回归、Target 编译、Stub/失败/Kimi 场景和演示指南均完成。 |

| 场景属性 | 证据结论 |
| --- | --- |
| `FS1-01` | UI 与真实 Kimi 接受未枚举自然语言。 |
| `FS1-02` | Action Parser 只执行 Face、Approach、MoveAway、Attack、Stop，未知输入明确拒绝。 |
| `FS1-03` | Personal Context 过滤其他 Observer；旁观调度只来自 `bHeardClearly`、Direct Self 或 `bSaw`。 |
| `FS1-04` | 显式目标、无目标 Shout 和 InEar 感知回归通过；无目标不会直接标记所有 NPC 为目标。 |
| `FS1-05` | Stub 与 Kimi 多人物差异证据通过。 |
| `FS1-06` | 每 NPC 最近 History、公开 Speech/Action Result、Pending 合并和连续重规划均有界。 |
| `FS1-07` | Rival 的负关系、公开强硬人物和可独立后退/面对建议证明表里不使用单数值固定映射。 |
| `FS1-08` | 所有 Tool 经 UE 版本、TTL、目标、距离、状态、冷却、速率和幂等校验后才执行。 |
| `FS1-09` | 默认地图具有名称/颜色、Speech/Action 气泡、移动、生命/立场和个人 Inspector。 |
| `FS1-10` | 离线烟测 `provider=local`、位置不变、状态安全推进并保持 UI 可操作。 |
| `FS1-11` | Milestone 9 升级—缓和回归保持通过，并扩展到逐 NPC 冲突状态和任意 NPC Attack。 |

## 性能与限制

- 本阶段性能验证关注硬上限和请求风暴：4 个 NPC、全局最多 2 个请求在途、每 NPC 1 个 Pending、0.75 秒冷却、最多 3 次自动重规划；多 NPC Stub 烟测结束时在途为 0。
- 120+5 确定性性能基准沿用 Milestone 6 证据，不把 100+ NPC 与真实 LLM 场景混为同一性能目标。
- 当前未加入正式动画、复杂导航、NPC 间自主对话或 MassEntity 场景集成；这些限制与 Milestone 10 明确不做一致。

后续只在实际执行验证后补充结果；未执行项目不得写为通过。
