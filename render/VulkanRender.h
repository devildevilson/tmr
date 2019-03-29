#ifndef VULKAN_RENDER_H
#define VULKAN_RENDER_H

#include "Render.h"
#include "yavf.h"

#include "ImageResourceContainer.h"

// #include <imgui.h>
#include "nuklear_header.h"

#define CHECK_ERROR(res, str) if (res) {                         \
                                Global::console()->printE(str);  \
                                throw std::runtime_error(str);   \
                              }

// #define DEFAULT_DESCRIPTOR_POOL_NAME "default_descriptor_pool"
// #define UNIFORM_BUFFER_LAYOUT_NAME "uniform_layout"
                              
// namespace std {
//   template<>
//   struct hash<std::pair<uint32_t, uint32_t>> {
//     size_t operator() (const std::pair<uint32_t, uint32_t> &pair) const {
//       return (pair.first + pair.second) * (pair.first + pair.second + 1) / 2 + pair.second;
//     }
//   };
//   
//   template<>
//   struct hash<VkExtent2D> {
//     size_t operator() (const VkExtent2D &extent) const {
//       return (extent.width + extent.height) * (extent.width + extent.height + 1) / 2 + extent.height;
//     }
//   };
//   
//   template<>
//   struct equal_to<VkExtent2D> {
//     bool operator() (const VkExtent2D &right, const VkExtent2D &left) const {
//       return right.width == left.width && right.height == left.height;
//     }
//   };
// }

// struct ImageCreateInfo {
//   std::string path;
//   std::string name;
//   std::string sampler;
//   uint32_t rows;
//   uint32_t columns;
//   uint32_t count;
//   VkExtent3D size;
// };

class VulkanRender : public Render/*, public ImageResourceContainer*/ {
public:
  struct CreateInfo {
    yavf::Instance* instance;
    yavf::Device* device;
    yavf::CombinedTask** task;
//     yavf::TaskInterface* task;
    
    size_t stageContainerSize;
  };
  
  //VulkanRender(const size_t &stageContainerSize);
  VulkanRender(const CreateInfo &info);
  ~VulkanRender();
  
  void setContextIndex(const uint32_t &index);
  
  void updateCamera() override;
  
  void update(const uint64_t &time) override;
  void start() override;
  void wait() override;
  
  // че с созданием инстансов и устройств? мне на каждый рендер нужно по крайней мере одно устройство
  // а на все рендеры мне в принципе нужен только один инстанс
//   void createInstance(const std::vector<const char*> &extensions);
//   void createDevice();
  
  yavf::Instance* getInstance();
  
//   void setData(const Data &data);
  
  yavf::Buffer* getCameraDataBuffer() const;
  yavf::Buffer* getMatrixesBuffer() const;
  
  // с текстурами обстоят дела так:
  // должен быть менеджер ресурсов (я его пытался сделать, но в итоге получилось не совсем то)
  // должен быть загрузчик ресурсов (общается с менеджером, нужен только в момент загрузки)
  // должен быть ресурс (набор данных определяющий на самом деле что угодно, но я соотвественно должен понимать что где)
  
  // в МЕНЕДЖЕРЕ ресурсы хранятся и он ими УПРАВЛЯЕТ (менеджер внезапно)
  // то есть например я должен сообщать менеджеру когда какой ресурс мне больше не нужен
  // менеджер после этого его удаляет (или просто освобождает память)
  // также было бы неплохой идеей как то внутри перераспределять память и освобождать ненужную 
  // (например выгрузить старые текстуры, сортануть оставшиеся, в свободое место загрузить новые, и если не хватает места пересоздать)
  // с пересозданием конечно могут возникнуть проблемы (у меня сейчас текстурки хранятся одним огромным массивом)
  
  // загрузчик работает только в тот момент когда мне нужно че нить загрузить дальше он удаляется
  
  // что из себя должен представлять ресурс? в общем то скорее всего только штуку которую можно найти по имени и удалить
  // индекс массива и индекс в массиве по идее представляют собой ресурс
  // нет, наверное ресурс представляет собой отдельная картинка, которая при загрузке получает кучу индексов
  // и когда мы удаляем этот ресурс, мы удалем все эти индексы
  // как мы должны получать тогда эти индексы? (сейчас у меня текстуры хранятся вполне свободно, и если я вдруг удалю ресурс никто не узнает об изменении)
  // то есть должен быть какой то указатель? то есть текстурка у меня превратится в 2 указателя (ресурс текстурки и ресурс сэмплера)?
  // звучит как буллщит, так как мне требуется уже сейчас получать индексы текстурки (а в будущем планирую перевести вычисления на гпу)
  // это означает: либо мне с текстурами нужно будет рабираться только в цпу
  // либо? хранить ВСЕ текстурные индексы + индексы сэмплеров отдельно?
  // а доступ получать через индекс (ну то есть у нас есть огромный массив с 3*uint данными)
  // вообще возможно это жизнеспособная идея
  
  // но что тогда такое анимации? набор фреймов, а фрейм это набор текстур с разных сторон
  // то есть у меня будет индекс анимации, в котором индекс фрейма, в котором индекс текстуры (это капец)
  // и к этому всему добавляется (умножается) несколько состояний объекта
  // по другому на самом деле вряд ли получится (зато появится возможность спихнть это дело на гпу)
  
  // пока что в роли менеджера будет выступать рендер
//   uint32_t imageCount() const override;
//   yavf::Descriptor imageDescriptor() const override;
//   yavf::DescriptorSetLayout imageSetLayout() const override;
//   
//   uint32_t samplerCount() const override;
//   yavf::Descriptor samplerDescriptor() const override;
//   yavf::DescriptorSetLayout samplerSetLayout() const override;
//   
//   // это тоже в принципе уйдет в виртуальные функции
//   void precacheLayers(const VkExtent2D &size, const uint32_t &count);
//   void loadTexture(const std::string &prefix, const ImageCreateInfo &info);
//   void createMipmaps();
//   void updateImageDescriptors();
//   void clearImages();
//   std::vector<Texture> getTextures(const std::string &name) const;
//   Texture getTexture(const std::string &name, const uint32_t &index) const;
//   uint32_t getSampler(const std::string &name) const;
  
  // тут что должно быть?
  void printStats() override;
private:
//   struct TextureArrayData {
//     yavf::Image* texture;
//     uint32_t currentLayer;
//     uint32_t descriptorIndex;
//   };
  
  // где создавать инстанс? по идее тут он не нужен, для него (ну и девайсов) тоже можно сделать какой-нибудь контейнер
  yavf::Instance* instance;
  
  // нам в последствии нужно будет задизайнить это дело
  // на несколько устройств (во первых параллелить, во вторых вывод на разные мониторы)
  yavf::Device* device = nullptr;
  
  // будет несколько скорее всего
  //yavf::TaskInterface* task = nullptr;
  uint32_t currentIndex;
  //yavf::TaskInterface** task;
  yavf::CombinedTask** task;
  
  yavf::Buffer* uniformCameraData;
  yavf::Buffer* uniformMatrixes;
  
  yavf::Internal::Queue waitFence;
  
  uint32_t currentArrayElement = 0;
  
//   uint32_t imagesCount;
//   uint32_t samplersCount;
//   yavf::Descriptor images;
//   yavf::Descriptor samplers;
//   yavf::DescriptorSetLayout imageLayout;
//   yavf::DescriptorSetLayout samplerLayout;
//   
//   yavf::Image* imagesBuffer = nullptr;
//   yavf::Sampler sampler; // пока 1
//   
//   std::vector<std::vector<TextureArrayData>> arrays;
//   std::unordered_map<std::string, std::vector<Texture>> textureNames;
//   std::unordered_map<std::string, uint32_t> samplerNames;
//   std::unordered_map<VkExtent2D, uint32_t> arrayIndices;
};

#endif
