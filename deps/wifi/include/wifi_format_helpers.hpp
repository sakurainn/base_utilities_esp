#pragma once

#include <sdkconfig.h>

#include <format>
#include <string>

// for std::format formatting of wifi_phy_rate_t
template <> struct std::formatter<wifi_phy_rate_t> : std::formatter<std::string> {
  template <typename FormatContext>
  auto format(const wifi_phy_rate_t &value, FormatContext &ctx) const -> decltype(ctx.out()) {
    switch (value) {
      // base
    case WIFI_PHY_RATE_1M_L:
      return std::format_to(ctx.out(), "1 Mbps with long preamble");
    case WIFI_PHY_RATE_2M_L:
      return std::format_to(ctx.out(), "2 Mbps with long preamble");
    case WIFI_PHY_RATE_5M_L:
      return std::format_to(ctx.out(), "5.5 Mbps with long preamble");
    case WIFI_PHY_RATE_11M_L:
      return std::format_to(ctx.out(), "11 Mbps with long preamble");
    case WIFI_PHY_RATE_2M_S:
      return std::format_to(ctx.out(), "2 Mbps with short preamble");
    case WIFI_PHY_RATE_5M_S:
      return std::format_to(ctx.out(), "5.5 Mbps with short preamble");
    case WIFI_PHY_RATE_11M_S:
      return std::format_to(ctx.out(), "11 Mbps with short preamble");

      // HT
    case WIFI_PHY_RATE_48M:
      return std::format_to(ctx.out(), "48 Mbps");
    case WIFI_PHY_RATE_24M:
      return std::format_to(ctx.out(), "24 Mbps");
    case WIFI_PHY_RATE_12M:
      return std::format_to(ctx.out(), "12 Mbps");
    case WIFI_PHY_RATE_6M:
      return std::format_to(ctx.out(), "6 Mbps");
    case WIFI_PHY_RATE_54M:
      return std::format_to(ctx.out(), "54 Mbps");
    case WIFI_PHY_RATE_36M:
      return std::format_to(ctx.out(), "36 Mbps");
    case WIFI_PHY_RATE_18M:
      return std::format_to(ctx.out(), "18 Mbps");
    case WIFI_PHY_RATE_9M:
      return std::format_to(ctx.out(), "9 Mbps");

      // Long GI
    case WIFI_PHY_RATE_MCS0_LGI:
      return std::format_to(ctx.out(), "MCS0_LGI (6.5-13.5 Mbps)");
    case WIFI_PHY_RATE_MCS1_LGI:
      return std::format_to(ctx.out(), "MCS1_LGI (13-27 Mbps)");
    case WIFI_PHY_RATE_MCS2_LGI:
      return std::format_to(ctx.out(), "MCS2_LGI (19.5-40.5 Mbps)");
    case WIFI_PHY_RATE_MCS3_LGI:
      return std::format_to(ctx.out(), "MCS3_LGI (26-54 Mbps)");
    case WIFI_PHY_RATE_MCS4_LGI:
      return std::format_to(ctx.out(), "MCS4_LGI (39-81 Mbps)");
    case WIFI_PHY_RATE_MCS5_LGI:
      return std::format_to(ctx.out(), "MCS5_LGI (52-108 Mbps)");
    case WIFI_PHY_RATE_MCS6_LGI:
      return std::format_to(ctx.out(), "MCS6_LGI (58.5-121.5 Mbps)");
    case WIFI_PHY_RATE_MCS7_LGI:
      return std::format_to(ctx.out(), "MCS7_LGI (65-135 Mbps)");
#if CONFIG_SOC_WIFI_HE_SUPPORT || !CONFIG_SOC_WIFI_SUPPORTED
    case WIFI_PHY_RATE_MCS8_LGI:
      return std::format_to(ctx.out(), "MCS8_LGI (97.5 Mbps)");
    case WIFI_PHY_RATE_MCS9_LGI:
      return std::format_to(ctx.out(), "MCS9_LGI (108.3 Mbps)");
#endif

      // Short GI
    case WIFI_PHY_RATE_MCS0_SGI:
      return std::format_to(ctx.out(), "MCS0_SGI (7.2-15 Mbps)");
    case WIFI_PHY_RATE_MCS1_SGI:
      return std::format_to(ctx.out(), "MCS1_SGI (14.4-30 Mbps)");
    case WIFI_PHY_RATE_MCS2_SGI:
      return std::format_to(ctx.out(), "MCS2_SGI (21.7-45 Mbps)");
    case WIFI_PHY_RATE_MCS3_SGI:
      return std::format_to(ctx.out(), "MCS3_SGI (28.9-60 Mbps)");
    case WIFI_PHY_RATE_MCS4_SGI:
      return std::format_to(ctx.out(), "MCS4_SGI (43.3-90 Mbps)");
    case WIFI_PHY_RATE_MCS5_SGI:
      return std::format_to(ctx.out(), "MCS5_SGI (57.8-120 Mbps)");
    case WIFI_PHY_RATE_MCS6_SGI:
      return std::format_to(ctx.out(), "MCS6_SGI (65.0-135 Mbps)");
    case WIFI_PHY_RATE_MCS7_SGI:
      return std::format_to(ctx.out(), "MCS7_SGI (72.2-150 Mbps)");
#if CONFIG_SOC_WIFI_HE_SUPPORT || !CONFIG_SOC_WIFI_SUPPORTED
    case WIFI_PHY_RATE_MCS8_SGI:
      return std::format_to(ctx.out(), "MCS8_SGI (103.2 Mbps)");
    case WIFI_PHY_RATE_MCS9_SGI:
      return std::format_to(ctx.out(), "MCS9_SGI (114.7 Mbps)");
#endif

      // LORA
    case WIFI_PHY_RATE_LORA_250K:
      return std::format_to(ctx.out(), "LoRa 250Kbps");
    case WIFI_PHY_RATE_LORA_500K:
      return std::format_to(ctx.out(), "LoRa 500Kbps");
    default:
      return std::format_to(ctx.out(), "Unknown PHY rate: {}", static_cast<int>(value));
    }
  }
};

// for std::format formatting of wifi_mode_t
template <> struct std::formatter<wifi_mode_t> : std::formatter<std::string> {
  template <typename FormatContext>
  auto format(const wifi_mode_t &value, FormatContext &ctx) const -> decltype(ctx.out()) {
    switch (value) {
    case WIFI_MODE_NULL:
      return std::format_to(ctx.out(), "WIFI_MODE_NULL");
    case WIFI_MODE_STA:
      return std::format_to(ctx.out(), "WIFI_MODE_STA");
    case WIFI_MODE_AP:
      return std::format_to(ctx.out(), "WIFI_MODE_AP");
    case WIFI_MODE_APSTA:
      return std::format_to(ctx.out(), "WIFI_MODE_APSTA");
    default:
      return std::format_to(ctx.out(), "Unknown mode: {}", static_cast<int>(value));
    }
  }
};

// for std::format formatting of wifi_ps_type_t
template <> struct std::formatter<wifi_ps_type_t> : std::formatter<std::string> {
  template <typename FormatContext>
  auto format(const wifi_ps_type_t &value, FormatContext &ctx) const -> decltype(ctx.out()) {
    switch (value) {
    case WIFI_PS_NONE:
      return std::format_to(ctx.out(), "WIFI_PS_NONE");
    case WIFI_PS_MIN_MODEM:
      return std::format_to(ctx.out(), "WIFI_PS_MIN_MODEM");
    case WIFI_PS_MAX_MODEM:
      return std::format_to(ctx.out(), "WIFI_PS_MAX_MODEM");
    default:
      return std::format_to(ctx.out(), "Unknown PS type: {}", static_cast<int>(value));
    }
  }
};

// for std::format formatting of wifi_storage_t
template <> struct std::formatter<wifi_storage_t> : std::formatter<std::string> {
  template <typename FormatContext>
  auto format(const wifi_storage_t &value, FormatContext &ctx) const -> decltype(ctx.out()) {
    switch (value) {
    case WIFI_STORAGE_RAM:
      return std::format_to(ctx.out(), "WIFI_STORAGE_RAM");
    case WIFI_STORAGE_FLASH:
      return std::format_to(ctx.out(), "WIFI_STORAGE_FLASH");
    default:
      return std::format_to(ctx.out(), "Unknown storage type: {}", static_cast<int>(value));
    }
  }
};
