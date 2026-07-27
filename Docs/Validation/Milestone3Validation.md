# Milestone 3 Validation

## 状态

- 里程碑：Milestone 3：NPC 上下文与人格
- 当前阶段：已完成
- 最后更新：2026-07-27
- 结论：通过

本文件只记录实际执行过的验证和可复查证据。验收标准正文见 [CurrentMilestone.md](../CurrentMilestone.md)；未执行项目保持“未验证”。

## 环境记录

| 项目 | 实际值 |
| --- | --- |
| 日期 | 2026-07-27 |
| Python | 3.12.13 |
| Unreal Engine | 5.8.0 |
| 编译器 | Visual Studio 2022 x64 toolchain |
| Dialogue Provider | 自动化与本地集成：Stub/Fake；真实端到端：Kimi |
| Kimi 模型 | `kimi-k2.6`（关闭思考，单次、非流式、无自动重试） |

不得记录 API Key、完整玩家输入、完整上下文、模型回复正文或可还原敏感数据的片段。

## 自动化验证

| 验证 | 命令或入口 | 结果 | 证据摘要 |
| --- | --- | --- | --- |
| Python 完整测试 | `PythonService/.venv/Scripts/python -m pytest -q -p no:cacheprovider` | 通过 | 92 passed；上下文 Schema/边界、Builder、Service、Provider、API、日志脱敏与既有回归通过 |
| Python 依赖检查 | `PythonService/.venv/Scripts/python -m pip check` | 通过 | No broken requirements found |
| Python 字节码编译 | `PythonService/.venv/Scripts/python -m compileall -q app tests` | 通过 | 应用与测试模块编译通过 |
| UE 编译 | `ZLEditor Win64 Development` | 通过 | UnrealBuildTool Result: Succeeded |
| UE 自动化测试 | `ZLAIRuntime` | 通过 | 完整插件 8/8 成功；本地 Stub 集成覆盖旧/新入口、服务端错误、非法上下文本地失败和单次回调 |
| UE Game 本地上下文演示 | `ZL.AI.DialogueContextDemo persona npc_demo_01` | 通过 | 无界面 Game 返回 `provider=stub`、回复非空，request ID 与 NPC ID 一致，进程正常退出 |
| UE Game 真实 Kimi 验收 | `ZL.AI.DialogueContextDemo <scenario> <npc_id>` | 通过 | 人格、世界、历史 3/3 返回 `provider=kimi` 且受控匹配为真；只记录脱敏元数据 |
| Service 重启无状态检查 | 重启后发送无 `context` 请求 | 通过 | HTTP 200、request/NPC ID 一致、回复非空；未出现重启前场景标记 |
| 安全审计 | 已跟踪文件、任务 diff、UE/Service 日志和响应 | 通过 | 无密钥形状字面量、无生成/环境文件、无完整输入/上下文/回复日志；错误响应保持脱敏 |

## 验收证据

| 验收 ID | 状态 | 证据 |
| --- | --- | --- |
| `M3-A01` | 通过 | Python 92/92 与 UE 8/8 完整回归通过；既有无上下文成功、Provider 错误/超时分类、协议响应和单次回调保持兼容 |
| `M3-A02` | 通过 | Python/UE 覆盖完整字段、最小/最大边界、Unicode code point、非法角色、空白、未知字段和无上下文序列化兼容 |
| `M3-A03` | 通过 | Context Builder 专项 5/5；固定系统约束、JSON 上下文数据、历史角色/顺序和末尾当前输入均以不可变内部类型确定性组装 |
| `M3-A04` | 通过 | Fake SDK 验证固定系统约束不能被请求数据改写；请求保持单次、非流式、无工具、无托管会话和无自动重试 |
| `M3-A05` | 通过 | Capturing/Fake Provider 验证人格、世界状态、历史和当前输入完整按序进入 `DialogueGenerationContext`，Provider 不依赖协议 Schema |
| `M3-A06` | 通过 | 完整离线测试 92/92；覆盖无上下文兼容、完整上下文、全部字段边界、非法角色、空白内容、历史顺序、注入边界、日志脱敏和既有错误路径；全局移除真实 Key 并拦截非本机网络 |
| `M3-A07` | 通过 | 旧入口与新上下文重载共享发送/完成逻辑；协议测试覆盖省略、完整、最大边界和非法上下文；非法上下文创建 HTTP 前失败且只回调一次 |
| `M3-A08` | 通过 | `ZLEditor Win64 Development` 编译成功；完整 `ZLAIRuntime` 8/8；本地 Stub 旧/新请求均成功，服务端错误和本地非法上下文均正常结束；无界面 Game 演示通过 |
| `M3-A09` | 通过 | 真实 Kimi 人格、世界、历史三类受控场景均通过 UE 闭环；`provider=kimi`、匹配为真，request ID 可与 HTTP 200 一一关联 |
| `M3-A10` | 通过 | 已跟踪文件和验收日志扫描无密钥、完整输入、完整上下文或原始回复；错误响应脱敏测试通过；Service 重启后的无上下文请求未恢复前三类场景状态 |

## 真实端到端记录

| 场景 | 受控变化 | `request_id` | HTTP | Provider | 脱敏结果 |
| --- | --- | --- | --- | --- | --- |
| 人格 | 仅使用人格变体，其余使用基线快照 | `B709A733-4C62-C073-A0D9-1DAE8845B8F6` | `200` | `kimi` | UE 回调完成；受控匹配为真 |
| 世界状态 | 仅使用世界变体，其余使用基线快照 | `CDA511CD-4BF3-B17A-60B0-A49C0D3B7DE1` | `200` | `kimi` | UE 回调完成；受控匹配为真 |
| 历史 | 仅增加两条有限历史，其余使用基线快照 | `E760A90C-421A-381E-8F1B-54A5D0AFA286` | `200` | `kimi` | UE 回调完成；受控匹配为真 |

重启检查请求 `m3-restart-check-001` 不携带 `context`，返回 HTTP 200、`provider=kimi`、非空回复，且未出现上述场景标记。记录只保留关联字段与布尔结果，不包含完整输入、上下文或模型回复正文。

## 最终结论

Milestone 3 验收通过。`M3-A01` 至 `M3-A10` 均有可复查证据；实现保持瞬时客户端上下文、无服务端会话、无 Memory、无 Tool Use 和 UE 最终 Gameplay 控制权边界。
