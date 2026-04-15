# AI Agent 规则（本项目）

## 1. 目标与范围

本规则用于约束 AI 在本仓库的输出与执行方式，目标是：

- 输出可落地、可复现、可学习；
- 与本项目技术路线一致（`macOS + VSCode + CubeMX(Makefile) + DAPLink + OpenOCD`）；
- 减少无效沟通与格式噪声。

## 2. 沟通与输出规则

- 默认使用中文回复。
- 先给结论，再给细节。
- 简单问题短答，学习类问题结构化深答。
- 解释配置时必须包含四点：
  - 这是什么；
  - 为什么要这样配；
  - 怎么改最安全；
  - 常见错误与排查。

## 3. 执行方式规则

- 能直接执行的任务优先直接执行，不空谈方案。
- 优先用真实命令验证，不凭感觉下结论。
- 命令失败时必须给出：根因 + 可复制修复命令。
- 多方案并存时，优先给“最稳默认方案”。

## 4. 项目技术默认值

- 工具链路径：`/Applications/ArmGNUToolchain/14.2.rel1/arm-none-eabi/bin`
- OpenOCD 接口：`interface/cmsis-dap.cfg`
- OpenOCD 目标：`target/stm32g4x.cfg`
- 默认调试速度：`adapter speed 1000`

## 5. VSCode 配置原则

- `tasks.json` 负责构建/烧录自动化；
- `launch.json` 负责调试会话编排；
- `extensions.json` 仅做扩展推荐，不参与构建逻辑；
- 任务里保留显式 `PATH`，避免 GUI 启动 VSCode 环境变量缺失；
- 调试里保留显式 `gdbPath` 和 `armToolchainPath`，避免路径歧义。

## 6. 文档输出规则

- 所有共享 Markdown 文档必须兼容 `markdownlint`。
- 标题和列表前后保留空行（规避 `MD022/MD032`）。
- 共享文档避免使用 Obsidian 专属 `[[双链]]`，优先标准 Markdown 链接。
- 学习主文档以 `Obsidian_Vault` 为准，避免多处重复维护。

## 7. Git 卫生规则

- 禁止把 `.vscode/cmsis-svd-stm32` 重新变成嵌套 Git 仓库。
- 未经明确要求，不提交 `Obsidian_Vault/.obsidian/`。
- 提交信息使用语义前缀：
  - `docs: ...`
  - `fix: ...`
  - `chore: ...`

## 8. 完成标准（DoD）

### 8.1 环境/调试类任务

完成前至少满足：

- `arm-none-eabi-gcc --version` 正常；
- `arm-none-eabi-gdb --version` 正常；
- `openocd --version` 正常；
- `make -j8` 通过；
- OpenOCD 可识别 `cmsis-dap` 和 `stm32g4x`；
- VSCode `F5` 可启动调试。

### 8.2 文档类任务

完成前至少满足：

- 关键链接可打开；
- Markdown 告警清理；
- 首页导航（`Obsidian_Vault/Home.md`）已更新。

