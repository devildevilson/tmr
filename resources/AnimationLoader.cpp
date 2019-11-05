#include "AnimationLoader.h"

#include <ctype.h>
#include <iostream>
#include "ImageLoader.h"
#include "Globals.h"
#include "AnimationSystem.h"

AnimationLoader::LoadData::LoadData(const CreateInfo &info) : Resource(info.resInfo), m_frames(info.frames) {}
const std::vector<AnimationLoader::LoadData::Frame> & AnimationLoader::LoadData::frames() const { return m_frames; }

bool VHpartValid(const size_t &indexFrom, const std::string &str) {
  const size_t count = str.length() - indexFrom;
  if (count > 2) return false;
  if (count == 2) return str[indexFrom] == 'h' || str[indexFrom+1] == 'h' || str[indexFrom] == 'v' || str[indexFrom+1] == 'v';
  return str[indexFrom] == 'h' || str[indexFrom] == 'v';
}

bool indexPartValid(const size_t &indexFrom, const size_t &indexLast, const std::string &str) {
  bool digitOk = true;
  for (size_t i = indexFrom; i < std::min(indexLast, str.size()); ++i) {
    digitOk = digitOk && isdigit(str[i]);
  }
  
  return digitOk;
}

static LoadData::Side parseSideString(const std::string &str) {
  size_t flipIndex = 0;
  size_t imageIndex = 0;
  
  bool hasVH = false;
  
  const size_t lastIndex = str.find_last_of('.');
  if (lastIndex == std::string::npos) return {
    ResourceID::get(str),
    0,
    false,
    false
  };
  
  bool flipU = false;
  bool flipV = false;
  
  size_t nextPart = lastIndex+1;
  const bool VHpart = VHpartValid(nextPart, str);
  if (VHpart) {
    flipU = str[nextPart] == 'h' || str[nextPart+1] == 'h';
    flipV = str[nextPart] == 'v' || str[nextPart+1] == 'v';
    
    for (nextPart = lastIndex-1; nextPart != SIZE_MAX; --nextPart) {
      if (str[nextPart] == '.') break;
      
      //digitOk = digitOk && isdigit(str[imageIndex]);
    }
    
    if (nextPart == SIZE_MAX) {
      return {
        ResourceID::get(str.substr(0, lastIndex)),
        0,
        flipU,
        flipV
      };
    }
    
    //nextPart += 1;
  }
  
  const bool indexPart = indexPartValid(nextPart+1, VHpart ? lastIndex : SIZE_MAX, str);
  if (indexPart) {
    imageIndex = stoi(str.substr(nextPart+1, VHpart ? lastIndex : str.size()));
  }
  
  return {
    ResourceID::get(str.substr(0, nextPart)),
    imageIndex,
    flipU,
    flipV
  };
}

bool checkAnimationJsonValidity(const std::string &path, const nlohmann::json &data, const size_t &mark, AnimationLoader::LoadData::CreateInfo &info, std::vector<ErrorDesc> &errors, std::vector<WarningDesc> &warnings) {
  bool hasId = false;
  
  const size_t errorsCount = errors.size();
  
  for (auto itr = data.begin(); itr != data.end(); ++itr) {
    if (itr.value().is_string() && itr.key() == "id") {
      hasId = true;
      info.resInfo.resId = ResourceID::get(itr.value().get<std::string>());
      continue;
    }
    
    if (itr.value().is_array() && itr.key() == "frames") {
      for (size_t i = 0; i < itr.value().size(); ++i) {
        if (!itr.value()[i].is_object()) {
          ErrorDesc desc(mark, AnimationLoader::ERROR_BAD_FRAME_DATA, "Bad animation frame data");
          std::cout << "Error: " << desc.description << "\n";
          errors.push_back(desc);
          return false;
        }
        
        bool hasSides = false;
        for (auto frameData = itr.value()[i].begin(); frameData != itr.value()[i].end(); ++frameData) {
          if (frameData.value().is_array() && frameData.key() == "sides") {
            hasSides = true;
            std::vector<AnimationLoader::LoadData::Side> sides(frameData.value().size(), AnimationLoader::LoadData::Side{ResourceID(), 0, false, false});
            for (size_t j = 0; j < frameData.value().size(); ++j) {
              if (frameData.value()[i].is_string()) {
                sides[i] = parseSideString(frameData.value()[i].get<std::string>());
                continue;
              }
              
              if (frameData.value()[i].is_object()) {
                bool hasImageId = false;
                for (auto side = frameData.value()[i].begin(); side != frameData.value()[i].end(); ++side) {
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
                }
                
                continue;
              }
            }
            
            info.frames.push_back(AnimationLoader::LoadData::Frame{sides});
            
            continue;
          }
        }
        
        if (!hasSides) {
          ErrorDesc desc(mark, AnimationLoader::ERROR_FRAME_MUST_HAVE_AT_LEAST_ONE_SIDE, "Frame must have at least one side");
          std::cout << "Error: " << desc.description << "\n";
          errors.push_back(desc);
        }
      }
      
      continue;
    }
  }
  
  if (!hasId) {
    ErrorDesc desc(mark, AnimationLoader::ERROR_ANIMATION_MUST_HAVE_AN_ID, "Animation must have an id");
    std::cout << "Error: " << desc.description << "\n";
    errors.push_back(desc);
    return false;
  }
  
  const size_t sideCount = info.frames[0].sides.size();
  for (size_t i = 1; i < info.frames.size(); ++i) {
    if (sideCount != info.frames[i].sides.size()) {
      ErrorDesc desc(mark, AnimationLoader::ERROR_EVERY_ANIMATION_FRAME_MUST_HAVE_EQUAL_NUMBER_OF_SIDES, "Every animation frame must have equal number of sides");
      std::cout << "Error: " << desc.description << "\n";
      errors.push_back(desc);
      return false;
    }
  }
  
  return errorsCount == errors.size();
}

AnimationLoader::AnimationLoader(const CreateInfo &info) : imageLoader(info.imageLoader) {}
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
    
    bool ret = checkAnimationJsonValidity(pathPrefix, data, 12512, info, errors, warnings);
    if (!ret) return false;
    
    auto ptr = tempData->create(info);
    resource.push_back(ptr);
    return true;
  }
  
  return false;
}
                            
bool AnimationLoader::forget(const ResourceID &name) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  const size_t index = findTempData(name);
  if (index == SIZE_MAX) return false;
  
  tempData->dataPool.deleteElement(tempData->dataPtr[index]);
  std::swap(tempData->dataPtr[index], tempData->dataPtr.back());
  tempData->dataPtr.pop_back();
  
  return true;
}

Resource* AnimationLoader::getParsedResource(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  const size_t index = findTempData(id);
  return index == SIZE_MAX ? nullptr : tempData->dataPtr[index];
}

const Resource* AnimationLoader::getParsedResource(const ResourceID &id) const {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  const size_t index = findTempData(id);
  return index == SIZE_MAX ? nullptr : tempData->dataPtr[index];
}

bool AnimationLoader::load(const ModificationParser* modifications, const Resource* resource) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  for (auto data : tempData->dataToLoad) {
    if (data->id() == resource->id()) return true;
  }
  
  const size_t index = findTempData(resource->id());
  if (index == SIZE_MAX) return false;
  
  tempData->dataToLoad.push_back(tempData->dataPtr[index]);
  
  return true;
}

bool AnimationLoader::unload(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  for (size_t i = 0; i < tempData->dataToLoad.size(); ++i) {
    if (tempData->dataToLoad[i]->id() == id) {
      std::swap(tempData->dataToLoad[i], tempData->dataToLoad.back());
      tempData->dataToLoad.pop_back();
      return true;
    }
  }
  
  return false;
}

void AnimationLoader::end() {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  for (auto loadData : tempData->dataToLoad) {
    std::vector<std::vector<Animation::Image>> frames(loadData->frames().size(), std::vector<Animation::Image>(loadData->frames()[0].sides.size()));
    
    for (size_t i = 0; i < loadData->frames().size(); ++i) {
      for (size_t j = 0; j < loadData->frames()[i].sides.size(); ++j) {
        frames[i][j].image = imageLoader->image(loadData->frames()[i].sides[j].image, loadData->frames()[i].sides[j].index);
        if (frames[i][j].image.index == UINT32_MAX || frames[i][j].image.layer == UINT32_MAX) throw std::runtime_error("Could not find an image "+loadData->frames()[i].sides[j].image.name()+" with index "+std::to_string(loadData->frames()[i].sides[j].index));
        frames[i][j].flipU = loadData->frames()[i].sides[j].flipU;
        frames[i][j].flipV = loadData->frames()[i].sides[j].flipV;
      }
    }
    
    Global::animations()->createAnimation(loadData->id(), Animation::CreateInfo{frames});
  }
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

size_t AnimationLoader::findTempData(const ResourceID &id) const {
  for (size_t i = 0; i < tempData->dataPtr.size(); ++i) {
    if (tempData->dataPtr[i]->id() == id) return i;
  }
  
  return SIZE_MAX;
}
