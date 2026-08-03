# Milestone 5 Validation

## 状态

- 里程碑：Milestone 5：确定性社会模拟基础
- 当前阶段：已完成
- 最后更新：2026-08-03
- 结论：通过，10/10 项已验证

验收标准正文见 [CurrentMilestone.md](../Current/CurrentMilestone.md)。本文只记录实际执行过的验证；未执行项目保持“未验证”。

## 验证环境

- Unreal Engine：5.8，Win64 Development Editor，NullRHI 无界面自动化。
- 编译器：Visual Studio 2022 14.44.35228，Windows SDK 10.0.22621.0。
- 构建工具报告硬件：24 个物理核心、32 个逻辑处理器、63.84 GB 内存。
- Python：3.12.13；pytest 9.1.1。
- AI Provider：`stub`；端到端回归使用隔离 SQLite 测试数据库。

本机 8000 端口在验收时被无关桌面进程占用，因此端到端测试通过临时 UE Config 覆盖连接 `127.0.0.1:18000`。测试结束后已关闭 Stub Service；仓库默认配置与协议未改变。

## 已执行验证

| 范围 | 方法 | 结果 | 证据 |
| --- | --- | --- | --- |
| Python 全量回归 | 在 `PythonService` 执行 `.venv/Scripts/python -m pytest` | 通过 | 收集 166 项，166 项通过，耗时 2.18 秒 |
| UE 既有本地自动化 | 无界面执行 `Automation RunTests ZLAIRuntime` | 通过 | Configuration、Failures 与 Protocol 共 8 项通过 |
| UE Service 端到端 | 启动隔离 Stub Service，执行 `ZLAIRuntime.Integration.ServiceClientCallbacks` | 通过 | 1 项通过；覆盖 Dialogue、Context、Memory、失败响应与 Game Thread 单次回调 |
| UE 社会模拟自动化 | 无界面执行 `Automation RunTests ZL.Social` | 通过 | 7 项通过；包含单元、规则、无界面 Gameplay 闭环和 120 Agent 基准 |
| UE Target 编译 | `Build.bat ZLEditor Win64 Development -Project=ZL/ZL.uproject -WaitMutex -NoHotReload` | 通过 | UHT、`ZLASocialRuntime`、`ZLAIRuntime` 与 `ZL` 编译链接成功 |
| Docs 本地链接 | 递归解析 Markdown 相对链接并检查目标存在 | 通过 | 所有本地 Markdown 链接可解析 |
| Docs JSON 示例 | 解析全部 `json` fenced code block | 通过 | 所有示例均为有效 JSON |
| 文档格式 | `git diff --check` | 通过 | 无空白错误；仅有 Git 行尾转换提示 |
| 协议边界 | 比较 `origin/main` 与当前分支的 `Docs/Reference/Protocol.md` | 通过 | 零差异；Dialogue 字段、错误、超时和 Memory 语义未改变 |

## 120 Agent 基准

固定场景在二维网格中注册 120 个 Level 1 Agent，按稳定索引生成位置与 Personality，随后在原点产生 Gunshot。2026-08-03 最终运行结果：

| 指标 | 结果 |
| --- | ---: |
| 注册 Agent | 120 |
| 查询覆盖 Cell | 441 |
| 空间候选 | 120 |
| 感知通过 | 120 |
| Rule Evaluation | 120 |
| 社会模拟核心处理耗时 | 0.117 ms |

该数字只表示上述本机、Development Editor、NullRHI 和固定场景下的单次证据，不作为跨硬件性能承诺。`ZL.Social.SpatialIndex.BoundedQuery` 另以 120 个沿 X 轴分散的 Agent 验证局部查询候选少于注册总数，证明查询路径按覆盖 Cell 枚举而非全量扫描。

## 验收项

| 验收 ID | 状态 | 证据 |
| --- | --- | --- |
| `M5-A01` | 通过 | Python 166 项、UE 既有 9 项回归通过；协议零差异 |
| `M5-A02` | 通过 | 独立 `ZLASocialRuntime` 编译通过；模块不依赖 `ZL`、HTTP、Python、Provider、Widget 或具体 Actor |
| `M5-A03` | 通过 | `ZL.Social.EventRouter.LifecycleAndDedup` 覆盖 Punch 默认值、生命周期、去重和非法预留事件；Gunshot/Help 由相同受控 Preset 路径定义 |
| `M5-A04` | 通过 | `ZL.Social.SpatialIndex.BoundedQuery` 覆盖 120 Agent、跨 Cell 移动和局部非全量查询 |
| `M5-A05` | 通过 | `ZL.Social.Perception.ChannelsAndBounds` 覆盖 Direct、Visual、Auditory、距离、视线、阈值和过期 |
| `M5-A06` | 通过 | `ZL.Social.State.BoundsDecayAndMemory` 覆盖六 Trait 输入、四 State 边界/衰减、固定容量覆盖和淘汰 |
| `M5-A07` | 通过 | `ZL.Social.Decision.PersonalityCooldownAndExtreme` 对同一 Punch 验证 Observe、Flee、Report、Confront 与重复确定性 |
| `M5-A08` | 通过 | 同一规则测试验证 Gunshot 覆盖 Observe，并显式验证低优先级事件冷却与边际切换迟滞；平分按 Tag 稳定排序 |
| `M5-A09` | 通过 | ZLEditor 编译、7 项社会自动化、9 项既有 Runtime 回归及 `HeadlessVerticalSlice` 通过 |
| `M5-A10` | 通过 | `Benchmark120` 注册并处理 120 Agent；Inspector/Benchmark 输出安全字段检查通过 |
