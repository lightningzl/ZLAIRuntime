# Social Sandbox 场景放置说明

## 必须放置

1. `PlayerStart`：玩家出生点。GameMode 不会补建。
2. 场景网格、碰撞、灯光与后处理：均由关卡或关卡蓝图负责；GameMode 不生成地板或灯光。
3. 一个或多个 `ZLSocialSandboxNpcSpawner`：每个 Spawner 指向 Persona Asset 或 Registry 条目，并设置生成位置。Spawner 成功生成后会向 GameMode 注册 NPC，使其出现在互动目标、Inspector 与通用决策队列中。

## 可选放置与配置

- World Context：在 `ZL/Content/SocialSandbox/WorldContexts/` 创建，并在 `DefaultGame.ini` 的 `DefaultWorldContext` 指向该 Asset。字段和 JSON 导入见 [WorldContextConfigurationGuide.md](WorldContextConfigurationGuide.md)。World Context 是静态背景，不负责生成 NPC 或环境 Actor。
- Persona 与 Registry：放在 `ZL/Content/SocialSandbox/Personas/`；配置和 JSON 见 [PersonaConfigurationGuide.md](PersonaConfigurationGuide.md)。

## 运行时边界

- 没有 Spawner 时，场景会给出“未注册 NPC”的提示，但不会生成默认角色。
- 固定角色 ID、Smoke 参数、退出码和断言属于 `ZL/Source/ZL/SocialSandbox/Tests/` 的测试 Fixture，不能作为关卡运行依赖。
- 若需要不同职业的决策能力，应通过 Persona/后续能力配置建模，不以 `npc_guard` 等固定 ID 判断。
