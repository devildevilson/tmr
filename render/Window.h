/**
 * Класс окна, решил на этот раз сделать его в рендере, так как он принимает непосредственное участие в отрисовке 
 * 
 * Выяснил, что у меня никак не работает VK_KHR_display, от слова совсем =(
 * поэтому буду пользоваться glfw
 * 
 * Для систем с несколькими мониторами, необходимо запилить compositor pattern
 * видимо буду рисовать в обычные картинки, а потом часть копировать в разные окна
 * Хотя наверное я бы и так это делал (VK_KHR_display не запустился же)
 */

#ifndef WINDOW_H
#define WINDOW_H

// #define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "RenderTarget.h"

#include "Core.h"

#define FAR_CLIPPING 256.0f

struct Swapchain {
//   VkSwapchainKHR handle = VK_NULL_HANDLE;
  yavf::Swapchain swapchain;
  std::vector<yavf::Image*> images;
//   std::vector<VkImageView> imageViews;
  std::vector<yavf::DescriptorSet*> sets;
};

struct SurfaceData {
  VkSurfaceKHR handle = VK_NULL_HANDLE;
  VkSurfaceCapabilitiesKHR capabilities;
  VkSurfaceFormatKHR format;
  VkPresentModeKHR presentMode;
  VkExtent2D extent;
};

struct VirtualFrame {
  yavf::Semaphore   imageAvailableSemaphore = VK_NULL_HANDLE;
  VkPipelineStageFlags flags;
  yavf::Semaphore   finishedRenderingSemaphore = VK_NULL_HANDLE;
};

struct MonitorInfo {
  GLFWmonitor* handle = nullptr;
  GLFWwindow* window = nullptr;
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  // здесь нужно наверное будет добавить устройство
  // хотя наверное несовсем здесь
  // все же у двух мониторов может быть одно устройство (чаще всего именно так)
};

class VulkanRender;

// : public yavf::RenderTarget
class WindowInterface {
public:
  WindowInterface();
  virtual ~WindowInterface() {}

  virtual void addMonitor(MonitorInfo info) = 0;
//   virtual void create(VulkanRender* render, uint32_t width, uint32_t height) = 0;

  virtual void nextFrame() = 0;
  virtual void present() = 0;

  virtual void resize() = 0;
  virtual void fullscreen() = 0;
  virtual void noFrame() = 0;

  virtual bool isFullscreen() const = 0;
  virtual bool isNoFrame() const = 0;
  virtual bool shouldClose() const = 0;

  virtual void hide() = 0;
  virtual void show() = 0;
  
  virtual yavf::Image* getImage() const = 0;
  virtual yavf::Image* getDepth() const = 0;
  
  size_t refreshTime();

  void setIconify(const int &i);
  void setFocus(const int &f);
  bool isIconified() const;
  bool isFocused() const;
protected:
  bool iconified;
  bool focused;
  size_t refreshTimeVar;
};

class WindowRenderTarget : public yavf::RenderTarget {
public:
  WindowRenderTarget();
  ~WindowRenderTarget();
  
  std::vector<VkClearValue> clearValues() const override;
  VkRect2D size() const override;
  yavf::RenderPass renderPass() const override;
  VkViewport viewport() const override;
  VkRect2D scissor() const override;
  yavf::Framebuffer framebuffer() const override;
  
  void set(const VkRect2D &windowSize, const yavf::RenderPass &renderPassHandle, const yavf::Framebuffer &buffer, std::vector<VkClearValue>* clearValuePtr);
private:
  VkRect2D windowSize;
  yavf::RenderPass renderPassHandle;
  yavf::Framebuffer buffer;
  std::vector<VkClearValue>* clearValuePtr;
};

class Window : public WindowInterface {
public:
  struct CreateInfo {
    yavf::Instance* inst;
    yavf::Device* device;
    GLFWmonitor* monitor;
    GLFWwindow* window;
    VkSurfaceKHR surface;
    
    //VulkanRender* render;
    
    float fov;
    bool fullscreen;
//     bool hidden;
  };
  
  Window();
  Window(const CreateInfo &info);
//   Window(const float &fov);
  virtual ~Window();

  void addMonitor(MonitorInfo info) override;
//   void create(VulkanRender* render, uint32_t width, uint32_t height) override;
  void create(const CreateInfo &info);
  
  VirtualFrame & at(const size_t &index);
  const VirtualFrame & at(const size_t &index) const;

  uint32_t framesCount() const;
  
  uint32_t currentFrame() const; // по этому индексу доступны семафоры, скорее всего придется менять их каждый кадр, хотя если только обновлять RenderTarget, то может и нет
  uint32_t currentImage() const; // по этому индексу у нас возвращается RenderTarget*, и его же мы должны использовать для командного буфера
  yavf::RenderTarget* currentRenderTarget();
  
  void nextFrame() override;
  void present() override;

  void resize() override;
  void fullscreen() override;
  void noFrame() override;

  GLFWwindow* handle() const;
  bool isFullscreen() const override;
  bool isNoFrame() const override;
  bool shouldClose() const override;
  void toggleVsync();
  bool isVsync() const;
  bool hasImmediatePresentMode() const;

  void hide() override;
  void show() override;

  void setFov(const float &fov);
  float getFov() const;

  void setRender(VulkanRender* render);

  uint32_t getFamily() const;
  uint32_t getFrameCount() const;
  
  yavf::Image* getImage() const override;
  yavf::Image* getDepth() const override;
  
  VkRect2D size() const;
  
  yavf::RenderPass pass() const;
private:
  void createRenderPass();

  bool vsync;
  bool immediatePresentMode;
  VkPresentModeKHR lastMode;
  bool fullscreenMode;
  bool windowedNoFrame;
  bool iconified;
  bool focused;
  VkRect2D windowSize;
  float fov;

  uint32_t imageIndex, currentVirtualFrame, family;

  yavf::Instance* instance;
  yavf::Device* device;
  std::vector<yavf::Image*> depthImages;
  GLFWmonitor* monitor;
  GLFWwindow* window;
  VulkanRender* render;
  yavf::RenderPass renderPassHandle;
  
//   yavf::SemaphoreOwner* owner = nullptr;
//   std::vector<yavf::SemaphoreProxy*> proxies;

//   VkViewport view;
//   VkRect2D scis;
  Swapchain swapchain;
  SurfaceData surface;
  std::vector<VirtualFrame> frames;
  std::vector<WindowRenderTarget> targets;
  std::vector<VkClearValue> values;
};

#endif // !WINDOW_H 
