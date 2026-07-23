# Milestone 2 Validation

## 状态

- 里程碑：Milestone 2：真实 LLM 自由对话
- 当前阶段：已完成
- 最后更新：2026-07-23
- 结论：通过

本文件只记录实际执行过的验证和可复查证据。验收标准正文见 [CurrentMilestone.md](../CurrentMilestone.md)；未执行项目保持“未验证”。

## 环境记录

| 项目 | 实际值 |
| --- | --- |
| 日期 | 2026-07-23 |
| Python | 3.12.13 |
| Unreal Engine | 5.8.0 |
| 编译器 | Visual Studio 2022 x64 toolchain |
| Dialogue Provider | 自动化：Stub/Fake Kimi；真实端到端：Kimi |
| Kimi 模型 | `kimi-k2.6`（关闭思考，单次、非流式、无自动重试） |
| UE 外层超时 | 30 秒 |
| Provider 超时 | 默认 20 秒 |
| 本机代理处理 | UE 验收进程使用 `-HttpNoProxy=127.0.0.1,localhost` 直连本机 Service |

不得记录 API Key 值或可还原密钥的片段。

## 自动化验证

| 验证 | 命令或入口 | 结果 | 证据摘要 |
| --- | --- | --- | --- |
| Python 完整测试 | `PythonService/.venv/Scripts/python -m pytest -q -p no:cacheprovider` | 通过 | 57 passed；自动移除 API Key、拒绝非本机网络、无真实 Token |
| Python 依赖检查 | `PythonService/.venv/Scripts/python -m pip check` | 通过 | No broken requirements found |
| UE 编译 | `ZLEditor Win64 Development` | 通过 | UnrealBuildTool Result: Succeeded |
| UE 自动化测试 | `ZLAIRuntime` | 通过 | 6/6 成功、0 失败；2 项含预期失败路径 warning |
| UE Game 演示入口 | `ZL.AI.DialogueDemo <npc_id> <player_input>` | 通过 | 真实 Kimi 回复可见；真实超时展示 `504 provider_timeout`，两次 Game 均正常退出 |

## 验收证据

| 验收 ID | 状态 | 证据 |
| --- | --- | --- |
| `M2-A01` | 通过 | Stub 启动与本地 UE HTTP 集成测试通过；无需 API Key 或外网 |
| `M2-A02` | 通过 | Settings、Factory、启动边界和脱敏错误自动化测试通过 |
| `M2-A03` | 通过 | 真实 `POST /v1/dialogue` 返回 `200`，`reply` 非空且响应结构符合协议 |
| `M2-A04` | 通过 | 真实 HTTP 与 UE 链路均保持 `request_id`、`npc_id` 一致，成功响应为 `provider: kimi` |
| `M2-A05` | 通过 | Fake SDK 到 HTTP 完整映射覆盖鉴权、限流、超时、不可用、无效响应和脱敏 |
| `M2-A06` | 通过 | 57 项 Python 离线测试通过，全局网络和真实密钥隔离生效 |
| `M2-A07` | 通过 | UE 自动化解析 `stub`、`kimi`、未来 Provider，并保留新增协议错误码 |
| `M2-A08` | 通过 | UE 真实请求显示非空 Kimi 回复；真实 Provider 超时显示结构化错误，Game 正常退出且无崩溃 |
| `M2-A09` | 通过 | Provider 超时临时设为 1 毫秒、UE 外层保持 30 秒；1 次请求只产生 1 次 `504 provider_timeout` 失败回调，无成功回调或崩溃 |
| `M2-A10` | 通过 | 同一 UE Game 会话提交 3 条不同输入；Service 记录 3 次 `200`，UE 收到 3 条回复、0 失败，三个请求 ID 全部一一对应 |

## 真实端到端记录

使用控制台入口：

```text
ZL.AI.DialogueDemo <npc_id> <player_input>
```

| 请求序号 | 脱敏输入说明 | `request_id` | HTTP | `provider` | UE 展示 | 结果 |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 地点询问 | `DA6F39E8-4682-ADBD-14DE-9F9B398FE035` | `200` | `kimi` | 非空回复 | 通过 |
| 2 | 道路安全询问 | `CA8FE255-4A49-F5A5-18D8-F7B63A35531D` | `200` | `kimi` | 非空回复 | 通过 |
| 3 | 事件询问 | `AC27F001-4B5B-6E41-88F8-DAB2FC614A98` | `200` | `kimi` | 非空回复 | 通过 |

三条请求在同一无界面 UE Game 会话中发出。验收只记录脱敏输入说明、请求 ID、状态和结构，不记录完整玩家输入、模型回复正文或供应商原始响应。

## 真实失败路径记录

| 场景 | Provider 超时 | UE 外层超时 | Service | UE 回调 | 进程结果 |
| --- | --- | --- | --- | --- | --- |
| Kimi 请求超时 | 1 毫秒（仅验收进程） | 30 秒 | `504 provider_timeout` | 失败 1 次、成功 0 次 | UE 正常退出 |

## 最终结论

Milestone 2 验收通过。`M2-A01` 至 `M2-A10` 均具有可复查证据，[TaskBoard.md](../TaskBoard.md) 中 `M2-08` 已完成，里程碑状态更新为 `已完成`。
