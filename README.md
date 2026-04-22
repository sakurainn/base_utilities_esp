# Base Utilities

ESP-IDF 基础工具组件，提供常用的开发工具封装。

## 功能介绍

- **FileHandle** - UNIX 风格文件操作封装
- **KVDB** - 键值数据库封装（支持 Flashdb 和 NVS 后端）
- **DelayExec** - FreeRTOS 延迟执行器
- **WiFi** - WiFi AP/STA 模式封装
- **Timer** - 定时器封装
- **UUID** - UUID v4 生成
- **WebsocketClient** - WebSocket 客户端封装
- **FTP Client** - FTP 客户端
- **Thread** - 线程封装
- **IPAddress** - IP 地址处理

## 依赖

### ESP-IDF 框架组件

| 组件 | 说明 |
|------|------|
| driver | 驱动库 |
| nvs_flash | 非易失性存储库 |
| freertos | 实时操作系统 |
| esp_wifi | WiFi 功能 |
| esp_event | 事件库 |
| esp_timer | 定时器库 |
| esp-tls | TLS 加密 |
| esp_websocket_client | WebSocket 客户端 |

### IDF 组件管理器依赖

在 `idf_component.yml` 中定义：

```yaml
dependencies:
  idf: ">=4.4"
  bblanchon/arduinojson: "^7.4.3"
  espressif/esp_websocket_client: "^1.6.1"
```

### 内置第三方源码依赖

本组件在 `deps/` 目录下内置了以下第三方开源项目的源代码：

| 依赖 | 用途 | 许可证 | 项目地址 |
|------|------|--------|----------|
| stb_ds | 动态数组和哈希表 | Public Domain | [nothings/stb](https://github.com/nothings/stb) |
| FlashDB | 键值/时间序列数据库 | MIT | [armink/FlashDB](https://github.com/armink/FlashDB) |
| magic_enum | 编译期枚举反射 | MIT | [Neargye/magic_enum](https://github.com/Neargye/magic_enum) |
| simple_uuid | UUID v4 生成 | Public Domain | - |
| Ticker | 定时回调器 | MIT | [arduino-libraries/Ticker](https://github.com/arduino-libraries/Ticker) |
| esp_event_cxx | ESP event C++ 封装 | MIT | - |
| Preferences | 偏好存储封装 | MIT | 基于 Arduino Core |

## 使用方法

在你的项目 `idf_component.yml` 中添加依赖：

```yaml
dependencies:
  base_utilities:
    path: components/base_utilities
```

在代码中引用头文件即可使用：

```cpp
#include "BaseUtilities.hpp"
#include "KVDB.hpp"
#include "WiFi.hpp"
#include "WebsocketClient.hpp"
```

## 许可证说明

本组件的原创代码采用 MIT 许可证授权。本组件内置了多个第三方开源项目，各自遵循其原许可证，请参见上表。

如需说明本组件的许可证，可以按照以下格式表述：

```
MIT License

Copyright (c) 2024

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

本软件包含了以下第三方开源软件：

- stb_ds: Public Domain - https://github.com/nothings/stb
- FlashDB: MIT License - https://github.com/armink/FlashDB
- magic_enum: MIT License - https://github.com/Neargye/magic_enum
- Ticker: MIT License - https://github.com/arduino-libraries/Ticker
```

## 注意事项

- 本组件针对 ESP-IDF 框架开发，需要 ESP-IDF v4.4 或更高版本
- 内置的第三方依赖保持其原始许可证不变，使用时请遵循各项目的许可要求
