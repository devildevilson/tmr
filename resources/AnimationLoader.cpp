#include "AnimationLoader.h"

#include <ctype.h>
#include <iostream>
#include "ImageLoader.h"
#include "Globals.h"
#include "AnimationSystem.h"
#include "SoundLoader.h"

AnimationLoader::LoadData::LoadData(const CreateInfo &info) : Resource(info.resInfo), m_frames(info.frames) {}
const std::vector<AnimationLoader::LoadData::Frame> & AnimationLoader::LoadData::frames() const { return m_frames; }

// bool VHpartValid(const size_t &indexFrom, const size_t &lastIndex, const std::string &str) {
//   const size_t count = std::min(str.length(), lastIndex) - indexFrom;
//   if (count > 2) return false;
//   if (count == 2) return (str[indexFrom] == 'h' && str[indexFrom+1] == 'v') || (str[indexFrom] == 'v' && str[indexFrom+1] == 'h');
//   return str[indexFrom] == 'h' || str[indexFrom] == 'v';
// }
// 
// bool indexPartValid(const size_t &indexFrom, const size_t &indexLast, const std::string &str) {
//   bool digitOk = true;
//   for (size_t i = indexFrom; i < std::min(indexLast, str.size()); ++i) {
//     digitOk = digitOk && isdigit(str[i]);
//   }
// 
//   return digitOk;
// }
// 
// AnimationLoader::LoadData::Side parseSideString(const std::string &str) {
//   size_t imageIndex = 0;
// 
//   size_t lastIndex = str.size();
//   size_t nextPart = str.find_last_of('.');
//   if (nextPart == std::string::npos) return {
//     ResourceID::get(str),
//     0,
//     false,
//     false
//   };
// 
//   const size_t dotCount = std::count(str.begin(), str.end(), '.');
// 
//   bool flipU = false;
//   bool flipV = false;
//   bool vhPart = false;
//   bool indexPart = false;
// 
//   for (uint32_t i = 0; i < std::min(size_t(2), dotCount); ++i) {
//     {
//       const bool tmp = VHpartValid(nextPart+1, lastIndex, str);
//       if (!vhPart && tmp) {
//         vhPart = tmp;
// 
//         flipU = str[nextPart+1] == 'h' || str[nextPart+2] == 'h';
//         flipV = str[nextPart+1] == 'v' || str[nextPart+2] == 'v';
// 
//         lastIndex = nextPart;
//         for (nextPart = lastIndex-1; nextPart != SIZE_MAX; --nextPart) {
//           if (str[nextPart] == '.') break;
// 
//           //std::cout << "char: " << str[nextPart] << '\n';
//         }
// 
//         continue;
//       }
//     }
// 
//     {
//       const bool tmp = indexPartValid(nextPart+1, lastIndex, str);
//       if (tmp) {
//         indexPart = tmp;
//         const std::string str122 = str.substr(nextPart+1, lastIndex);
//         // std::cout << str122 << '\n';
//         imageIndex = stoi(str122);
// 
//         lastIndex = nextPart;
//         for (nextPart = lastIndex-1; nextPart != SIZE_MAX; --nextPart) {
//           if (str[nextPart] == '.') break;
//         }
// 
//         continue;
//       }
//     }
// 
//     if (i == 0 && !(vhPart || indexPart)) return {
//       ResourceID::get(str),
//       0,
//       false,
//       false
//     };
//   }
// 
//   return {
//     ResourceID::get(str.substr(0, lastIndex)),
//     imageIndex,
//     flipU,
//     flipV
//   };
// }

bool checkAnimationJsonValidity(const std::string &path, const nlohmann::json &data, const size_t &mark, AnimationLoader::LoadData::CreateInfo &info, std::vector<ErrorDesc> &errors, std::vector<WarningDesc> &warnings) {
  bool hasId = false, hasTime = false;
  
  const size_t errorsCount = errors.size();
  
  info.m_time = 0;
  info.m_sound.sound = ResourceID();
  info.m_sound.delay = 0;
  info.m_sound.static_sound = true;
  info.m_sound.relative = false;
  info.m_sound.scalar = 0.0f;
  
  for (auto itr = data.begin(); itr != data.end(); ++itr) {
    if (itr.value().is_string() && itr.key() == "id") {
      hasId = true;
      info.resInfo.resId = ResourceID::get(itr.value().get<std::string>());
      continue;
    }
    
    if (itr.value().is_number_unsigned() && itr.key() == "time") {
      info.m_time = itr.value().get<size_t>();
      hasTime = true;
      continue;
    }
    
    if (itr.value().is_object() && itr.key() == "sound") {
      bool hasId = false;
      for (auto soundItr = itr.value().begin(); soundItr != itr.value().end(); ++soundItr) {
        if (soundItr.value().is_string() && soundItr.key() == "id") {
          info.m_sound.sound = ResourceID::get(soundItr.value().get<std::string>());
          hasId = true;
          continue;
        }
        
        if (soundItr.value().is_number_unsigned() && soundItr.key() == "delay") {
          info.m_sound.delay = soundItr.value().get<size_t>();
          continue;
        }
        
        if (soundItr.value().is_boolean() && soundItr.key() == "static") {
          info.m_sound.static_sound = soundItr.value().get<bool>();
          continue;
        }
        
        if (soundItr.value().is_boolean() && soundItr.key() == "relative") {
          info.m_sound.relative = soundItr.value().get<bool>();
          continue;
        }
        
        if (soundItr.value().is_number() && soundItr.key() == "scalar") {
          info.m_sound.scalar = soundItr.value().get<float>();
          continue;
        }
      }
      
      if (!hasId) {
        ErrorDesc desc(mark, AnimationLoader::ERROR_SOUND_RESOURCE_ID_MUST_BE_SPECIFIED, "Could not find sound resource id");
        std::cout << "Error: " << desc.description << "\n";
        errors.push_back(desc);
      }
    }
    
    if (itr.value().is_array() && itr.key() == "frames") {
      for (size_t i = 0; i < itr.value().size(); ++i) {
        if (itr.value()[i].is_string()) {
          std::vector<AnimationLoader::LoadData::Side> sides(1, AnimationLoader::LoadData::Side{ResourceID(), 0, false, false});
          parseTextureDataString(itr.value()[i].get<std::string>(), sides[0].image, sides[0].index, sides[0].flipU, sides[0].flipV);
          info.frames.push_back(AnimationLoader::LoadData::Frame{sides});
          continue;
        }
        
        if (itr.value()[i].is_object()) {
          bool hasImageId = false;
          std::vector<AnimationLoader::LoadData::Side> sides(1, AnimationLoader::LoadData::Side{ResourceID(), 0, false, false});
          for (auto side = itr.value()[i].begin(); side != itr.value()[i].end(); ++side) {
            if (side.value().is_string() && side.key() == "image") {
              hasImageId = true;
              sides[i].image = ResourceID::get(side.value().get<std::string>());
              continue;
            }
            
            if (side.value().is_number_unsigned() && side.key() == "index") {
              sides[i].index = side.value().get<size_t>();
              continue;
            }
            
            if (side.value().is_string() && side.key() == "flip") {
              const std::string &str = side.value().get<std::string>();
              
              if (str.length() > 2) {
                ErrorDesc desc(mark, AnimationLoader::ERROR_BAD_FLIP_DATA, "Bad side image flip data");
                std::cout << "Error: " << desc.description << "\n";
                errors.push_back(desc);
                continue;
              }
              
              sides[i].flipU = str[0] == 'h' || str[1] == 'h';
              sides[i].flipV = str[0] == 'v' || str[1] == 'v';
              
              continue;
            }
          }
          
          if (!hasImageId) {
            ErrorDesc desc(mark, AnimationLoader::ERROR_SIDE_MUST_HAVE_AN_IMAGE_ID, "Could not find side image id");
            std::cout << "Error: " << desc.description << "\n";
            errors.push_back(desc);
            continue;
          }
          
          info.frames.push_back(AnimationLoader::LoadData::Frame{sides});
          continue;
        }
        
        if (itr.value()[i].is_array()) {
          std::vector<AnimationLoader::LoadData::Side> sides(itr.value()[i].size(), AnimationLoader::LoadData::Side{ResourceID(), 0, false, false});
          for (size_t j = 0; j < itr.value()[i].size(); ++j) {
            if (itr.value()[i][j].is_string()) {
              parseTextureDataString(itr.value()[i][j].get<std::string>(), sides[j].image, sides[j].index, sides[j].flipU, sides[j].flipV);
              continue;
            }
            
            if (itr.value()[i][j].is_object()) {
              bool hasImageId = false;
              for (auto side = itr.value()[i][j].begin(); side != itr.value()[i][j].end(); ++side) {
                if (side.value().is_string() && side.key() == "image") {
                  hasImageId = true;
                  sides[j].image = ResourceID::get(side.value().get<std::string>());
                  continue;
                }
                
                if (side.value().is_number_unsigned() && side.key() == "index") {
                  sides[j].index = side.value().get<size_t>();
                  continue;
                }
                
                if (side.value().is_string() && side.key() == "flip") {
                  const std::string &str = side.value().get<std::string>();
                  
                  if (str.length() > 2) {
                    ErrorDesc desc(mark, AnimationLoader::ERROR_BAD_FLIP_DATA, "Bad side image flip data");
                    std::cout << "Error: " << desc.description << "\n";
                    errors.push_back(desc);
                    continue;
                  }
                  
                  sides[j].flipU = str[0] == 'h' || str[1] == 'h';
                  sides[j].flipV = str[0] == 'v' || str[1] == 'v';
                  
                  continue;
                }
              }
              
              if (!hasImageId) {
                ErrorDesc desc(mark, AnimationLoader::ERROR_SIDE_MUST_HAVE_AN_IMAGE_ID, "Could not find side image id");
                std::cout << "Error: " << desc.description << "\n";
                errors.push_back(desc);
              }
              
              continue;
            }
          }
          
          info.frames.push_back(AnimationLoader::LoadData::Frame{sides});
          
          continue;
        }
      }
      
      continue;
    }
  }
  
  if (!hasId) {
    ErrorDesc desc(mark, AnimationLoader::ERROR_ANIMATION_MUST_HAVE_AN_ID, "Animation must have an id");
    std::cout << "Error: " << desc.description << "\n";
    errors.push_back(desc);
  }
  
  if (!hasTime) {
    ErrorDesc desc(mark, AnimationLoader::ERROR_COULD_NOT_FIND_ANIMATION_TIME, "Animation time must be specified");
    std::cout << "Error: " << desc.description << "\n";
    errors.push_back(desc);
  }
  
  const size_t sideCount = info.frames[0].sides.size();
  for (size_t i = 1; i < info.frames.size(); ++i) {
    if (sideCount != info.frames[i].sides.size()) {
      ErrorDesc desc(mark, AnimationLoader::ERROR_EVERY_ANIMATION_FRAME_MUST_HAVE_EQUAL_NUMBER_OF_SIDES, "Every animation frame must have equal number of sides");
      std::cout << "Error: " << desc.description << "\n";
      errors.push_back(desc);
    }
  }
  
  return errorsCount == errors.size();
}

AnimationLoader::AnimationLoader(const CreateInfo &info) : imageLoader(info.imageLoader), soundLoader(info.soundLoader) {}
AnimationLoader::~AnimationLoader() {
  clear();
}

bool AnimationLoader::canParse(const std::string &key) const {
  return key == "animations";
}

bool AnimationLoader::parse(const Modification* mod,
                            const std::string &pathPrefix,
                            const nlohmann::json &data,
                            std::vector<Resource*> &resource,
                            std::vector<ErrorDesc> &errors,
                            std::vector<WarningDesc> &warnings) {
  if (tempData == nullptr) tempData = new LoadingTemporaryData<LoadData, 30>;
  
  if (data.is_string()) {
    const std::string &path = data.get<std::string>();
    std::ifstream file(pathPrefix + path);
    if (!file) {
      ErrorDesc desc(4123, ERROR_COULD_NOT_LOAD_FILE, "Could not load file "+pathPrefix+path);
      std::cout << "Error: " << desc.description << "\n";
      errors.push_back(desc);
      return false;
    }
    
    nlohmann::json json;
    file >> json;
    return parse(mod, pathPrefix, json, resource, errors, warnings);
  } else if (data.is_array()) {
    bool ret = true;
    for (size_t i = 0; i < data.size(); ++i) {
      ret = ret && parse(mod, pathPrefix, data[i], resource, errors, warnings);
    }
    return ret;
  } else if (data.is_object()) {
    LoadData::CreateInfo info = {};
    info.resInfo.mod = mod;
    info.resInfo.parsedBy = this;
    info.resInfo.resGPUSize = 0;
    info.resInfo.pathStr = "";
    //info.resInfo.resSize = sizeof(Animation);
    
    const bool ret = checkAnimationJsonValidity(pathPrefix, data, 12512, info, errors, warnings);
    if (!ret) return false;
    
    info.resInfo.resSize = info.frames.size() * info.frames[0].sides.size() * sizeof(Animation::Image) + sizeof(Animation);
    
    std::cout << "Parsed " << info.resInfo.resId.name() << " res size " << info.resInfo.resSize << "\n";
    
    auto ptr = tempData->container.create(info.resInfo.resId, info);
    resource.push_back(ptr);
    return true;
  }
  
  return false;
}
                            
bool AnimationLoader::forget(const ResourceID &name) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
//   const size_t index = findTempData(name);
//   if (index == SIZE_MAX) return false;
  
//   tempData->dataPool.deleteElement(tempData->dataPtr[index]);
//   std::swap(tempData->dataPtr[index], tempData->dataPtr.back());
//   tempData->dataPtr.pop_back();
  
  return tempData->container.destroy(name);
}

Resource* AnimationLoader::getParsedResource(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
//   const size_t index = findTempData(id);
//   return index == SIZE_MAX ? nullptr : tempData->dataPtr[index];
  return tempData->container.get(id);
}

const Resource* AnimationLoader::getParsedResource(const ResourceID &id) const {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
//   const size_t index = findTempData(id);
//   return index == SIZE_MAX ? nullptr : tempData->dataPtr[index];
  return tempData->container.get(id);
}

bool AnimationLoader::load(const ModificationParser* modifications, const Resource* resource) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
//   for (auto data : tempData->dataToLoad) {
//     if (data->id() == resource->id()) return true;
//   }
  
  const uint32_t index = Global::animations()->getAnimationId(resource->id());
  if (index != UINT32_MAX) return true;
  
  auto res = tempData->container.get(resource->id());
  if (res == nullptr) return false;
  
//   tempData->dataToLoad.push_back(res);
  // нужно загрузить анимацию здесь 
  // вот мы и подошли к необходимости использовать imageLoader 
  // при том так чтобы я учитывал еще и конфликты
  // мне нужно предоставить доступ к конфликтам 
  // а пока можно загрузить и так
  
  if (res->frames().empty()) throw std::runtime_error("Empty animation");
  
  const size_t frameSize = res->frames()[0].sides.size();
  if (frameSize > 256) throw std::runtime_error("Frame size > 256 is not allowed");
  if (frameSize == 0) throw std::runtime_error("Frames size == 0");
  
  for (uint32_t i = 1; i < res->frames().size(); ++i) {
    if (res->frames()[i].sides.size() != frameSize) throw std::runtime_error("Frame size must be the same across animation");
  }
  
  std::vector<std::vector<Animation::Image>> frames(res->frames().size(), std::vector<Animation::Image>(res->frames()[0].sides.size()));
  for (size_t i = 0; i < res->frames().size(); ++i) {
    for (size_t j = 0; j < res->frames()[i].sides.size(); ++j) {
      const bool loadRes = imageLoader->load(nullptr, imageLoader->getParsedResource(res->frames()[i].sides[j].image));
      if (!loadRes) throw std::runtime_error("Could not find parsed resource "+res->frames()[i].sides[j].image.name());
      
      const Image img = imageLoader->image(res->frames()[i].sides[j].image, res->frames()[i].sides[j].index);
      if (img.index == UINT32_MAX || img.layer == UINT32_MAX) throw std::runtime_error("Could not load "+res->frames()[i].sides[j].image.name()+" with index "+std::to_string(res->frames()[i].sides[j].index));
      
      frames[i][j] = {
        img,
        res->frames()[i].sides[j].flipU,
        res->frames()[i].sides[j].flipV
      };
    }
  }
  
  const SoundData* sound = nullptr;
  if (res->sound().sound.valid()) {
    const bool load = soundLoader->load(nullptr, soundLoader->getParsedResource(res->sound().sound));
    if (!load) throw std::runtime_error("Could not load sound "+res->sound().sound.name());
    
    sound = soundLoader->getSound(res->sound().sound);
    if (sound == nullptr) throw std::runtime_error("Could not find sound "+res->sound().sound.name());
  }
  
  uint8_t concreteFrameSize = static_cast<uint8_t>(frameSize);
  const Animation::ConstructorInfo cInfo{
    Type::get(res->id().name()),
    Global::get<AnimationSystem>()->addAnimationTextureData(frames),
    concreteFrameSize,
    0, res->frames().size(),
    res->time(),
    {
      sound,
      res->sound().delay,
      res->sound().static_sound,
      res->sound().relative,
      res->sound().scalar
    }
  };
  container.create(cInfo);
  
  //auto id = Global::animations()->createAnimation(res->id(), Animation::CreateInfo{frames});
  //PRINT("ANIMATION ID: "+std::to_string(id))  
  
  return true;
}

bool AnimationLoader::unload(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  // удалять анимации я вообще могу?
  // похоже что в текущем виде нет, что делать в этом случае?
  // я складываю имайджы неочень удачно для того чтобы удалять изображения
  
  std::cout << "Unload animations dont worked yet" << "\n";
  
//   for (size_t i = 0; i < tempData->dataToLoad.size(); ++i) {
//     if (tempData->dataToLoad[i]->id() == id) {
//       std::swap(tempData->dataToLoad[i], tempData->dataToLoad.back());
//       tempData->dataToLoad.pop_back();
//       return true;
//     }
//   }
  
  return false;
}

void AnimationLoader::end() {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
//   for (auto loadData : tempData->dataToLoad) {
//     std::vector<std::vector<Animation::Image>> frames(loadData->frames().size(), std::vector<Animation::Image>(loadData->frames()[0].sides.size()));
//     
//     for (size_t i = 0; i < loadData->frames().size(); ++i) {
//       for (size_t j = 0; j < loadData->frames()[i].sides.size(); ++j) {
//         frames[i][j].image = imageLoader->image(loadData->frames()[i].sides[j].image, loadData->frames()[i].sides[j].index);
//         if (frames[i][j].image.index == UINT32_MAX || frames[i][j].image.layer == UINT32_MAX) throw std::runtime_error("Could not find an image "+loadData->frames()[i].sides[j].image.name()+" with index "+std::to_string(loadData->frames()[i].sides[j].index));
//         frames[i][j].flipU = loadData->frames()[i].sides[j].flipU;
//         frames[i][j].flipV = loadData->frames()[i].sides[j].flipV;
//       }
//     }
//     
//     Global::animations()->createAnimation(loadData->id(), Animation::CreateInfo{frames});
//   }
}

void AnimationLoader::clear() {
  delete tempData;
  tempData = nullptr;
}

size_t AnimationLoader::overallState() const {
  
}

size_t AnimationLoader::loadingState() const {
  
}

std::string AnimationLoader::hint() const {
  
}

const Animation* AnimationLoader::getAnim(const Type &id) const {
  return container.get(id);
}

size_t AnimationLoader::findTempData(const ResourceID &id) const {
//   for (size_t i = 0; i < tempData->dataPtr.size(); ++i) {
//     if (tempData->dataPtr[i]->id() == id) return i;
//   }
//   
//   return SIZE_MAX;
}

// if (!itr.value()[i].is_object()) {
//           ErrorDesc desc(mark, AnimationLoader::ERROR_BAD_FRAME_DATA, "Bad animation frame data");
//           std::cout << "Error: " << desc.description << "\n";
//           errors.push_back(desc);
//           return false;
//         }
//         
//         bool hasSides = false;
//         for (auto frameData = itr.value()[i].begin(); frameData != itr.value()[i].end(); ++frameData) {
//           if (frameData.value().is_array() && frameData.key() == "sides") {
//             hasSides = true;
//             std::vector<AnimationLoader::LoadData::Side> sides(frameData.value().size(), AnimationLoader::LoadData::Side{ResourceID(), 0, false, false});
//             for (size_t j = 0; j < frameData.value().size(); ++j) {
//               if (frameData.value()[i].is_string()) {
//                 parseTextureDataString(frameData.value()[i].get<std::string>(), sides[i].image, sides[i].index, sides[i].flipU, sides[i].flipV);
//                 continue;
//               }
//               
//               if (frameData.value()[i].is_object()) {
//                 bool hasImageId = false;
//                 for (auto side = frameData.value()[i].begin(); side != frameData.value()[i].end(); ++side) {
//                   if (side.value().is_string() && side.key() == "image") {
//                     hasImageId = true;
//                     sides[i].image = ResourceID::get(side.value().get<std::string>());
//                     continue;
//                   }
//                   
//                   if (side.value().is_number_unsigned() && side.key() == "index") {
//                     sides[i].index = side.value().get<size_t>();
//                     continue;
//                   }
//                   
//                   if (side.value().is_string() && side.key() == "flip") {
//                     const std::string &str = side.value().get<std::string>();
//                     
//                     if (str.length() > 2) {
//                       ErrorDesc desc(mark, AnimationLoader::ERROR_BAD_FLIP_DATA, "Bad side image flip data");
//                       std::cout << "Error: " << desc.description << "\n";
//                       errors.push_back(desc);
//                       continue;
//                     }
//                     
//                     sides[i].flipU = str[0] == 'h' || str[1] == 'h';
//                     sides[i].flipV = str[0] == 'v' || str[1] == 'v';
//                     
//                     continue;
//                   }
//                 }
//                 
//                 if (!hasImageId) {
//                   ErrorDesc desc(mark, AnimationLoader::ERROR_SIDE_MUST_HAVE_AN_IMAGE_ID, "Could not find side image id");
//                   std::cout << "Error: " << desc.description << "\n";
//                   errors.push_back(desc);
//                 }
//                 
//                 continue;
//               }
//             }
//             
//             info.frames.push_back(AnimationLoader::LoadData::Frame{sides});
//             
//             continue;
//           }
//         }
//         
//         if (!hasSides) {
//           ErrorDesc desc(mark, AnimationLoader::ERROR_FRAME_MUST_HAVE_AT_LEAST_ONE_SIDE, "Frame must have at least one side");
//           std::cout << "Error: " << desc.description << "\n";
//           errors.push_back(desc);
//         }
