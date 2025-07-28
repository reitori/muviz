#include "datasets/include/CorryWrapper.h"


namespace{
    auto logger = logging::make_log("CorryWrapper");
}

namespace viz{
    uint8_t CorryWrapper::totalInstances = 0;

    CorryWrapper::CorryWrapper(std::unique_ptr<EventReconstructor>&& event_reconstructor) : run_thread_mutex() {
        logger->info("Initiaizlizing CorryWrapper instance {}", totalInstances);

        run_thread = false;
        isInit = false;
        reconstructor = std::move(event_reconstructor);

        totalFEs = reconstructor->getTotalFEs();

        corryStreamBuf = std::make_unique<logging::LoggerStreamBuf>(logger, spdlog::level::err);
        corryStreamLogger = std::make_unique<std::ostream>(static_cast<std::streambuf*>(corryStreamBuf.get()));

        eventDataClip = std::make_shared<ClipBoard<FEEvents>>();
        trackDataClip = std::make_shared<ClipBoard<TrackData>>();

        outputYARRFiles.resize(totalFEs);
        outputYARRFileNames.resize(totalFEs);

        id = totalInstances;
        totalInstances++;
    }


    bool CorryWrapper::configure(const json& config){
        if(isInit){
            //TODO: Update this to detector whenever you change FEBookie to operate with detectors
            //      Want to change so it displays detector name
            logger->warn("Corrywrapper is already init for chips");
            return isInit;
        }

        std::string corryGeometryFileName;
        std::string configGeometryPath;
        //Require Corryvreckan geometry (or detector) file.
        if(!config.contains("corryvreckan_geometry_file") || !config["corryvreckan_geometry_file"].is_string()){
            logger->error("Please provide parameter \"corryvreckan_geometry_file\" in global source configurations and ensure parameter is string");
            return false;
        }
        corryGeometryFileName = (std::string)config["corryvreckan_geometry_file"];

        //Check to make sure the geometry file exists:
        std::ifstream geometryFile(corryGeometryFileName.c_str(), std::ios::in);
        if(!geometryFile.is_open()){
            logger->error("Could not open Corryvreckan geometry file specified in \"corryvreckan_geometry_file\" parameter! Path is: {}", corryGeometryFileName);
            return false;
        }

        std::string userCorryFileName;
        bool useUserCorryFile = false;
        //Search for Corryvreckan main file
        if(config.contains("corryvreckan_main_file")){
            if(config["corryvreckan_main_file"].is_string()){
                userCorryFileName = config["corryvreckan_main_file"];
                useUserCorryFile = true;
            }else{
                logger->warn("Ensure that corryvreckan_main_file is a string. Using default Corryvreckan main file instead.");
            }
        }else{
            logger->info("corryvreckan_main_file parameter is not set. Using default Corryvreckan main file instead.");
        }

        std::string userCorryLibPath;
        if(config.contains("corryvreckan_lib_folder")){
            if(config["corryvreckan_lib_folder"].is_string()){
                userCorryLibPath = config["corryvreckan_lib_folder"];
            }else{
                logger->error("Please provide a valid path to corryvreckan libraries");
            }
        }else{
            logger->error("corryvreckan_lib_path parameter is not set.");
        }

        //Set folder path for intermediary Corryvreckan input data and output data
        std::filesystem::path executableDirectory = viz::getExecutableDir();
        std::string folderName = std::string("corrywrapper_") + std::to_string(id);
        internalsPath = executableDirectory / folderName;
        std::filesystem::create_directory(internalsPath);

        //Create intermediary YARR files
        for(int i = 0; i < outputYARRFiles.size(); i++){
            //Create and open YARR files
            std::fstream& currYARRFile = outputYARRFiles[i];
            std::string YARRFileName = internalsPath.string() + std::string("/") + FEBookie::getName(i) + std::string("_data.raw");
            currYARRFile.open(YARRFileName.c_str(), std::fstream::out | std::fstream::binary | std::fstream::trunc);
            outputYARRFileNames[i] = YARRFileName;
            if(!currYARRFile.is_open()){
                logger->error("Failed to create YARR intermediary file for chip {0}.", FEBookie::getName(i));
                return false;
            }
            currYARRFile.close();
        }

        //Create intermediary Corryvreckan data file (from the TextWriter)
        corryInputFileName = internalsPath.string() + std::string("/") + std::string("corry_data.txt");

        //Make sure that the corryvreckan data file exists before reading and truncate it
        inputCorryFile.open(corryInputFileName.c_str(), std::fstream::out | std::fstream::trunc);
        if(!inputCorryFile.is_open()){
            logger->error("Failed to create Corryvreckan intermediary file.");
            return false;
        }
        inputCorryFile.close();

        inputCorryFile.open(corryInputFileName.c_str(), std::fstream::in);
        if(!inputCorryFile.is_open()){
            logger->error("Failed to open the Corryvreckan intermediary file.");
            return false;
        }
        inputCorryFile.close();

        //Create the corry main file
        corryMainFileName = internalsPath.string() + std::string("/") + std::string("corry_main.conf");
        std::ofstream corryMainFile(corryMainFileName.c_str(), std::ios::trunc);
        if(!corryMainFile.is_open()){
            logger->error("Could not create corry main file");
            return false;
        }

        //Construct configuration file by either:  
        // 1. Copying the Corryvreckan file given by the user and replacing the [Corryvreckan] module with the below
        // 2. Creating and using a hardcoded Corryvreckan module file
        corryMainFile << "[Corryvreckan]\n"
                      << "detectors_file = \"" << corryGeometryFileName << "\" \n"
                      << "output_directory = \"" << internalsPath.string() << "\" \n"
                      << "library_directories = \"" << userCorryLibPath << "\"\n"
                      << "histogram_file = \"\"\n" 
                      << "log_level = \"ERROR\"\n\n";
 
        if(useUserCorryFile){ //Copy Corryvreckan main configuration file given by user. Ensure TextWriter is used properly at the end by either changing parameters or appending to end of .conf file
            std::ifstream userCorryFile(userCorryFileName.c_str());
            if(userCorryFile.is_open()){
                
                //Lambda functions to remove TOML comments (which is the format Corryvreckan configs follow) and finding new Tables
                auto isInsideString = [](const std::string& line, size_t pos){
                                            bool in_single = false, in_double = false;
                                            for (size_t i = 0; i < pos; ++i) {
                                                if (line[i] == '\'' && !in_double) {
                                                    in_single = !in_single;
                                                } else if (line[i] == '"' && !in_single) {
                                                    in_double = !in_double;
                                                } else if (line[i] == '\\' && i + 1 < pos) {
                                                    ++i; // Skip escaped character
                                                }
                                            }

                                            return in_single || in_double;
                                        };

                auto removeTOMLComment =  [&isInsideString](const std::string& line){
                                                for (size_t i = 0; i < line.length(); ++i) {
                                                    if (line[i] == '#' && !isInsideString(line, i)) {
                                                        return line.substr(0, i); // Strip from '#' onwards
                                                    }
                                                }
                                                return line;
                                            };

                auto isTableDefinition = [](const std::string& line){
                                                std::string trimmed = line;
                                                trimmed.erase(0, trimmed.find_first_not_of(" \t")); // Trim leading whitespace

                                                if (trimmed.empty() || trimmed[0] != '[')
                                                    return false;

                                                size_t closing = trimmed.find(']');
                                                return closing != std::string::npos && trimmed.find_first_not_of(" \t", closing + 1) == std::string::npos;
                                            };

                userCorryFile.seekg(0, std::ios::beg);

                bool currTableIsEventLoader = false;
                std::string line;
                while(std::getline(userCorryFile, line)){
                    removeTOMLComment(line);

                    if(isTableDefinition(line)){
                        //Skip over global Corryvreckan parameters for visualizer's replacement
                        if(line.find(std::string("[Corryvreckan]")) != std::string::npos){
                            while(std::getline(userCorryFile, line) && !isTableDefinition(line)){}
                        }

                        //Skip over TextWriter parameters for visualizer's replacement
                        if(line.find(std::string("[TextWriter]")) != std::string::npos){
                            while(std::getline(userCorryFile, line) && !isTableDefinition(line)){}
                            corryMainFile << "file_name = \"corry_data.txt\"\n";
                        }
                        
                        if(line.find(std::string("EventLoader")) != std::string::npos){
                            currTableIsEventLoader = true;
                        }else{
                            currTableIsEventLoader = false;
                        }
                    }

                    if(currTableIsEventLoader && (line.find("input_directory") != std::string::npos)){
                        //Change input directory towards the internals
                        corryMainFile << "input_directory = \"" << internalsPath.string() << "\"\n";
                    }else{
                        //Otherwise copy everything over
                        corryMainFile << line << "\n";
                    }
                }

                //Finish up with the TextWriter
                corryMainFile << "[TextWriter]\n"
                              <<  "file_name = \"corry_data.txt\"";
            }else{
                logger->warn("Cannot find Corryvreckan main configuration file \"{0}\". Using default configuration instead", userCorryFileName);
                useUserCorryFile = false;
            }
        }
        if(!useUserCorryFile){ //Default Corryvreckan main configuration
            corryMainFile << "[EventLoaderYarr]\n"
                          << "input_directory = \"" << internalsPath.string() << "\"\n\n"
                          << "[ClusteringSpatial]\n\n"
                          << "[Tracking4D]\n\n"
                          << "[TextWriter]\n"
                          <<  "file_name = \"corry_data.txt\"";
        }
        //Write to file
        corryMainFile.flush();

        //Taken from https://gitlab.cern.ch/corryvreckan/corryvreckan/-/blob/master/src/exec/corry.cpp
        //corryvreckan::Log::addStream(*corryStreamLogger);
        corryvreckan::Log::setReportingLevel(corryvreckan::LogLevel::FATAL);

        isInit = true;
        return true;
    }

    void CorryWrapper::run(){
        run_thread_mutex.lock();
        run_thread = true;
        run_thread_mutex.unlock();
        thread_ptr.reset(new std::thread(&CorryWrapper::process, this));
    }

    void CorryWrapper::join(){
        run_thread_mutex.lock(); //Ensure that there is no thread racing whenever checking isThreadRunning in proces()
        run_thread = false;
        run_thread_mutex.unlock();

        thread_ptr->join();

        while(eventDataClip->size() > 0) {
            auto raw = eventDataClip->popData();
            raw.reset();
        }  
        while(trackDataClip->size() > 0) {
            auto raw = trackDataClip->popData();
            raw.reset();
        }   
    }

    std::unique_ptr<FEEvents> CorryWrapper::getEvents(){
        return std::move(eventDataClip->popData());
    }

    std::unique_ptr<TrackData> CorryWrapper::getTracks(){
        return std::move(trackDataClip->popData());
    }

    void CorryWrapper::process(){

        // long total_usTime = 0.0;
        // uint32_t totalLoops = 0;

        while(isThreadRunning()){
            std::unique_ptr<FEEvents> currFEEvents = std::move(reconstructor->getEvents());
            if(!currFEEvents){
                std::this_thread::yield();
                continue;
            }

            // totalLoops++;
            // auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < totalFEs; i++) {
                outputYARRFiles[i].open(outputYARRFileNames[i].c_str(), std::fstream::out | std::fstream::trunc);
                auto& currEvents = (*currFEEvents)[i]->events;
                unsigned int totalHits = 0;

                // Write to file
                for (Event& event : currEvents) {
                    toFileBinary(outputYARRFiles[i], event);
                    totalHits += event.hits.size();
                }
                
                (*currFEEvents)[i]->nHits = totalHits;
                outputYARRFiles[i].close();
            }

            //Run corryvreckan
            //Corryvreckan object unfortunately does not provide any other way other than reading off the configuration file
            //to load and initialize the modules used in the run() (i.e. no interface for passing in a ConfigManager into Corryvreckan object).
            //Means we have to take a roundabout approach of creating a Corry configuration file and passing its path into Corryvreckan object.
            corry.reset(new corryvreckan::Corryvreckan(corryMainFileName));
            corry->load();
            corry->init();
            corry->run();
            corry->finalize();

            // Read off output txt files, parse for Tracks, and load onto clipboard
            inputCorryFile.open(corryInputFileName.c_str(), std::ios::in);
            readCorryFile(inputCorryFile, trackDataClip);
            inputCorryFile.close();

            // Finally put data on the EventData clipboards
            eventDataClip->pushData(std::move(currFEEvents));

            // auto end = std::chrono::high_resolution_clock::now();
            // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            // long elapsedMicroseconds = static_cast<long>(duration.count());
            // total_usTime += elapsedMicroseconds;
        }


        // std::cout << total_usTime / totalLoops << std::endl;
    }

    void CorryWrapper::toMemoryBinary(std::vector<uint8_t> &handle, const Event& event){
        size_t data_size = sizeof(uint32_t) + (3 * sizeof(uint16_t)) + (event.hits.size() * sizeof(Hit));
        size_t offset = handle.size();
        handle.resize(offset + data_size);
        
        memcpy(handle.data() + offset, &event.tag, sizeof(uint32_t));   offset += sizeof(uint32_t);
        memcpy(handle.data() + offset, &event.l1id, sizeof(uint16_t));  offset += sizeof(uint16_t);
        memcpy(handle.data() + offset, &event.bcid, sizeof(uint16_t));  offset += sizeof(uint16_t);
        memcpy(handle.data() + offset, &event.nHits, sizeof(uint16_t)); offset += sizeof(uint16_t);
        if(event.hits.data())
            memcpy(handle.data() + offset, event.hits.data(), event.hits.size() * sizeof(Hit));
    }


    void CorryWrapper::toFileBinary(std::fstream& handle, const Event& event){
        std::vector<uint8_t> buffer;
        toMemoryBinary(buffer, event);
        handle.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    }

    std::vector<float> CorryWrapper::getFloats(const std::string& input) {
        std::vector<float> results;
        // This regex matches floats, including optional signs, decimals, and scientific notation
        std::regex floatRegex(R"([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)");
        std::smatch match;
        std::string::const_iterator searchStart(input.cbegin());

        while (std::regex_search(searchStart, input.cend(), match, floatRegex)) {
            results.push_back(std::stof(match[0]));
            searchStart = match.suffix().first;
        }

        return std::move(results);
    }

    void CorryWrapper::readCorryFile(std::fstream& handle, const std::shared_ptr<ClipBoard<TrackData>>& track_clipboard){
        std::string line;
        std::unique_ptr<TrackData> tempTracks = std::make_unique<TrackData>();
        handle.seekg(0);
        while(std::getline(handle, line)){
            //Currently Corry has three different types of Track (GlbTrack, Multiplet, StraightLineTrack)
            //Format:
            //StraightLineTrack (x,y,z), (t1,t2,t3), chi2, ndof, chi2ndof, timestamp
            if(line.find("StraightLineTrack") != std::string::npos){
                auto currTrack = std::make_unique<viz::StraightLineTrack>();

                std::vector<float> sltData = getFloats(line.substr(19)); //Prase line for relevant track data, should be 10 floats.
                if(sltData.size() == 10){
                    currTrack->trackType = viz::Track::type::straight;
                    currTrack->point = glm::vec3(sltData[0], sltData[1], sltData[2]);
                    currTrack->direction = glm::vec3(sltData[3], sltData[4], sltData[5]);
                    currTrack->uncertainties = glm::vec4(sltData[6], sltData[7], sltData[8], sltData[9]);

                    tempTracks->emplace_back(std::move(currTrack));
                }
                else{
                    logger->error("Corryvreckan track reconstruction data corruption");
                }
            }

            else if(line.find("GlblTrack") != std::string::npos){
                //TODO: Fill this portion
            }

            else if(line.find("Multiplet") != std::string::npos){
                //TODO: Fill this portion
            }
        }

        track_clipboard->pushData(std::move(tempTracks));
    }

    bool CorryWrapper::isThreadRunning(){
        std::lock_guard<std::mutex> guard(run_thread_mutex);
        return run_thread && isInit;
    }

}
