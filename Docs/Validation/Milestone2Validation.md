# Milestone 2 Validation

## 状态

- 里程碑：Milestone 2：真实 LLM 自由对话
- 当前阶段：验收中
- 最后更新：2026-07-23
- 结论：未验收

本文件只记录实际执行过的验证和可复查证据。验收标准正文见 [CurrentMilestone.md](../CurrentMilestone.md)；未执行项目保持“未验证”。

## 环境记录

| 项目 | 实际值 |
| --- | --- |
| 日期 | 2026-07-23 |
| Python | 3.12.13 |
| Unreal Engine | 5.8.0 |
| 编译器 | Visual Studio 2022 x64 toolchain |
| Dialogue Provider | 自动化：Stub/Fake OpenAI；真实 OpenAI 尚未验证 |
| OpenAI 模型 | 未验证 |
| UE 外层超时 | 30 秒 |
| Provider 超时 | 默认 20 秒 |

不得记录 API Key 值或可还原密钥的片段。

## 自动化验证

| 验证 | 命令或入口 | 结果 | 证据摘要 |
| --- | --- | --- | --- |
| Python 完整测试 | `PythonService/.venv/Scripts/python -m pytest -q` | 通过 | 56 passed；自动移除 API Key、拒绝非本机网络、无真实 Token |
| Python 依赖检查 | `PythonService/.venv/Scripts/python -m pip check` | 通过 | No broken requirements found |
| UE 编译 | `ZLEditor Win64 Development` | 通过 | UnrealBuildTool Result: Succeeded |
| UE 自动化测试 | `ZLAIRuntime` | 通过 | 6/6 成功、0 失败；2 项含预期失败路径 warning |
| UE Game 演示入口 | `ZL.AI.DialogueDemo npc_guard_01 Hello` | 通过 | Stub 返回可见回复；Fake Provider 返回并展示 `503 provider_unavailable` |

## 验收证据

| 验收 ID | 状态 | 证据 |
| --- | --- | --- |
| `M2-A01` | 通过 | Stub 启动与本地 UE HTTP 集成测试通过；无需 API Key 或外网 |
| `M2-A02` | 通过 | Settings、Factory、启动边界和脱敏错误自动化测试通过 |
| `M2-A03` | 未验证 | 未执行 |
| `M2-A04` | 未验证 | 未执行 |
| `M2-A05` | 通过 | Fake SDK 到 HTTP 完整映射覆盖鉴权、限流、超时、不可用、无效响应和脱敏 |
| `M2-A06` | 通过 | 56 项 Python 离线测试通过，全局网络和真实密钥隔离生效 |
| `M2-A07` | 通过 | UE 自动化解析 `stub`、`openai`、未来 Provider，并保留新增协议错误码 |
| `M2-A08` | 未验证 | 未执行 |
| `M2-A09` | 未验证 | 未执行 |
| `M2-A10` | 未验证 | 未执行 |

## 真实端到端记录

使用控制台入口：

```text
ZL.AI.DialogueDemo <npc_id> <player_input>
```

| 请求序号 | 脱敏输入说明 | `request_id` | HTTP | `provider` | UE 展示 | 结果 |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 未验证 | 未验证 | 未验证 | 未验证 | 未验证 | 未验证 |
| 2 | 未验证 | 未验证 | 未验证 | 未验证 | 未验证 | 未验证 |
| 3 | 未验证 | 未验证 | 未验证 | 未验证 | 未验证 | 未验证 |

## 最终结论

Milestone 2 当前未验收。只有 `M2-A01` 至 `M2-A10` 全部具有通过证据，且 [TaskBoard.md](../TaskBoard.md) 中 `M2-07` 完成后，才能将里程碑状态更新为 `已完成`。
