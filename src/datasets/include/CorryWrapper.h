#ifndef CORRYWRAPPER_H
#define CORRYWRAPPER_H

#include <thread>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <regex>
#include <ostream>
#include <cstdlib>

#include "util/include/ClipBoard.h"
#include "datasets/include/DataBase.h"
#include "datasets/include/EventReconstructor.h"

#include "util/include/json.hpp"
#include "util/include/util.hpp"
#include "util/include/logging.h"

#include "core/Corryvreckan.hpp"

namespace viz{
    //Current implementation disallows any modification of reconstructor once loaded into the wrapper
    //TODO: Potentially add functions that allow for swapping out EventLoaders (currently only EventLoaderYARR is supported)
    //      Add parsing of Global Tracks and loading on Clipboard.
    //      Add parsing of Multiplets and loading on Clipboard.
    //      SIGNALS
    class CorryWrapper{
        public:
            CorryWrapper() = delete;
            CorryWrapper(std::unique_ptr<EventReconstructor>&& event_reconstructor);

            //Internal files remain open on init. Closed on join of CorryWrapper
            bool configure(const json& config);
            void run();
            void join();

            std::unique_ptr<FEEvents>  getEvents();
            std::unique_ptr<TrackData> getTracks();
        private:
            void process();
            void toMemoryBinary(std::vector<uint8_t> &handle, const Event& event);
            void toFileBinary(std::fstream &handle, const Event& event);
            void readCorryFile(std::fstream& handle, const std::shared_ptr<ClipBoard<TrackData>>& track_clipboard);
            std::vector<float> getFloats(const std::string& input);
            bool isThreadRunning();

            static uint8_t totalInstances; //Current implmementation has its own folder for corryvreckan input/output. This is used for specifying the folder (corrywrapper_{id})
            uint8_t id; //If multiple CorryWrappers are made. Note: Corrywrappers should be made one-to-one to Detector
            std::filesystem::path internalsPath;

            bool run_thread, isInit;
            std::mutex run_thread_mutex;
            std::unique_ptr<std::thread> thread_ptr;
            std::unique_ptr<corryvreckan::Corryvreckan> corry;
            std::unique_ptr<logging::LoggerStreamBuf> corryStreamBuf;
            std::unique_ptr<std::ostream> corryStreamLogger;

            std::unique_ptr<EventReconstructor> reconstructor;
            std::vector<std::fstream> outputYARRFiles;
            std::vector<std::string> outputYARRFileNames;
            std::fstream inputCorryFile;
            std::shared_ptr<ClipBoard<FEEvents>> eventDataClip;
            std::shared_ptr<ClipBoard<TrackData>> trackDataClip; // Corryvreckan Tracks do not provide any constructors to store useful information about the constructed track
                                                                       // Therefore, must use vizualizer track instead
            std::string corryMainFileName, corryInputFileName;
            uint8_t totalFEs;
    };
}

#endif