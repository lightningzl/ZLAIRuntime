# Milestone 7 Validation

## 状态

- 里程碑：Milestone 7：可操作交互舞台与定向感知
- 当前阶段：已完成
- 最后更新：2026-08-31
- 结论：通过，10/10 项已验证

验收标准正文见已归档的 [Milestone7.md](../Milestones/Milestone7.md)。本文只记录实际执行过的验证，不保存完整玩家输入、Prompt、Memory scope、回复正文或凭据。

## 验证环境

- Unreal Engine：5.8.1，Win64 Development Editor，NullRHI 无渲染自动化与受控场景运行。
- 编译器：Visual Studio 2022 14.44.35228，Windows SDK 10.0.22621.0。
- 构建工具报告硬件：24 个物理核心、32 个逻辑处理器、63.84 GB 内存。
- Python：3.12.13；pytest 9.1.1。
- AI Provider：`stub`；UE 端到端回归使用 `Saved/Automation` 下的隔离 SQLite 文件，完成后已关闭 Service。
- 按本次验收要求未生成或检查截图；场景验收使用 NullRHI 进程退出码、地图/GameMode 加载日志和结构化自动化断言。

## 已执行验证

| 范围 | 方法 | 结果 | 证据 |
| --- | --- | --- | --- |
| UE Target 编译 | 执行 `Build.bat ZLEditor Win64 Development` | 通过 | UHT、`ZLASocialRuntime`、`ZLAIRuntime` 与 `ZL` 编译链接成功 |
| UE 社会模拟自动化 | NullRHI 执行 `Automation RunTests ZL.Social` | 通过 | 收集 21 项，21 项全部为 `Success`，退出码 0 |
| Python 全量回归 | 在 `PythonService` 执行 `.venv/Scripts/python -m pytest` | 通过 | 收集 166 项，166 项通过，耗时 2.04 秒 |
| UE Dialogue/Protocol/Service 回归 | 启动隔离 Stub Service，NullRHI 执行 `Automation RunTests ZLAIRuntime` | 通过 | 收集 9 项，9 项通过；包含 Configuration、Failures、Protocol 6 项和 ServiceClientCallbacks，退出码 0 |
| UE→Python 真实 HTTP | 检查上述 Stub Service 请求结果 | 通过 | 合法 Dialogue/Context/Memory 请求返回 200；受控非法请求返回预期 400；回调测试通过 |
| 专用场景运行 | NullRHI 加载 `/Game/SocialSandbox/Lvl_SocialSandbox`，带 `-ZLSandboxDemo` 运行 60 帧 | 通过 | 地图加载完成，Game class 为 `ZLSocialSandboxGameMode`，正常退出码 0 |
| Docs 本地链接 | 递归解析 Markdown 相对链接并检查目标存在 | 通过 | 所有本地 Markdown 链接可解析 |
| Docs JSON 示例 | 解析全部 `json` fenced code block | 通过 | 所有示例均为有效 JSON |
| 文档与代码格式 | `git diff --check` | 通过 | 无空白错误；仅有 Git 行尾转换提示 |
| 协议边界 | 比较 `origin/main` 与里程碑分支的 `Docs/Reference/Protocol.md` | 通过 | 零差异；Dialogue 字段、错误、超时和 SQLite Memory 语义未改变 |

引擎启动期间出现的 `UnifiedErrorTest`、`Condition failed` 和未安装非 Win64 SDK 信息来自 UE 自带测试/平台探测，发生在本项目自动化队列开始前；本项目 21 项与 9 项队列均以 `TEST COMPLETE. EXIT CODE: 0` 完成。

## 社交沙盒自动化覆盖

| 测试 | 主要证明 |
| --- | --- |
| `ZL.Social.InputValidation.Boundaries` | Unicode 长度、空输入、512 上限和 InEar 显式目标约束 |
| `ZL.Social.ActionParser.Whitelist` | Face、Approach、MoveAway、Stop 中英文白名单及未知行为拒绝 |
| `ZL.Social.Observation.SpeechRangesAndTargets` | Whisper/Talk/Shout/InEar 距离边界、听觉强度与显式目标判断分离 |
| `ZL.Social.Observation.DirectionalVision` | 120 度视野、1500 cm 视觉距离和边界外过滤 |
| `ZL.Social.Observation.BoundsAndIsolation` | 事件生命周期、Observation 容量和逐 NPC 隔离 |
| `ZL.Social.Sandbox.BoundedMotion` | Approach/MoveAway 实际位移、停止距离和完成条件 |
| `ZL.Social.Sandbox.StageConfiguration` | 专用地图存在，默认 Pawn 和交互 Controller 配置正确 |
| `ZL.Social.Sandbox.PerNpcVerticalSlice` | 同一玩家说话对前方、背向和远距离 NPC 产生不同个人 Observation，重置清空状态 |

其余 13 项 `ZL.Social` 回归继续覆盖 Milestone 5/6 的 Event、Perception、State、Memory、Propagation、Relationship、Decision、纵向切片和基准边界。

## 可重复操作路径

1. 在编辑器打开 `/Game/SocialSandbox/Lvl_SocialSandbox` 并运行；使用 WASD 移动、鼠标转向。
2. 在左侧交互面板选择 Speech，分别选择 Whisper、Talk、Shout、InEar，选择可选目标并提交；InEar 无目标或超距会在副作用前显示拒绝。
3. 选择 Action，使用 Face、Approach、MoveAway、Stop 的中英文别名；缺目标、未知行为和不可执行请求显示拒绝，已接受行为才产生 Started/Completed 反馈。
4. 在 Target 中逐个选择 Guard、Merchant、Scout、Civilian；Inspector 显示 Source、Distance、Saw、Heard、Clear、Strength、SpeechMode、ExplicitTarget、TargetJudgment 与过滤原因。
5. 观察玩家说话气泡、动作状态和 NPC 的 `RulePlaceholder` 气泡；点击 Reset 后角色变换、气泡和个人 Observation 回到确定初始状态。
6. 不进行图像采集时，可用启动参数 `-ZLSandboxDemo` 经同一 GameMode 提交路径自动选择 Guard 并执行一次 Talk；本次验收已用该路径完成地图运行检查。

## 验收项

| 验收 ID | 状态 | 证据 |
| --- | --- | --- |
| `M7-A01` | 通过 | Python 166 项、UE `ZLAIRuntime` 9 项与 `ZL.Social` 21 项通过；Protocol 零差异，Python Runtime 未修改 |
| `M7-A02` | 通过 | `StageConfiguration`、专用地图运行日志和 GameMode 确认 1 个可控制玩家、4 个稳定 ID NPC、名称/朝向组件与 Reset 入口 |
| `M7-A03` | 通过 | Widget 提供模式、说话级别、目标、文本、Submit/Reset/状态；`Boundaries` 与 GameMode 前置校验覆盖拒绝且无副作用 |
| `M7-A04` | 通过 | `SpeechRangesAndTargets` 覆盖四种范围、强度、InEar 唯一目标以及普通说话的听见/被指向分离 |
| `M7-A05` | 通过 | `DirectionalVision` 与 `PerNpcVerticalSlice` 覆盖视野内、视野外和距离外差异 |
| `M7-A06` | 通过 | 每个 NPC 使用独立容量 32 Buffer；纵向测试验证 Observer ID 隔离、无目标判断和重置，Inspector 只读取选中 NPC 最近 Observation |
| `M7-A07` | 通过 | `Whitelist` 与 `BoundedMotion` 覆盖四种行为、未知输入、目标边界和真实位移；Action Observation 结构不包含输入正文 |
| `M7-A08` | 通过 | 玩家/NPC 使用屏幕空间 UMG 气泡；动作只在接受后显示，NPC 本地回应显式标记 `RulePlaceholder`；运行时 UI 与 GameMode 成功加载 |
| `M7-A09` | 通过 | Speech/Action 生命周期、512 码点、Buffer 32、气泡 0.5 至 8 秒、空间与输入边界均有硬限制；相关 8 项自动化通过 |
| `M7-A10` | 通过 | Target 编译、21 项社会自动化、166 项 Python、9 项 Dialogue/Protocol/Service、真实 HTTP 和无截图场景运行均通过；本文保留可复查操作路径 |

## 范围边界确认

- 未修改 [Protocol.md](../Reference/Protocol.md)，未新增 Decision Endpoint、ToolCall 或 Python 社会状态。
- 未把 Relationship、Social Memory 或个人 Observation 注入现有 Dialogue 请求。
- 未实现 LLM 社会决策、NPC 自主 Gameplay Tool、战斗、复杂导航、Mass 正式集成、语音或正式美术资产。
- Validation 与运行日志不保存完整玩家输入、Prompt、scope、模型隐式推理、API Key 或凭据。
