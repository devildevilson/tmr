#include "Helper.h"

#include <algorithm>

static const std::vector<const char*> instanceLayers = {
  "VK_LAYER_LUNARG_standard_validation",
  "VK_LAYER_LUNARG_api_dump",
  "VK_LAYER_LUNARG_assistant_layer"
};

void clipbardPaste(nk_handle usr, nk_text_edit *edit) {
    const char *text = glfwGetClipboardString(reinterpret_cast<Window*>(usr.ptr)->handle());
    
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
}

void clipbardCopy(nk_handle usr, const char *text, const int len) {
  if (len == 0) return;
  
  char str[len+1];
  memcpy(str, text, len);
  str[len] = '\0';
  
  glfwSetClipboardString(reinterpret_cast<Window*>(usr.ptr)->handle(), str);
}

KeyConfiguration::KeyConfiguration(const KeyConfiguration &copy) {
  cont = copy.cont;
}

KeyConfiguration::KeyConfiguration(const uint32_t &key1) {
  cont = (uint64_t(key1) << UINT16_WIDTH*0) | (uint64_t(UINT16_MAX) << UINT16_WIDTH*1) | (uint64_t(UINT16_MAX) << UINT16_WIDTH*2) | (uint64_t(UINT16_MAX) << UINT16_WIDTH*3);
}

KeyConfiguration::KeyConfiguration(const uint32_t &key1, const uint32_t &key2) {
  cont = (uint64_t(key1) << UINT16_WIDTH*0) | (uint64_t(key2) << UINT16_WIDTH*1) | (uint64_t(UINT16_MAX) << UINT16_WIDTH*2) | (uint64_t(UINT16_MAX) << UINT16_WIDTH*3);
}

KeyConfiguration::KeyConfiguration(const uint32_t &key1, const uint32_t &key2, const uint32_t &key3) {
  cont = (uint64_t(key1) << UINT16_WIDTH*0) | (uint64_t(key2) << UINT16_WIDTH*1) | (uint64_t(key3) << UINT16_WIDTH*2) | (uint64_t(UINT16_MAX) << UINT16_WIDTH*3);
}

KeyConfiguration::KeyConfiguration(const uint32_t &key1, const uint32_t &key2, const uint32_t &key3, const uint32_t &key4) {
  cont = (uint64_t(key1) << UINT16_WIDTH*0) | (uint64_t(key2) << UINT16_WIDTH*1) | (uint64_t(key3) << UINT16_WIDTH*2) | (uint64_t(key4) << UINT16_WIDTH*3);
}

uint32_t KeyConfiguration::getFirstKey() const {
  const uint16_t tmp = (cont >> UINT16_WIDTH*0) | UINT16_MAX;
  return tmp == UINT16_MAX ? UINT32_MAX : tmp;
}

uint32_t KeyConfiguration::getSecondKey() const {
  const uint16_t tmp = (cont >> UINT16_WIDTH*1) | UINT16_MAX;
  return tmp == UINT16_MAX ? UINT32_MAX : tmp;
}

uint32_t KeyConfiguration::getThirdKey() const {
  const uint16_t tmp = (cont >> UINT16_WIDTH*2) | UINT16_MAX;
  return tmp == UINT16_MAX ? UINT32_MAX : tmp;
}

uint32_t KeyConfiguration::getForthKey() const {
  const uint16_t tmp = (cont >> UINT16_WIDTH*3) | UINT16_MAX;
  return tmp == UINT16_MAX ? UINT32_MAX : tmp;
}

uint32_t KeyConfiguration::getKeysCount() const {
  uint32_t count = 0;
  for (uint8_t i = 0; i < 4; ++i) {
    const uint16_t tmp = uint16_t(cont >> UINT16_WIDTH*i); // | UINT16_MAX
    count += uint32_t(tmp != UINT16_MAX); //  ? count : count + 1;
  }
  
  return count;
}

uint32_t KeyConfiguration::operator[] (const uint8_t &index) const {
  const uint16_t tmp = (cont >> UINT16_WIDTH*index); // | UINT16_MAX
  return tmp == UINT16_MAX ? UINT32_MAX : tmp;
}

Reaction::Reaction() {}
Reaction::Reaction(const std::string &name, const std::function<void()> &f) : name(name), f(f) {}

ActionKey::ActionKey(const KeyConfiguration &keys, const std::vector<ActionKey*> &keysPtr) 
  : handled(false), 
    keys(keys), 
    keysPtr(keysPtr), 
    commands({nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}), 
    time(0), 
    currentState(UNKNOWN) {
//   std::cout << "keysCount " << keys.getKeysCount() << "\n";
  
  const uint32_t count = keys.getKeysCount();
  if (count == 0) throw std::runtime_error("Key count == 0");
  if (count != 1 && count != keysPtr.size()) throw std::runtime_error("Wrong key number");
  //if (count > 1 && count == keysPtr.size())
}

void ActionKey::execute(const int32_t &state, const uint64_t &_time) {
  if (handled) {
    handled = !handled;
    return;
  }
  
  time += _time;
  bool changed = false;
  if (state == GLFW_PRESS || state == GLFW_REPEAT) {
    if (currentState == CLICK) {
      if (time < DOUBLE_PRESS_TIME) {
        currentState = DOUBLE_PRESS;
        changed = true;
      } else {
        currentState = PRESS;
        time = 0;
        changed = true;
      }
    } else if (currentState == PRESS) {
      if (time > LONG_PRESS_TIME) {
        currentState = PRESS;
        changed = true;
      } else {
        currentState = PRESS;
        changed = true;
      }
    } else if ((currentState == DOUBLE_CLICK) || (currentState == LONG_CLICK) || (currentState == UNKNOWN)) {
      currentState = PRESS;
      time = 0;
      changed = true;
    } else if (currentState == LONG_PRESS) {
      changed = true;
    } else if (currentState == DOUBLE_PRESS) {
      if (time > DOUBLE_CLICK_TIME) {
        currentState = PRESS;
        time = 0;
        changed = true;
      } else {
        changed = true;
      }
    }
  } else if (state == GLFW_RELEASE) {
    if (currentState == PRESS) {
      currentState = CLICK;
      changed= true;
      //std::cout << "clicked " << glfwGetKeyName(*key, 0) << std::endl;
    } else if (currentState == DOUBLE_PRESS) {
      if (time < DOUBLE_CLICK_TIME) {
        currentState = DOUBLE_CLICK;
        changed = true;
        //std::cout << "double clicked " << glfwGetKeyName(*key, 0) << std::endl;
      } else {
        currentState = CLICK;
        time = 0;
        changed = true;
      }
    } else if (currentState == LONG_PRESS) { // наверное long click будет происходить сразу же после long press
      if (time > LONG_CLICK_TIME) {
        currentState = LONG_CLICK;
        changed = true;
        //std::cout << "long clicked " << glfwGetKeyName(*key, 0) << std::endl;
      } else {
        currentState = CLICK;
        time = 0;
        changed = true;
      }
    }
  }
  
  if (changed && commands[currentState] != nullptr) {
    commands[currentState]->f();
    
    for (uint8_t i = 0; i < keysPtr.size(); ++i) keysPtr[i]->setHandled();
  }
}

void ActionKey::setHandled() {
  handled = true;
}

Reaction* ActionKey::getReaction(uint8_t i) const {
  return commands[i];
}

void ActionKey::setReaction(const uint8_t &index, Reaction* r) {
  commands[index] = r;
}

uint32_t ActionKey::getKey(const uint8_t &index) const {
  return keys[index];
}

uint32_t ActionKey::getKeysCount() const {
  return keys.getKeysCount();
}

KeyContainer::KeyContainer(KeyConfig* config) : config(config) {}

KeyContainer::~KeyContainer() {
  for (size_t i = 0; i < config->keys.size(); ++i) {
    keysPool.deleteElement(config->keys[i]);
  }
  
  for (auto itr = config->reactions.begin(); itr != config->reactions.end(); ++itr) {
    reactionPool.deleteElement(itr->second);
  }
}

ActionKey* KeyContainer::create(const KeyConfiguration &keys, const std::vector<ActionKey*> &keysPtr) {
  ActionKey* key = keysPool.newElement(keys, keysPtr);
  config->keys.push_back(key);
  
  return key;
}

struct KeyCompare {
  bool operator() (const ActionKey* first, const ActionKey* second) const {
    return first->getKeysCount() > second->getKeysCount();
  }
};

void KeyContainer::sort() {
  std::sort(config->keys.begin(), config->keys.end(), KeyCompare());
}

Reaction* KeyContainer::create(const std::string &name, const std::function<void()> &f) {
  Reaction* react = reactionPool.newElement(name, f);
  config->reactions[name] = react;
  
  return react;
}

void initGLFW() {
  if (glfwInit() != GLFW_TRUE) {
    Global::console()->printE("Cannot init glfw!");
    throw std::runtime_error("Cannot init glfw!");
  }

  if (glfwVulkanSupported() != GLFW_TRUE) {
    Global::console()->printE("Vulkan is not supported!");
    throw std::runtime_error("Vulkan is not supported!");
  }

  glfwSetErrorCallback(callback);
}

void deinitGLFW() {
  glfwTerminate();
}

void initnk(yavf::Device* device, Window* window, nuklear_data &data) {
  glfwSetKeyCallback(window->handle(), keyCallback);
  glfwSetCharCallback(window->handle(), charCallback);
  glfwSetMouseButtonCallback(window->handle(), mouseButtonCallback);
  glfwSetScrollCallback(window->handle(), scrollCallback);
  glfwSetWindowFocusCallback(window->handle(), focusCallback);
  glfwSetWindowIconifyCallback(window->handle(), iconifyCallback);
  
  nk_buffer_init_default(&data.cmds);
  
  {
    const void *image; 
    int w, h;
    nk_font_atlas_init_default(&data.atlas);
    nk_font_atlas_begin(&data.atlas);
    data.font = nk_font_atlas_add_default(&data.atlas, 13.0f, NULL);
    image = nk_font_atlas_bake(&data.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
//     device_upload_atlas(&device, image, w, h); // загрузить текстуру
    yavf::Image* img = nullptr;
    yavf::ImageView* view = nullptr;
    {
      auto staging = device->create(yavf::ImageCreateInfo::texture2DStaging({static_cast<uint32_t>(w), static_cast<uint32_t>(h)}), VMA_MEMORY_USAGE_CPU_ONLY);
      
      const size_t imageSize = w * h * 4;
      memcpy(staging->ptr(), image, imageSize);
      
      img = device->create(yavf::ImageCreateInfo::texture2D({static_cast<uint32_t>(w), static_cast<uint32_t>(h)}, 
                                                            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), 
                           VMA_MEMORY_USAGE_GPU_ONLY);
      
      yavf::TransferTask* task = device->allocateTransferTask();
      
      task->begin();
      task->setBarrier(staging, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
      task->setBarrier(img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      task->copy(staging, img);
      task->setBarrier(img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      task->end();
      
      task->start();
      task->wait();
      
      device->deallocate(task);
      device->destroy(staging);
      
      view = img->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
      
      // тут нужен еще сэмплер и дескриптор
      yavf::Sampler sampler;
      {
        yavf::SamplerMaker sm(device);
        
        sampler = sm.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT)
                    .anisotropy(0.0f)
                    .borderColor(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK)
                    .compareOp(VK_FALSE, VK_COMPARE_OP_GREATER)
                    .filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
                    .lod(0.0f, 1.0f)
                    .mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
                    .unnormalizedCoordinates(VK_FALSE)
                    .create("default_nuklear_sampler");
                         
//         img->setSampler(sampler);
      }
      
      {
        yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
        yavf::DescriptorSetLayout sampled_image_layout = device->setLayout(SAMPLED_IMAGE_LAYOUT_NAME);
        {
          yavf::DescriptorLayoutMaker dlm(device);
          
          if (sampled_image_layout == VK_NULL_HANDLE) {
            sampled_image_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).create(SAMPLED_IMAGE_LAYOUT_NAME);
          }
        }
        
        yavf::DescriptorMaker dm(device);
        
        auto d = dm.layout(sampled_image_layout).create(pool)[0];
        
        const size_t i = d->add({sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
        view->setDescriptor(d, i);
      }
    }
    
    // сюда мы по всей видимости передаем указатель на картинку
    nk_font_atlas_end(&data.atlas, nk_handle_ptr(view), &data.null);
  }
  
  nk_init_default(&data.ctx, &data.font->handle);
  
  data.ctx.clip.copy = clipbardCopy;
  data.ctx.clip.paste = clipbardPaste;
  data.ctx.clip.userdata = nk_handle_ptr(window);
}

void deinitnk(nuklear_data &data) {
  nk_font_atlas_clear(&data.atlas);
  nk_buffer_free(&data.cmds);
  nk_free(&data.ctx);
}

void nextnkFrame(Window* window, nk_context* ctx) {
  double x, y;
  int widht, height, display_width, display_height;
  glfwGetWindowSize(window->handle(), &widht, &height);
  glfwGetFramebufferSize(window->handle(), &display_width, &display_height);
  Global::data()->fbScaleX = float(display_width / widht);
  Global::data()->fbScaleY = float(display_height / height);
  
  if (Global::data()->focusOnInterface) {
    nk_input_begin(ctx);
    // nk_input_unicode нужен для того чтобы собирать набранный текст
    // можем ли мы воспользоваться им сразу из коллбека?
    // по идее можем, там не оч сложные вычисления
    //nk_input_unicode(ctx, 'f');
    
    for (uint32_t i = 0; i < Global::data()->currentText; ++i) {
      nk_input_unicode(ctx, Global::data()->text[i]);
    }
    
    const bool* keys = Global::data()->keys;
    nk_input_key(ctx, NK_KEY_DEL, keys[GLFW_KEY_DELETE]);
    nk_input_key(ctx, NK_KEY_ENTER, keys[GLFW_KEY_ENTER]);
    nk_input_key(ctx, NK_KEY_TAB, keys[GLFW_KEY_TAB]);
    nk_input_key(ctx, NK_KEY_BACKSPACE, keys[GLFW_KEY_BACKSPACE]);
    nk_input_key(ctx, NK_KEY_UP, keys[GLFW_KEY_UP]);
    nk_input_key(ctx, NK_KEY_DOWN, keys[GLFW_KEY_DOWN]);
    nk_input_key(ctx, NK_KEY_SHIFT, keys[GLFW_KEY_LEFT_SHIFT] ||
                                    keys[GLFW_KEY_RIGHT_SHIFT]);

    if (keys[GLFW_KEY_LEFT_CONTROL] ||
      keys[GLFW_KEY_RIGHT_CONTROL]) {
      nk_input_key(ctx, NK_KEY_COPY, keys[GLFW_KEY_C]);
      nk_input_key(ctx, NK_KEY_PASTE, keys[GLFW_KEY_V]);
      nk_input_key(ctx, NK_KEY_CUT, keys[GLFW_KEY_X]);
      nk_input_key(ctx, NK_KEY_TEXT_UNDO, keys[GLFW_KEY_Z]);
      nk_input_key(ctx, NK_KEY_TEXT_REDO, keys[GLFW_KEY_R]);
      nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, keys[GLFW_KEY_LEFT]);
      nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, keys[GLFW_KEY_RIGHT]);
      nk_input_key(ctx, NK_KEY_TEXT_SELECT_ALL, keys[GLFW_KEY_A]);
    
      nk_input_key(ctx, NK_KEY_SCROLL_START, keys[GLFW_KEY_PAGE_DOWN]);
      nk_input_key(ctx, NK_KEY_SCROLL_END, keys[GLFW_KEY_PAGE_UP]);
      nk_input_key(ctx, NK_KEY_TEXT_START, keys[GLFW_KEY_HOME]);
      nk_input_key(ctx, NK_KEY_TEXT_END, keys[GLFW_KEY_END]);
    } else {
      nk_input_key(ctx, NK_KEY_LEFT, keys[GLFW_KEY_LEFT]);
      nk_input_key(ctx, NK_KEY_RIGHT, keys[GLFW_KEY_RIGHT]);
      nk_input_key(ctx, NK_KEY_COPY, 0);
      nk_input_key(ctx, NK_KEY_PASTE, 0);
      nk_input_key(ctx, NK_KEY_CUT, 0);
      nk_input_key(ctx, NK_KEY_SHIFT, 0);
      
      nk_input_key(ctx, NK_KEY_SCROLL_DOWN, keys[GLFW_KEY_PAGE_DOWN]);
      nk_input_key(ctx, NK_KEY_SCROLL_UP, keys[GLFW_KEY_PAGE_UP]);
      nk_input_key(ctx, NK_KEY_TEXT_LINE_START, keys[GLFW_KEY_HOME]);
      nk_input_key(ctx, NK_KEY_TEXT_LINE_END, keys[GLFW_KEY_END]);
    }

    glfwGetCursorPos(Global::window()->handle(), &x, &y);
    if (Global::window()->isFocused()) {
      // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
      nk_input_motion(ctx, (int)x, (int)y);
    } else {
      nk_input_motion(ctx, -1, -1);
    }
    
    // тоже заменить, также наклир дает возможность обработать даблклик, как это сделать верно?
    nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, keys[GLFW_MOUSE_BUTTON_LEFT]);
    nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, keys[GLFW_MOUSE_BUTTON_MIDDLE]);
    nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, keys[GLFW_MOUSE_BUTTON_RIGHT]);
    
    bool doubleClick = false;
    if (keys[GLFW_MOUSE_BUTTON_LEFT]) {
      auto p = std::chrono::steady_clock::now();
      
      auto diff = p - Global::data()->doubleClickPoint;
      auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
      
      Global::data()->doubleClickPoint = p;
      doubleClick = mcs < DOUBLE_CLICK_TIME;
    }
    
    // не понимаю че делать с двойным нажатием
    // точнее мне для этого нужно время
    nk_input_button(ctx, NK_BUTTON_DOUBLE, int(Global::data()->clickPos.x), int(Global::data()->clickPos.y), doubleClick);
    nk_input_scroll(ctx, nk_vec2(Global::data()->mouseWheel, 0.0f));
    nk_input_end(ctx);
    // обнуляем
    Global::data()->clickPos = glm::uvec2(x, y);
    Global::data()->currentText = 0;
    Global::data()->mouseWheel = 0.0f;
    
    glfwSetInputMode(window->handle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  } else {
    glfwSetInputMode(window->handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
  
  // короче отрисовка наклира выглядит очень похоже на imgui
}

void nkOverlay(const SimpleOverlayData &data, nk_context* ctx) {
  nk_style* s = &ctx->style;
  nk_color* oldColor = &s->window.background;
  nk_style_item* oldStyleItem = &s->window.fixed_background;
  nk_style_push_color(ctx, oldColor, nk_rgba(oldColor->r, oldColor->g, oldColor->b, int(0.5f*255)));
  nk_style_push_style_item(ctx, oldStyleItem, nk_style_item_color(nk_rgba(oldStyleItem->data.color.r, oldStyleItem->data.color.g, oldStyleItem->data.color.b, int(0.5f*255))));
  
  if (nk_begin(ctx, "Basic window", nk_rect(10, 10, 300, 240),
        NK_WINDOW_NO_SCROLLBAR)) {
    {
      const glm::vec4 &pos = glm::vec4(data.pos);
      const auto &str = fmt::sprintf("Camera pos: (%.2f,%.2f,%.2f,%.2f)", pos.x, pos.y, pos.z, pos.w);
      
      nk_layout_row_static(ctx, 30.0f, 300, 1); // ряд высотой 30, каждый элемент шириной 300, 1 столбец
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT); // nk_layout_row_static скорее всего нужно указывать каждый раз
    }
    
    {
      const glm::vec3 &dir = data.rot;
      const auto &str = fmt::sprintf("Camera dir: (%.2f,%.2f,%.2f)", dir.x, dir.y, dir.z);
      
      nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }
    
    {
      const auto &str = fmt::sprintf("Frame rendered in %lu mcs (%.2f fps)", data.frameComputeTime, 1000000.0f/float(data.frameComputeTime));
      
      nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }
    
    {
      const auto &str = fmt::sprintf("Sleep between frames equals %lu mcs", data.frameSleepTime);
      
      nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }
    
    {
      const auto &str = fmt::sprintf("Final fps is %.2f", data.fps);
      
      nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }
    
    {
      const auto &str = fmt::sprintf("In frustum %zu objects", data.frustumObjCount);
      
      nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }
    
    {
      const auto &str = fmt::sprintf("Ray collide %zu objects", data.rayCollideCount);
      
      nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }
    
    {
      const auto &str = fmt::sprintf("Player see %zu objects", data.visibleObjCount);
      
      nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }
  }
  nk_end(ctx);
  
  nk_style_pop_color(ctx);
  nk_style_pop_style_item(ctx);
}

// void initGui(yavf::Device* device, Window* window) {
//   ImGuiIO& io = ImGui::GetIO();
//   io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
//   io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
//   io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
//   io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
//   io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
//   io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
//   io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
//   io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
//   io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
//   io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
//   io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
//   io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
//   io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
//   io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
//   io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
//   io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
//   io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
//   io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
//   io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
// 
//   io.SetClipboardTextFn = setClipboard;
//   io.GetClipboardTextFn = getClipboard;
// 
//   io.RenderDrawListsFn = nullptr; // рендер имплементирован с использованием ImGui::GetDrawData()
//   io.ClipboardUserData = window;
//   io.DeltaTime = 0.0f;
// 
//   glfwSetKeyCallback(window->handle(), keyCallback);
//   glfwSetCharCallback(window->handle(), charCallback);
//   glfwSetMouseButtonCallback(window->handle(), mouseButtonCallback);
//   glfwSetScrollCallback(window->handle(), scrollCallback);
//   glfwSetWindowFocusCallback(window->handle(), focusCallback);
//   glfwSetWindowIconifyCallback(window->handle(), iconifyCallback);
// 
//   unsigned char* pixels;
//   int width, height;
//   io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
//   
//   yavf::Image* image = nullptr;
//   {
//     yavf::Image* staging = nullptr;
//     {
//       const yavf::ImageCreateInfo info{
//         0,
//         VK_IMAGE_TYPE_2D,
//         VK_FORMAT_R8G8B8A8_UNORM,
//         {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
//         1,
//         1,
//         VK_SAMPLE_COUNT_1_BIT,
//         VK_IMAGE_TILING_LINEAR,
//         VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
//         VK_IMAGE_ASPECT_COLOR_BIT,
//         VMA_MEMORY_USAGE_CPU_ONLY
//       };  
//       staging = device->createImage(info);
//       
//       memcpy(staging->ptr(), pixels, width*height*4*sizeof(char));
//     }
//     
//     {
//       const yavf::ImageCreateInfo info{
//         0,
//         VK_IMAGE_TYPE_2D,
//         VK_FORMAT_R8G8B8A8_UNORM,
//         {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
//         1,
//         1,
//         VK_SAMPLE_COUNT_1_BIT,
//         VK_IMAGE_TILING_OPTIMAL,
//         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//         VK_IMAGE_ASPECT_COLOR_BIT,
//         VMA_MEMORY_USAGE_GPU_ONLY
//       };
//       image = device->createImage(info);
//     }
//     
//     yavf::TransferTask* task = device->allocateTransferTask();
//     
//     task->begin();
//     task->setBarrier(staging, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
//     task->setBarrier(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//     task->copy(staging, image);
//     task->setBarrier(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//     task->end();
//     
//     task->start();
//     task->wait();
//     
//     device->deallocate(task);
//     device->destroy(staging);
//   }
//   
//   image->createView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
//   image->setSampler(device->sampler("default_sampler"));
//   
//   {
//     yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
//     yavf::DescriptorSetLayout sampled_image_layout = device->setLayout(SAMPLED_IMAGE_LAYOUT_NAME);
//     {
//       yavf::DescriptorLayoutMaker dlm(device);
//       
//       if (sampled_image_layout == VK_NULL_HANDLE) {
//         sampled_image_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).create(SAMPLED_IMAGE_LAYOUT_NAME);
//       }
//     }
//     
//     yavf::DescriptorMaker dm(device);
//     
//     auto d = dm.layout(sampled_image_layout).create(pool)[0];
//     
//     const yavf::DescriptorUpdate data{
//       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
//       0, 
//       0,
//       d
//     };
//     image->setDescriptorData(data);
//   }
//   
//   // Store our identifier
//   io.Fonts->TexID = (void*)image;
// }

void simpleOverlay(const SimpleOverlayData &data) {
//   bool open = true;
  {
    ImGui::SetNextWindowPos(ImVec2(10,10));
    bool t = ImGui::Begin("Example: Fixed Overlay",
                      nullptr,
                      ImVec2(0,0),
                      0.5f,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
    if (!t) {
        ImGui::End();
        return;
    }
    ImGui::Text("Simple overlay\non the top-left side of the screen.");
    ImGui::Separator();

    const glm::vec4 &pos = glm::vec4(data.pos); //playerTransform->pos;
    ImGui::Text("Camera pos: (%.2f,%.2f,%.2f,%.2f)", pos.x, 
                                                      pos.y, 
                                                      pos.z,
                                                      pos.w);
    
    const glm::vec3 &dir = data.rot;//GameObjectContainer::getObject(0)->getRot();
    ImGui::Text("Camera dir: (%.2f,%.2f,%.2f)", dir.x, 
                                                dir.y, 
                                                dir.z);
    
    ImGui::Separator();
    ImGui::Text("Frame rendered in %lu mcs (%.2f fps)", data.frameComputeTime, 1000000.0f/float(data.frameComputeTime));
    ImGui::Text("Sleep between frames equals %lu mcs", data.frameSleepTime);
    ImGui::Text("Final fps is %.2f", data.fps);
    ImGui::Separator();
    ImGui::Text("In frustum %zu objects", data.frustumObjCount);
    //ImGui::Text("Ray collide %zu objects", collideRay.size());
    ImGui::Text("Ray collide %zu objects", data.rayCollideCount);
    //ImGui::Text("Player see %zu objects", visibleObjects);
    ImGui::Text("Player see %zu objects", data.visibleObjCount);
//       ImGui::Text("Visible geo count: %zu", geometryCount);
//       ImGui::Text("Visible obj count: %zu", objCount);
    //if (!collideRay.empty()) ImGui::Text("Ray collide object index %zu", collideRay.back()->index);
    ImGui::End();
  }
}

// void createInstance(yavf::Instance* inst) {
//   uint32_t count;
//   const char** ext = glfwGetRequiredInstanceExtensions(&count);
//   if (count == 0) {
//     Global::console()->print("Found no extensions\n");
//     throw std::runtime_error("Extensions not founded!");
//   }
//   
//   std::vector<const char*> extensions;
//   for (uint32_t i = 0; i < count; ++i) {
//     extensions.push_back(ext[i]);
//   }
//   
// //   yavf::Instance::setExtensions(extensions);
// //   yavf::Instance::setLayers({
// //     "VK_LAYER_LUNARG_standard_validation",
// //     "VK_LAYER_LUNARG_api_dump",
// //     "VK_LAYER_LUNARG_assistant_layer"
// //   });
//   
//   const yavf::Instance::ApplicationInfo appInfo{
//     APPLICATION_NAME,
//     APP_VERSION,
//     ENGINE_NAME,
//     EGINE_VERSION,
//     VK_API_VERSION_1_0
//   };
//   
//   const yavf::Instance::CreateInfo info{
//     nullptr,
//     &appInfo,
//     instanceLayers,
//     extensions,
//     true,
//     false,
//     false
//   };
//   
//   inst->construct(info);
// }
// 
// void createGLFWwindow(yavf::Instance* inst, WindowData &data) {
// //   const bool fullscreen = false;
//   const bool fullscreen = Global::settings()->get<int64_t>("game.graphics.fullscreen");
// //   uint32_t width = 1280;
// //   uint32_t height = 720;
//   uint32_t width = Global::settings()->get<int64_t>("game.graphics.width");
//   uint32_t height = Global::settings()->get<int64_t>("game.graphics.height");
// //   const float fov = 60.0f;
//   const float fov = Global::settings()->get<float>("game.graphics.fov");
//   GLFWmonitor* monitor = nullptr;
//   GLFWwindow* glfwWindow = nullptr;
//   VkSurfaceKHR surface = VK_NULL_HANDLE;
//   
//   if (fullscreen) {
//     monitor = glfwGetPrimaryMonitor();
//     
//     int32_t count;
//     auto monitors = glfwGetMonitors(&count);
//     for (int32_t i = 0; i < count; ++i) {
//       std::cout << "Monitor name: " << glfwGetMonitorName(monitors[i]) << "\n";
//       int32_t x, y;
//       glfwGetMonitorPhysicalSize(monitors[i], &x, &y);
//       std::cout << "Monitor phys size: x " << x << " y " << y << "\n";
//       glfwGetMonitorPos(monitors[i], &x, &y);
//       std::cout << "Monitor pos: x " << x << " y " << y << "\n";
//     }
//     
//     const auto data = glfwGetVideoMode(monitor);
//     width = data->width;
//     height = data->height;
//     
//     Global::settings()->get<int64_t>("game.graphics.width") = data->width;
//     Global::settings()->get<int64_t>("game.graphics.height") = data->height;
//     
//     // в какой то момент мне может потребоваться даунсэмплить изображение чтоб зернистость появилась
//     // возможно это делается другим способом
//   }
//   
//   glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
//   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//   if (fullscreen) {
//     glfwWindow = glfwCreateWindow(width, height, APPLICATION_NAME, monitor, nullptr);
//   } else {
//     glfwWindow = glfwCreateWindow(width, height, APPLICATION_NAME, nullptr, nullptr);
//   }
//   
//   yavf::vkCheckError("glfwCreateWindowSurface", nullptr, 
//   glfwCreateWindowSurface(inst->handle(), glfwWindow, nullptr, &surface));
//   
//   data.fullscreen = fullscreen;
//   data.width = width;
//   data.height = height;
//   data.fov = fov;
//   data.monitor = monitor;
//   data.glfwWindow = glfwWindow;
//   data.surface = surface;
// }
// 
// void createDevice(yavf::Instance* inst, const WindowData &data, yavf::Device** device) {
//   const std::vector<const char*> deviceExtensions = {
//     VK_KHR_SWAPCHAIN_EXTENSION_NAME
//   };
//   
//   VkSurfaceKHR s = data.surface;
//   
//   auto physDevices = inst->getPhysicalDevices();
//   
//   // как выбирать устройство? 
//   uint32_t index = 0;
//   size_t maxMem = 0;
//   
//   yavf::PhysicalDevice choosen = VK_NULL_HANDLE;
//   for (size_t i = 0; i < physDevices.size(); ++i) {
//     VkPhysicalDeviceProperties deviceProperties;
//     VkPhysicalDeviceFeatures deviceFeatures;
//     VkPhysicalDeviceMemoryProperties memProp;
//     physDevices[i].getProperties(&deviceProperties);
//     physDevices[i].getFeatures(&deviceFeatures);
//     physDevices[i].getMemoryProperties(&memProp);
//     
//     size_t a = 0;
//     for (uint32_t j = 0; j < memProp.memoryHeapCount; ++j) {
//       if ((memProp.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
//         a = std::max(memProp.memoryHeaps[i].size, a);
//       }
//     }
// 
//     std::cout << "Device name: " << deviceProperties.deviceName << "\n";
//     
// 
//     bool extSupp = yavf::checkDeviceExtensions(physDevices[i], instanceLayers, deviceExtensions);
// 
//     uint32_t count = 0;
//     physDevices[i].getQueueFamilyProperties(&count, nullptr);
// 
//     bool presentOk = false;
//     for (uint32_t i = 0; i < count; ++i) {
//       VkBool32 present;
//       vkGetPhysicalDeviceSurfaceSupportKHR(physDevices[i], i, s, &present);
// 
//       if (present) {
//         presentOk = true;
//         break;
//       }
//     }
// 
//     if (extSupp && presentOk && maxMem < a) {
//       maxMem = a;
//       choosen = physDevices[i];
//       //break;
//     }
//   }
//   
// //   auto devices = inst->getDevices([s, deviceExtensions] (VkPhysicalDevice physDevice) -> bool {
// //     VkPhysicalDeviceProperties deviceProperties;
// //     VkPhysicalDeviceFeatures deviceFeatures;
// //     vkGetPhysicalDeviceProperties(physDevice, &deviceProperties);
// //     vkGetPhysicalDeviceFeatures(physDevice, &deviceFeatures);
// // 
// //     std::cout << "Device name: " << deviceProperties.deviceName << "\n";
// // 
// //     bool extSupp = yavf::checkDeviceExtensions(physDevice, yavf::Instance::getLayers(), deviceExtensions);
// // 
// //     uint32_t count = 0;
// //     vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &count, nullptr);
// // 
// //     bool presentOk = false;
// //     for (uint32_t i = 0; i < count; ++i) {
// //       VkBool32 present;
// //       vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, i, s, &present);
// // 
// //       if (present) {
// //         presentOk = true;
// //         break;
// //       }
// //     }
// // 
// //     return //deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
// //            extSupp &&
// //            presentOk;
// //   });
//   
//   yavf::DeviceMaker dm(inst);
// //   yavf::DeviceMaker::setExtensions(deviceExtensions);
//   VkPhysicalDeviceFeatures f = {};
//   f.samplerAnisotropy = VK_TRUE;
// //   f.multiDrawIndirect = VK_TRUE;
// //   f.drawIndirectFirstInstance = VK_TRUE;
// //   f.fragmentStoresAndAtomics = VK_TRUE;
//   //*device = dm.beginDevice(devices[0]).createQueues().features(f).create("Graphic device");
//   *device = dm.beginDevice(choosen).setExtensions(deviceExtensions).createQueues().features(f).create(instanceLayers, "Graphic device");
// }
// 
// void createWindow(yavf::Instance* inst, yavf::Device* device, const WindowData &data, Window &window) {
//   const Window::CreateInfo info{
//     inst,
//     device,
//     data.monitor,
//     data.glfwWindow,
//     data.surface,
//     data.fov,
//     data.fullscreen
//   };
// //   *window = new Window(info);
//   
//   window.create(info);
// }
// 
// // void destroyWindow(Window* window) {
// //   delete window;
// // }
// 
// void createRender(yavf::Instance* inst, yavf::Device* device, const uint32_t &frameCount, const size_t &stageContainerSize, GameSystemContainer &container, VulkanRender** render, yavf::CombinedTask** task) {
//   *task = device->allocateCombinedTask(frameCount);
//   
//   //const size_t stageContainerSize = sizeof(BeginTaskStage) + sizeof(EndTaskStage) + sizeof(GBufferStage) + sizeof(DefferedLightStage);
//   const VulkanRender::CreateInfo info{
//     inst,
//     device,
//     *task,
//     stageContainerSize
//   };
//   
//   *render = container.addSystem<VulkanRender>(info);
// }

// нужно не забыть переделать под гпу буферы!!!
void createDataArrays(yavf::Device* device, ArrayContainers &arraysContainer, DataArrays &arrays) {
  (void)device;
  
  arrays.externals = arraysContainer.add<CPUContainer<ExternalData>>();
  arrays.inputs = arraysContainer.add<CPUContainer<InputData>>();
  arrays.matrices = arraysContainer.add<CPUContainer<glm::mat4>>();
  arrays.rotationCountBuffer = arraysContainer.add<CPUBuffer<uint32_t>>();
  arrays.rotations = arraysContainer.add<CPUContainer<RotationData>>();
  arrays.transforms = arraysContainer.add<CPUContainer<Transform>>();
  arrays.textures = arraysContainer.add<CPUContainer<TextureData>>();
  arrays.animStates = arraysContainer.add<CPUContainer<AnimationState>>();
  arrays.broadphasePairs = arraysContainer.add<CPUArray<BroadphasePair>>();
  
  TransformComponent::setContainer(arrays.transforms);
  InputComponent::setContainer(arrays.inputs);
  GraphicComponent::setContainer(arrays.matrices);
  GraphicComponent::setContainer(arrays.rotations);
  GraphicComponent::setContainer(arrays.textures);
  PhysicsComponent2::setContainer(arrays.externals);
  AnimationComponent::setStateContainer(arrays.animStates);
}

// void destroyDataArrays(StageContainer &arraysContainer, DataArrays &arrays) {
//   arraysContainer.destroyStage(arrays.externals);
//   arraysContainer.destroyStage(arrays.inputs);
//   arraysContainer.destroyStage(arrays.matrices);
//   arraysContainer.destroyStage(arrays.rotationCountBuffer);
//   arraysContainer.destroyStage(arrays.rotations);
//   arraysContainer.destroyStage(arrays.transforms);
// }

void createRenderStages(const RenderConstructData &data, std::vector<DynamicPipelineStage*> &dynPipe) {
//   yavf::TaskInterface* interface = data.task[0];
  //data.render->addStage<BeginTaskStage>(reinterpret_cast<yavf::TaskInterface**>(data.task));
  data.render->addStage<BeginTaskStage>(data.container->tasks3());
  
  // короч для того чтобы перенести оптимизеры на гпу
  // мне нужно добавить много новых стейджев, может быть немного пересмотреть концепцию?
  // отдельно вытащить оптимизеры... создать отдельный стейдж с оптимизацией?
  // 
  
//   yavf::GraphicTask* graphicsTasks = data.task[0];
//   yavf::ComputeTask* computeTasks = data.task[0];
  
  const size_t gBufferStageContainerSize = sizeof(MonsterGBufferStage) + sizeof(GeometryGBufferStage);
  const GBufferStage::CreateInfo info{
    data.device,
    data.render->getCameraDataBuffer(),
    data.container->tasks2(), //reinterpret_cast<yavf::GraphicTask**>(data.task),// &graphicsTasks,
    data.window->size().extent.width,
    data.window->size().extent.height,
  
//       textureLoader->imageDescriptor(),
//       textureLoader->samplerDescriptor()
  };
  GBufferStage* gBuffer = data.render->addStage<GBufferStage>(gBufferStageContainerSize, info);
  dynPipe.push_back(gBuffer);
  
  auto monGbuffer = gBuffer->addPart<MonsterGBufferStage>(data.mon);
  auto geoGbuffer = gBuffer->addPart<GeometryGBufferStage>(data.mapLoader->mapVertices(), data.geo); // тут должен быть буфер карты, как его получить?
  
//   data.monDebugOpt->setInputBuffers({monGbuffer->getInstanceData()});
  data.geoDebugOpt->setInputBuffers({geoGbuffer->getInstanceData()});
  
//     gBuffer->recreatePipelines(textureLoader); // не тут это должно быть
  
  // нужно получить из деферед дескриптор
  const DefferedLightStage::CreateInfo dInfo{
    data.device,
    data.render->getCameraDataBuffer(),
    data.render->getMatrixesBuffer(),
    data.container->tasks1(), //reinterpret_cast<yavf::ComputeTask**>(data.task), //&computeTasks,
    
    data.lights,
  
    data.window->size().extent.width,
    data.window->size().extent.height,
  
    gBuffer->getDeferredRenderTargetDescriptor(),
    gBuffer->getDeferredRenderTargetLayoutDescriptor()
  };
  DefferedLightStage* lightStage = data.render->addStage<DefferedLightStage>(dInfo);
  
  const ToneMappingStage::CreateInfo tInfo{
    data.device,
    data.container->tasks1(), //reinterpret_cast<yavf::ComputeTask**>(data.task), //&computeTasks,
    
    data.window->size().extent.width,
    data.window->size().extent.height,
    
    lightStage->getOutputDescriptor()
  };
  ToneMappingStage* tone = data.render->addStage<ToneMappingStage>(tInfo);
  
  const CopyStage::CreateInfo cInfo{
    data.device,
    data.container->tasks2(), //reinterpret_cast<yavf::GraphicTask**>(data.task), //&graphicsTasks,
    
    tone->getOutputImage(),
    gBuffer->getDepthBuffer(),
    
    data.window->getFamily(),
    data.window
  };
  CopyStage* copy = data.render->addStage<CopyStage>(cInfo);
  
  // и отрисовка гуи
  // до гуи у нас еще должна быть закраска какой-нибудь текстурой если у нас ничего не отрисовалось
  // может быть это скайбокс? вполне возможно
  
  const size_t postRenderStageContainerSize = sizeof(GuiStage) + sizeof(MonsterDebugStage) + sizeof(GeometryDebugStage);
  const PostRenderStage::CreateInfo pInfo{
    data.device,
    data.container->tasks2(), //reinterpret_cast<yavf::GraphicTask**>(data.task), //&graphicsTasks,
    data.window
  };
  PostRenderStage* postRender = data.render->addStage<PostRenderStage>(postRenderStageContainerSize, pInfo);
  dynPipe.push_back(postRender);
  
//     const GuiStage::CreateInfo gInfo{
//       device,
//       task,
//       window
//     };
  //GuiStage* gui = render->addStage<GuiStage>(gInfo);
  GuiStage* gui = postRender->addPart<GuiStage>(data.data);
  
  const MonsterDebugStage::CreateInfo mdInfo{
    data.monDebugOpt,
    data.mon,
    data.render->getCameraDataBuffer(),
    monGbuffer->getInstanceData()
  };
  MonsterDebugStage* monDebug = postRender->addPart<MonsterDebugStage>(mdInfo);
  
  const GeometryDebugStage::CreateInfo gdInfo{
    data.geoDebugOpt,
    data.geo,
    
    data.render->getCameraDataBuffer(),
    
    geoGbuffer->getIndicesArray(),
    data.mapLoader->mapVertices()
  };
  GeometryDebugStage* geoDebug = postRender->addPart<GeometryDebugStage>(gdInfo);
  
  //data.render->addStage<EndTaskStage>(reinterpret_cast<yavf::TaskInterface**>(data.task));
  data.render->addStage<EndTaskStage>(data.container->tasks3());
}

void createPhysics(dt::thread_pool* threadPool, const DataArrays &arrays, PhysicsContainer &physicsContainer, PhysicsEngine** engine) {
// const GPUOctreeBroadphase::GPUOctreeBroadphaseCreateInfo octreeInfo{
  //   {glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(100.0f, 100.0f, 100.0f, 0.0f), 5},
  //   device,
  //   task, // таск
  //   nullptr
  // };
  // GPUOctreeBroadphase broad(octreeInfo);

//   const CPUOctreeBroadphase::OctreeCreateInfo octree{
//     glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 
//     glm::vec4(100.0f, 100.0f, 100.0f, 0.0f),
//     4
//   };
//   CPUOctreeBroadphase broad(octree);

  const CPUOctreeBroadphaseParallel::OctreeCreateInfo octree{
    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 
    glm::vec4(100.0f, 100.0f, 100.0f, 0.0f),
    5
  };
  //CPUOctreeBroadphaseParallel broad(threadPool, octree);
  CPUOctreeBroadphaseParallel* broad = physicsContainer.createBroadphase<CPUOctreeBroadphaseParallel>(threadPool, octree);

  //GPUNarrowphase narrow(device, task);
  //CPUNarrowphase narrow(octree.depth);
  //CPUNarrowphaseParallel narrow(&threadPool, octree.depth);
  CPUNarrowphaseParallel* narrow = physicsContainer.createNarrowphase<CPUNarrowphaseParallel>(threadPool, octree.depth);

  //GPUSolver solver(device, task);
  //CPUSolver solver;
  //CPUSolverParallel solver(&threadPool);
  CPUSolverParallel* solver = physicsContainer.createSolver<CPUSolverParallel>(threadPool);

  // const GPUPhysicsSorterCreateInfo sorterInfo{
  //   {
  //     "shaders/sorting.spv"
  //   },
  //   {
  //     "shaders/sortingOverlapping1.spv", "shaders/sortingOverlapping2.spv"
  //   }
  // };
  // GPUPhysicsSorter sorter(device, task, sorterInfo);

  //CPUPhysicsSorter sorter;
  CPUPhysicsSorter* sorter = physicsContainer.createPhysicsSorter<CPUPhysicsSorter>();
  
  const uint32_t staticMatrix = arrays.matrices->insert(glm::mat4(1.0f));
  const uint32_t dynamicMatrix = arrays.matrices->insert(glm::mat4(1.0f));
  const PhysicsExternalBuffers bufferInfo{
    staticMatrix, 
    dynamicMatrix, 
    arrays.inputs, 
    arrays.transforms, 
    arrays.matrices, 
    arrays.rotationCountBuffer,
    arrays.rotations, 
    arrays.externals
  };

//   const GPUPhysicsCreateInfo physInfo {
//     &broad, 
//     &narrow, 
//     &solver, 
//     &sorter,
//     &bufferInfo
//   };
//   GPUPhysics phys(device, task, physInfo);
  
  const CPUPhysicsParallel::CreateInfo physInfo{
    broad,
    narrow, 
    solver, 
    sorter,
    
    threadPool,
    &bufferInfo
  };
  //CPUPhysicsParallel phys(physInfo);
  *engine = physicsContainer.createPhysicsEngine<CPUPhysicsParallel>(physInfo);
}

void nextGuiFrame() {
  static double timef = 0.0;
  ImGuiIO &io = ImGui::GetIO();

  // Setup display size (every frame to accommodate for window resizing)
  int w, h;
  int display_w, display_h;
  glfwGetWindowSize(Global::window()->handle(), &w, &h);
  glfwGetFramebufferSize(Global::window()->handle(), &display_w, &display_h);
  io.DisplaySize = ImVec2(float(w), float(h));
  io.DisplayFramebufferScale = ImVec2(w > 0 ? (float(display_w) / w) : 0, h > 0 ? (float(display_h) / h) : 0);

  // Setup time step
  double current_time = glfwGetTime();
  io.DeltaTime = timef > 0.0 ? float(current_time - timef) : float(1.0f/60.0f);
  timef = current_time;

  if (Global::data()->focusOnInterface) {
    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    if (Global::window()->isFocused()) {
      double mouse_x, mouse_y;
      glfwGetCursorPos(Global::window()->handle(), &mouse_x, &mouse_y);
      // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
      io.MousePos = ImVec2(float(mouse_x), float(mouse_y));
    } else {
      io.MousePos = ImVec2(-1,-1);
    }

//     for (int i = 0; i < 3; i++) {
//       // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
//       io.MouseDown[i] = mousePressed[i] || glfwGetMouseButton(window->handle(), i) != 0;
//       mousePressed[i] = false;
//     }

    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    // клик меньше чем за один фрейм? может быть когда фпс мелкий? может пригодится
    for (uint32_t i = 0; i < 5; ++i) {
      io.MouseDown[i] = Global::data()->keys[i];
    }

    io.MouseWheel = Global::data()->mouseWheel;
    Global::data()->mouseWheel = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    glfwSetInputMode(Global::window()->handle(), GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
  } else {
    glfwSetInputMode(Global::window()->handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  // Start the frame
  ImGui::NewFrame();
}


void sync(TimeMeter &tm, const size_t &syncTime) {
  // отмечаем время "сна"
  // нам здесь все же будет полезен TimeMeter
  tm.stop();
  
  {
//     RegionLog rl("Global::render()->wait()"); 
    Global::render()->wait();
  }
  
  size_t accumulatedTime = 0;
  while (accumulatedTime < syncTime) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    const auto point = std::chrono::steady_clock::now() - tm.getStart();
    accumulatedTime += std::chrono::duration_cast<std::chrono::microseconds>(point).count();
  }
}

void guiShutdown(yavf::Device* device) {
  ImGuiIO& io = ImGui::GetIO();

  yavf::Image* image = (yavf::Image*)io.Fonts->TexID;
  device->destroy(image);

  ImGui::Shutdown();
}

void createReactions(const ReactionsCreateInfo &info) {
  auto input = info.input;
  info.container->create("Step forward", [input] () {
    input->forward();
  });
  
  info.container->create("Step backward", [input] () {
    input->backward();
  });
  
  info.container->create("Step left", [input] () {
    input->left();
  });
  
  info.container->create("Step right", [input] () {
    input->right();
  });
  
  info.container->create("Jump", [input] () {
    input->jump();
  });
  
  auto window = info.window;
  info.container->create("Interface focus", [window] () {
    static bool lastFocus = false;
    Global::data()->focusOnInterface = !Global::data()->focusOnInterface;
    
    if (lastFocus && Global::data()->focusOnInterface != lastFocus) {
      int width, height;
      glfwGetWindowSize(window->handle(), &width, &height);
      double centerX = double(width) / 2.0, centerY = double(height) / 2.0;
      glfwSetCursorPos(window->handle(), centerX, centerY);
    }
    
    lastFocus = Global::data()->focusOnInterface;
  });
  
  // а также use, attack, spells (1-9?), item use, hide weapon
  // и прочее
  // где располагать доступ ко всему этому?
  // может ли пользователь добавить свои функции к вызову?
}

void setUpKeys(KeyContainer* container) {
  {
    ActionKey* key = container->create(KeyConfiguration(GLFW_KEY_W), {});
    key->setReaction(PRESS,        container->config->reactions["Step forward"]);
    key->setReaction(DOUBLE_PRESS, container->config->reactions["Step forward"]);
    key->setReaction(LONG_PRESS,   container->config->reactions["Step forward"]);
  }
  
  {
    ActionKey* key = container->create(KeyConfiguration(GLFW_KEY_S), {});
    key->setReaction(PRESS,        container->config->reactions["Step backward"]);
    key->setReaction(DOUBLE_PRESS, container->config->reactions["Step backward"]);
    key->setReaction(LONG_PRESS,   container->config->reactions["Step backward"]);
  }
  
  {
    ActionKey* key = container->create(KeyConfiguration(GLFW_KEY_A), {});
    key->setReaction(PRESS,        container->config->reactions["Step left"]);
    key->setReaction(DOUBLE_PRESS, container->config->reactions["Step left"]);
    key->setReaction(LONG_PRESS,   container->config->reactions["Step left"]);
  }
  
  {
    ActionKey* key = container->create(KeyConfiguration(GLFW_KEY_D), {});
    key->setReaction(PRESS,        container->config->reactions["Step right"]);
    key->setReaction(DOUBLE_PRESS, container->config->reactions["Step right"]);
    key->setReaction(LONG_PRESS,   container->config->reactions["Step right"]);
  }
  
  {
    ActionKey* key = container->create(KeyConfiguration(GLFW_KEY_SPACE), {});
    key->setReaction(PRESS,        container->config->reactions["Jump"]);
  }
  
  {
    ActionKey* key = container->create(KeyConfiguration(GLFW_KEY_LEFT_ALT), {});
    key->setReaction(CLICK,        container->config->reactions["Interface focus"]);
  }
}

void mouseInput(UserInputComponent* input, const uint64_t &time) {
  if (Global::data()->focusOnInterface) return;
  
  double xpos, ypos;
  int32_t width, height;
  glfwGetCursorPos(Global::window()->handle(), &xpos, &ypos);
  glfwGetWindowSize(Global::window()->handle(), &width, &height);
  double centerX = double(width) / 2.0, centerY = double(height) / 2.0;
  glfwSetCursorPos(Global::window()->handle(), centerX, centerY);
  
  // играя с +, - можно делать такие штуки как инверися по осям
  // чувствительность мыши это mouseSpeed
  
  static float horisontalAngle = 0.0f, verticalAngle = 0.0f;
  
  static const float mouseSpeed = 5.0f;
  
  horisontalAngle = mouseSpeed * MCS_TO_SEC(time) * (float(width)  / 2.0f - float(xpos));
  verticalAngle   = mouseSpeed * MCS_TO_SEC(time) * (float(height) / 2.0f - float(ypos));
  
//   std::cout << "width: " << width << "\n";
//   std::cout << "height: " << height << "\n";
//   std::cout << "xpos: " << xpos << "\n";
//   std::cout << "ypos: " << ypos << "\n";

  input->mouseMove(horisontalAngle, verticalAngle);
}

void keysCallbacks(KeyConfig* config, const uint64_t &time) {
  for (size_t i = 0; i < config->keys.size(); ++i) {
    const uint32_t keyCount = config->keys[i]->getKeysCount();
    
//     std::cout << "keyCount " << keyCount << "\n";
    
    bool state = true;
    for (uint32_t j = 0; j < keyCount; ++j) {
      const uint32_t key = config->keys[i]->getKey(j);
      
//       std::cout << "key " << key << "\n";
      
      state = state && Global::data()->keys[key];
    }
    
    config->keys[i]->execute(state ? GLFW_PRESS : GLFW_RELEASE, time);
  }
}

void callback(int error, const char* description) {
  std::cout << "Error code: " << error << std::endl;
  std::cout << "Error: " << description << std::endl;
}

void scrollCallback(GLFWwindow*, double xoffset, double yoffset) {
  if (!Global::window()->isFocused()) return;
  if (!Global::data()->focusOnInterface) return;
  
  (void)xoffset;
  Global::data()->mouseWheel += float(yoffset);
}

void charCallback(GLFWwindow*, unsigned int c) {
  if (!Global::window()->isFocused()) return;
  if (!Global::data()->focusOnInterface) return;

  ImGuiIO& io = ImGui::GetIO();
  if (c > 0 && c < 0x10000) io.AddInputCharacter((unsigned short)c);
}

void mouseButtonCallback(GLFWwindow*, int button, int action, int mods) {
  if (!Global::window()->isFocused()) return;
  
  (void)mods;
//   if (action == GLFW_PRESS) mousePressed[button] = true;
//   if (action == GLFW_RELEASE) mousePressed[button] = false;
  
  Global::data()->keys[button] = !(action == GLFW_RELEASE);
}

void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods) {
  if (!Global::window()->isFocused()) return;
  (void)scancode;
  
  ImGuiIO& io = ImGui::GetIO();
  
  Global::data()->keys[key] = !(action == GLFW_RELEASE);
  
  if (!Global::data()->focusOnInterface) return;
  
  io.KeysDown[key] = !(action == GLFW_RELEASE);
  
  (void)mods; // Modifiers are not reliable across systems
  io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
  io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
  io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
  io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void iconifyCallback(GLFWwindow*, int iconified) {
  Global::window()->setIconify(iconified);
}

void focusCallback(GLFWwindow*, int focused) {
  Global::window()->setFocus(focused);
}

const char* getClipboard(void* user_data) {
  return glfwGetClipboardString(((Window*)user_data)->handle());
}

void setClipboard(void* user_data, const char* text) {
  glfwSetClipboardString(((Window*)user_data)->handle(), text);
}
