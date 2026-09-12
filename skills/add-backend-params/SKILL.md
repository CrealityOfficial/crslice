---
name: add-backend-params
description: 增加后台参数。用户要求在 crslice 参数仓库添加切片参数 key 时，参考 C3DSlicer 的 PrintConfig.cpp 补充公共参数定义，并按机器、工艺、耗材分类同步 key 清单；按用户要求执行 Git 提交。
---

# 增加后台参数

在 crslice 仓库中维护 `parameter/base/fdm_machine_common.json` 和 `parameter/keys/`。默认源码参考 `F:/work/C3DSlicer/src/libslic3r/PrintConfig.cpp`，用户指定其他源码路径时采用该路径。仓库位置以当前工作区为准，本机通常为 `C:/work/crslice`。

## 确认定义与分类

- 从用户输入提取 key，去掉 Markdown 对下划线的转义；读取仓库指令和 Git 状态，保留已有修改。
- 在 `PrintConfig.cpp` 中定位每个 `this->add("key", type)`，读取该定义块的 label、tooltip、sidetext、min/max、枚举和 `set_default_value`。拼接 C++ 相邻字符串，保留换行与转义的实际含义。没有定义的 key 不凭名称编造，报告缺失项。
- 用户要求参考源码默认值时，以 `set_default_value` 为准，输入中的示例值不自动成为公共默认值。用户明确要求覆盖时才采用指定值，并说明与源码的差异。
- 分类参考同目录 `Preset.cpp` 的 `Preset::machine_options()`（包括 machine limits；`printer_options()` 还组合 nozzle 子集）、`Preset::print_options()`、`Preset::filament_options()`，必要时追踪其数组和组合逻辑。不要根据 key 名称或 UI category 猜测。确实属于多个清单时按源码及后台已有拆分规则处理。

## 修改公共定义与清单

公共定义文件包含所有类别的参数，不仅是机器参数。沿用现有结构、缩进及换行，局部插入，避免整份文件重新序列化。

| 后台字段 | 源码依据 |
| --- | --- |
| `default_value` | 默认选项的 `serialize()` 结果；布尔为字符串 `"0"` / `"1"`，数值也用字符串；数组、百分比、枚举等采用源码序列化格式 |
| `type` | `coBool`、`coFloat`、`coInt` 等定义类型 |
| `label` / `description` / `unit` | `label` / `tooltip` / `sidetext`；未设置则空字符串 |
| `minimum_value` / `maximum_value` | 源码有效上下限；无约束不自行添加 |
| `options` | 枚举值到标签的映射，遵循源码导出逻辑 |
| `enabled` | 字符串 `"true"` |

`settable_*` 按 `src/slic3r/GUI/BackgroundSlicingProcess.cpp` 中 `export_metas_impl()` 的实际导出规则确认，结合 `PrintConfig.hpp` 的类成员和继承关系：

- `settable_globally`：`PrintConfig.has(key)`。
- `settable_per_extruder`：是否位于 `Preset::print_options()`，不是由 key 是否涉及挤出机决定。
- `settable_per_mesh`：`PrintObjectConfig.has(key)`。
- `settable_per_meshgroup`：`PrintRegionConfig.has(key)`。

这些字段均写为 `"true"` / `"false"` 字符串。源码版本的导出规则变化时以实际源码为准。

| 分类 | 同步文件 |
| --- | --- |
| 机器 | `parameter/keys/machine_keys` 和 `parameter/keys/machine_keys.json` |
| 工艺 | `parameter/keys/profile_keys` 和 `parameter/keys/profile_keys.json` |
| 耗材 | `parameter/keys/material_keys.json`；若仓库存在 `material_keys` 无扩展名文件，也同步它 |

无扩展名清单一行一个 key，JSON 清单使用 `{"keys": [...]}`。重复提到的 `machine_keys` 不当作耗材清单。仓库还存在 `extruder_keys` / `extruder_keys.json`；当机器参数属于源码及后台导出规则中的挤出机子集时，遵循已有拆分规则。必要时查看同一 CPP 的 key 导出函数。不要将所有机器参数重复放入挤出机清单。

已有 key 不重复插入；存在定义差异时按本次请求更新。只修改目标 key，不顺带整理历史重复项、全量排序或修复无关分类。

## 校验与提交

- 解析受影响的 JSON，检查新增 key 没有重复定义，字段类型、默认值、文本及作用域与源码一致。
- 核对本次 key 在所属类别的两个清单中各出现一次，在不所属的类别中没有新增。耗材只有 JSON 文件时不凭空创建配对文件。
- 检查 diff，确认保留所有原有条目且没有无关改动，执行 `git diff --check`。纯参数变更无需构建切片器。
- 用户要求提交时，只暂存本次明确修改的文件或区块并创建本地提交；已有提交授权时直接执行。未要求提交时保留工作区结果。推送或后台上线需要对应的用户授权。
- 完成后报告新增 key、分类、默认值差异、校验结果及提交哈希（若有）。
