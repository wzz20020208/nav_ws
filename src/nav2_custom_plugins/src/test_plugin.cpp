#include <iostream>
#include <pluginlib/class_loader.hpp>
#include <nav2_core/controller.hpp>

int main()
{
  std::cout << "正在扫描环境中的 Nav2 局部规划器插件...\n" << std::endl;

  try {
    // 实例化一个专门寻找 "nav2_core::Controller" 接口的加载器
    pluginlib::ClassLoader<nav2_core::Controller> loader("nav2_core", "nav2_core::Controller");

    // 获取所有已注册的类名
    auto classes = loader.getDeclaredClasses();
    
    std::cout << "--- 已发现以下插件 ---" << std::endl;
    for (const auto & c : classes) {
      if (c.find("nav2_custom_plugins") != std::string::npos) {
        std::cout << "✅ [命中目标] -> " << c << std::endl;
      } else {
        std::cout << "   " << c << std::endl;
      }
    }
  } catch(pluginlib::PluginlibException& ex) {
    std::cout << "加载器初始化失败: " << ex.what() << std::endl;
  }

  return 0;
}
