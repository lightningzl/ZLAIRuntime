# Milestone 3 Validation

## 状态

- 里程碑：Milestone 3：NPC 上下文与人格
- 当前阶段：实现中
- 最后更新：2026-07-27
- 结论：未验证

本文件只记录实际执行过的验证和可复查证据。验收标准正文见 [CurrentMilestone.md](../CurrentMilestone.md)；未执行项目保持“未验证”。

## 环境记录

| 项目 | 实际值 |
| --- | --- |
| 日期 | 2026-07-27 |
| Python | 3.12.13 |
| Unreal Engine | 5.8.0 |
| 编译器 | Visual Studio 2022 x64 toolchain |
| Dialogue Provider | M3-02 至 M3-06 自动化与本地集成：Stub/Fake |
| Kimi 模型 | M3-06 未访问真实模型 |

不得记录 API Key、完整玩家输入、完整上下文、模型回复正文或可还原敏感数据的片段。

## 自动化验证

| 验证 | 命令或入口 | 结果 | 证据摘要 |
| --- | --- | --- | --- |
| Python 完整测试 | `PythonService/.venv/Scripts/python -m pytest -q -p no:cacheprovider` | 通过 | 92 passed；上下文 Schema/边界、Builder、Service、Provider、API、日志脱敏与既有回归通过 |
| Python 依赖检查 | `PythonService/.venv/Scripts/python -m pip check` | 通过 | No broken requirements found |
| Python 字节码编译 | `PythonService/.venv/Scripts/python -m compileall -q app tests` | 通过 | 应用与测试模块编译通过 |
| UE 编译 | `ZLEditor Win64 Development` | 通过 | UnrealBuildTool Result: Succeeded |
| UE 自动化测试 | `ZLAIRuntime` | 通过 | 完整插件 8/8 成功；本地 Stub 集成覆盖旧/新入口、服务端错误、非法上下文本地失败和单次回调 |
| UE Game 上下文演示 | `ZL.AI.DialogueContextDemo persona npc_demo_01 <input>` | 通过 | 无界面 Game 返回 `provider=stub`、回复非空，request ID 与 NPC ID 一致，进程正常退出 |

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
| `M3-A09` | 未验证 | 未验证 |
| `M3-A10` | 未验证 | 未验证 |

## 真实端到端记录

未执行。最终验收只记录脱敏场景说明、请求 ID、结构化上下文摘要、HTTP 状态、Provider 和结果，不记录完整输入、上下文或回复正文。

## 最终结论

未验证。全部实现任务完成后，按 [CurrentMilestone.md](../CurrentMilestone.md) 中 `M3-A01` 至 `M3-A10` 逐项补充证据。
