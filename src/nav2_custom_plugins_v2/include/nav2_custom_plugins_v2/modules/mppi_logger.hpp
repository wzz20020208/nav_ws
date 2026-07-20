#ifndef NAV2_CUSTOM_PLUGINS_V2__MPPI_LOGGER_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__MPPI_LOGGER_HPP_

#include <string>
#include <fstream>
#include "nav2_custom_plugins_v2/core/mppi_core.hpp"

namespace nav2_custom_plugins_v2 {

/// 运行时日志与统计: CSV 文件输出 + 停滞检测
class MPPILogger {
public:
  MPPILogger() = default;

  /// 初始化 (打开文件, 写 CSV 表头)
  void init(const std::string& file_path, bool enabled);

  /// 记录一帧数据
  void logFrame(
      int frame, double time_s,
      const Control& cmd, const Control& raw,
      float best_cost,
      double cross_track_err, double heading_err, double dist_to_goal);

  /// 停滞检测: 速度低于阈值时累计, 返回是否持续停滞超过 report_interval
  bool checkStall(double speed, double threshold, double report_interval,
                  double current_time);

  /// 关闭文件
  void close();

  bool enabled() const { return enabled_; }

private:
  std::ofstream file_;
  bool enabled_ = false;
  bool header_written_ = false;

  double stall_start_time_ = -1.0;
  double last_stall_report_time_ = -1.0;
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__MPPI_LOGGER_HPP_
