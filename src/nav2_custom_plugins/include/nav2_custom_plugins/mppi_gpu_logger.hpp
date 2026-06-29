#ifndef NAV2_CUSTOM_PLUGINS__MPPI_GPU_LOGGER_HPP_
#define NAV2_CUSTOM_PLUGINS__MPPI_GPU_LOGGER_HPP_

#include <fstream>
#include <sstream>
#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>

namespace nav2_custom_plugins
{

class MPPIGPULogger
{
public:
  MPPIGPULogger() = default;
  ~MPPIGPULogger() { close(); }

  bool open(const std::string & filepath, const std::string & params = "")
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (ofs_.is_open()) ofs_.close();
    ofs_.open(filepath, std::ios::out | std::ios::trunc);
    if (!ofs_.is_open()) return false;
    ofs_ << std::fixed << std::setprecision(4);
    start_time_ = std::chrono::steady_clock::now();
    ofs_ << "# MPPI GPU Controller Log\n";
    if (!params.empty()) ofs_ << "# PARAM " << params << "\n";
    ofs_ << "# elapsed_s  EVENT  data...\n#\n";
    ofs_.flush();
    return true;
  }

  void close()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (ofs_.is_open()) { ofs_ << "# log closed\n"; ofs_.flush(); ofs_.close(); }
  }

  bool is_open() const { return ofs_.is_open(); }

  void logFrame(int frame, double vx, double vy, double w, double dg, double df,
                double cte, double he, float bc, float mc, int ci, int ti,
                bool ta, bool dz, bool gpu, double pdx, double pdy)
  {
    std::ostringstream o;
    o << "FRAME f=" << frame << " vx=" << vx << " vy=" << vy << " w=" << w
      << " dg=" << dg << " df=" << df << " cte=" << cte << " he=" << he
      << " bc=" << bc << " mc=" << mc << " ci=" << ci << " ti=" << ti
      << " term=" << ta << " dz=" << dz << " gpu=" << gpu
      << " pdx=" << pdx << " pdy=" << pdy;
    write("FRAME", o.str());
  }

  void logTerminal(bool entering, double df, double ye, const char *br,
                   double cv, double cvy, double cw)
  {
    std::ostringstream o;
    o << "TERMINAL in=" << entering << " df=" << df << " ye=" << ye
      << " br=" << br << " cvx=" << cv << " cvy=" << cvy << " cw=" << cw;
    write("TERMINAL", o.str());
  }

  void logDeadzone(double sb, double df, double cv, double cvy)
  {
    std::ostringstream o;
    o << "DEADZONE sb=" << sb << " df=" << df << " cvx=" << cv << " cvy=" << cvy;
    write("DEADZONE", o.str());
  }

  void logGpuError(const char *st, int ec)
  {
    std::ostringstream o;
    o << "GPU_ERR stage=" << st << " code=" << ec;
    write("GPU_ERR", o.str());
  }

  void logStall(double dur, double spd, double dg, double df,
                int lhc, int ahc, int ci, int li, int pl,
                bool ta, bool dz, bool gpu, float bc, const char *reason)
  {
    std::ostringstream o;
    o << "STALL dur=" << dur << " speed=" << spd << " dist_goal=" << dg
      << " dist_final=" << df << " lh_cost=" << lhc << " ahead_cost=" << ahc
      << " closest=" << ci << " lh_idx=" << li << " path_len=" << pl
      << " term=" << ta << " dz=" << dz << " gpu=" << gpu
      << " best_cost=" << bc << " reason=" << reason;
    write("STALL", o.str());
  }

  void logState(const char *key, const std::string &val)
  {
    write("STATE", std::string(key) + "=" + val);
  }

private:
  std::ofstream ofs_;
  std::mutex mutex_;
  std::chrono::steady_clock::time_point start_time_;

  double elapsed()
  {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now - start_time_).count();
  }

  void write(const char *, const std::string &line)
  {
    ofs_ << elapsed() << " " << line << "\n";
    ofs_.flush();
  }
};

}  // namespace nav2_custom_plugins
#endif
