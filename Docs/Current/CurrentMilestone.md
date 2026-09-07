# Milestone 15：配置驱动场景与测试隔离

## 状态

- 状态：`进行中`
- 归档前置：[Milestone14.md](../Milestones/Milestone14.md)
- 协议状态：不修改 UE/Python 协议或 Decision Context。

## 目标

移除 Social Sandbox 运行时场景中的默认 Actor、固定 NPC 与测试编排；场景只使用已放置 Actor、Spawner、Persona 与 World Context 配置。Smoke、固定测试角色、命令行预设和断言仅保留在测试代码或测试资源中。

## 玩家可操作成果

- 在地图中放置 PlayerStart、场景网格/灯光、NPC Spawner 和可选 World Context 配置后进入场景。
- 在不依赖默认 Guard/Merchant/Rival/Civilian 的前提下，对已放置 NPC 进行互动。

## 屏幕可见成果

- 场景中只出现关卡实际放置的 Actor；没有 Spawner 时显示明确配置提示，不隐式补建对象。
- Inspector 和互动目标只列出已由 Spawner 注册的 NPC。

## 本阶段范围

### 场景运行时边界

- GameMode 不得 Spawn 地板、灯光、PlayerStart、默认 NPC 或固定坐标。
- NPC 只能经关卡中的 Spawner 生成并注册；普通路径不读取命令行预设或测试参数。
- 行为调度不得依赖固定 NPC ID；测试专用身份只能存在于测试 Fixture。

### 测试隔离

- 移除 GameMode 中的 Smoke 定时器、命令行解析、退出码和固定角色断言。
- 测试 Fixture 在测试 World 或测试资源中创建预设角色，执行断言并清理。

## 明确不做

- 不改变 UE/Python 协议、Provider、Prompt 或 Decision Context 字段。
- 不将测试 Actor、固定 ID 或命令行 Smoke 开关保留在普通运行时场景。

## 验收标准

| ID | 标准 |
| --- | --- |
| `M15-A01` | 普通 GameMode 不创建默认环境或固定 NPC，且场景无 Spawner 时不隐式生成角色。 |
| `M15-A02` | 关卡 Spawner 生成的 NPC 均注册到互动、Inspector 和通用调度链路。 |
| `M15-A03` | GameMode 不包含命令行 Smoke、退出码或测试角色固定断言；对应测试在 `SocialSandbox/Tests` 中。 |
| `M15-A04` | 场景放置指南明确列出 PlayerStart、网格/灯光、Spawner 和 World Context 的职责；UE Target 编译通过。 |

## 完成定义

1. TaskBoard 的 M15 工作包全部完成，且 `M15-A01` 至 `M15-A04` 有可复查证据。
2. 普通地图不含隐式场景或角色投放，且所有 NPC 由内容配置驱动。
3. 测试代码与普通运行时边界清晰，不依赖生产 GameMode 的测试分支。
