#ifndef NAV2_CUSTOM_PLUGINS_V2__COST_EVALUATOR_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__COST_EVALUATOR_HPP_

#include <vector>
#include "nav2_custom_plugins_v2/core/mppi_core.hpp"

namespace nav2_custom_plugins_v2 {

/// 代价评估器: GPU 返回代价数组后的 CPU 端后处理
class CostEvaluator {
public:
  explicit CostEvaluator(const MPPIParams& params);

  struct EvaluationResult {
    float min_cost = 0.0f;
    int best_sample_idx = 0;
  };

  /// 从 GPU 代价数组中找最小值及索引
  EvaluationResult evaluate(const std::vector<float>& gpu_costs) const;

  /// 计算前瞻点代价 (CPU 端, 用于窄道检测等)
  float computeLookaheadCost(
      const LookaheadPoint& lh,
      const unsigned char* costmap, int w, int h,
      float resolution, float origin_x, float origin_y) const;

private:
  const MPPIParams& params_;
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__COST_EVALUATOR_HPP_
