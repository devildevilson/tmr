#include "FileTools.h"

#if defined (_WIN32)
  #include <windows.h>
#elif defined(__linux__)
  #include <sys/types.h>
  #include <unistd.h>
#endif

namespace FileTools {
  std::vector<char> readFile(const std::string& filename) {
    //std::cout << "File reading start: " << filename << std::endl;
    std::ifstream file;
    file.open(filename, std::ios::ate | std::ios::binary);
    //std::cout << "File reading end"<< std::endl;

    if (!file) {
      throw std::runtime_error("failed to open file!");
    } 
    // непонимаю почему, но это падает
    // else if (file.is_open()) {
    //  throw std::runtime_error("failed to open file!");
    //}
    //std::cout << "Error before buffer creation" << std::endl;
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    return buffer;
  }
  
  std::string getAppDir() {
    std::string path;
#if defined(_WIN32)
	size_t len = 4096;
    char* pBuf = nullptr;
    while(1) {
      pBuf = new char[len + 1];
      uint32_t r = GetModuleFileName(NULL, pBuf, len);
      if (r < len && r != 0) break;
      if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        delete [] pBuf;
        len += 64;
      }
    }
      
    char* pch = strrchr(pBuf, '\\');
    *(pch+1) = '\0';
#elif defined(__linux__)
	size_t len = 4096;
    char pBuf[len];
	
    char szTmp[32];
    sprintf(szTmp, "/proc/%d/exe", getpid());
	ssize_t length = readlink(szTmp, pBuf, len-1);
	if (length == -1) {
	  // errors
	}
	pBuf[length] = '\0';
	char* pch = strrchr(pBuf, '/');
	*(pch+1) = '\0';
#endif
    
    path = std::string(pBuf);
	
#if defined(_WIN32)
	delete [] pBuf;
#endif

    return path;
  }

  void createLogFile(std::vector<ConsoleLine> history) {
    time_t now;
    tm currentTime;
    std::ofstream fOut;

    time(&now);
    currentTime = *localtime(&now);

    std::stringstream ss;
    ss << "log_" << currentTime.tm_year+1900 << "_" << currentTime.tm_mon+1 << "_" << currentTime.tm_mday << "-" << currentTime.tm_hour << "_" << currentTime.tm_min << "_" << currentTime.tm_sec << ".txt";
    
    std::string path = getAppDir();
    char* buf = new char[path.length()+ss.str().length()];
    strcat(buf, path.c_str());
    strcat(buf, ss.str().c_str());

    fOut.open(buf, std::ios::out | std::ios::app);
    //std::cout << "File " << strcat(getAppDir(), ss.str().c_str()) << " is " << fOut.is_open() << std::endl;
    for (size_t i = 0; i < history.size(); i++) {
      fOut << history[i].string;
    }
    fOut.close();
  }
}
