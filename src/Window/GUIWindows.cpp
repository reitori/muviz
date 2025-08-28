#include "GUIWindows.h"

namespace{
    auto sceneLogger = logging::make_log("SceneWindow");
    auto dockspaceLogger = logging::make_log("DockspaceLogger");
    auto managerLogger = logging::make_log("ManagerLogger");
    auto consoleLogger = logging::make_log("ConsoleLogger");
}

namespace viz
{
    void Dockspace::init(){
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        m_windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        m_windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin(m_name.c_str(), nullptr, m_windowFlags);
            ImGui::PopStyleVar();
            ImGui::PopStyleVar(2);
            //           
            ImGuiID dockspace_id = ImGui::GetID(m_name.c_str());
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspace_id,     static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_None) |
                                                        static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_DockSpace));
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
            //
            // split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
            //   window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
            //                                                              out_id_at_dir is the id of the node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction
            ImGuiID dock_id_right, dock_id_down;
                        
            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, &dock_id_right, &dockspace_id);
            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, &dock_id_down, &dockspace_id);
                        // we now dock our windows into the docking node we made above
            ImGui::DockBuilderDockWindow("Manager", dock_id_right);
            ImGui::DockBuilderDockWindow("Scrubber Window", dock_id_down);
            ImGui::DockBuilderDockWindow("Scene", dockspace_id);
            ImGui::DockBuilderFinish(dockspace_id);
        ImGui::End();

        dockspaceLogger->info("Dockspace initialized");
    }

    void Dockspace::onRender(){
        ImGui::PopStyleVar();
        ImGui::PopStyleVar(2);

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID(m_name.c_str());
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        }
    }

    void Dockspace::preFrame(){
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }

    SceneWindow::SceneWindow(const char* name, ImGuiWindowFlags windowFlags) : GUIWindow(name, windowFlags){
        m_renderer = std::make_shared<Renderer>(1, 1); //Arbitrary size that is updated onRender()
    }

    void SceneWindow::onRender(){
        if(m_renderer){
            ImVec2 windim = ImGui::GetContentRegionAvail();
            m_currPos = ImGui::GetWindowPos();
            m_currSize = ImGui::GetWindowSize();

            if(windim.x != m_renderer->getWidth() || windim.y != m_renderer->getHeight()){
                m_renderer->resize(windim.x, windim.y);
            }

            m_renderer->render();

            ImVec2 topRight = ImVec2(m_currPos.x + m_currSize.x, m_currPos.y + m_currSize.y);
            ImGui::GetWindowDrawList()->AddImage(m_renderer->getTexID(), m_currPos, topRight, ImVec2(0, 1), ImVec2(1, 0));
        }
    }

    void SceneWindow::onEvent(const system::event& e){
        system::eventType type = e.getEventType();
        const system::eventData* data = e.getData();
        switch (type)
        {
            case system::eventType::mouseButtonPress: {
                if(data->mouseButton == system::mouse::mouseCodes::ButtonRight)
                    m_mousePressed = true;
                return;
            }
            case system::eventType::mouseButtonRelease:{
                m_mousePressed = false;
                return;
            }
            case system::eventType::keyPress:{
                if(data->keyButton == system::key::R){
                    m_RPressed = true;
                    return;
                }
                return;
            }
            case system::eventType::keyRelease:{
                if(data->keyButton == system::key::R){
                    m_RPressed = false;
                    return;
                }
                return;
            }
            case system::eventType::mouseMove:{
                ImVec2 mousePos = mouseRelToWin(ImVec2(data->floatPairedData.first, data->floatPairedData.second));
                if(mouseInWin(mousePos)){
                    m_mouseInWin = true;

                    if(m_mousePressed){
                        Camera* cam = m_renderer->getCamera();
                        ImVec2 dir = ImVec2(mousePos.x - m_lastMouse.x, m_lastMouse.y - mousePos.y); //note reversal in y-coordinate is due to glfw coordinate system is from top down
                        glm::vec3 disp = 0.75f * glm::normalize(dir.x * cam->getRight() + dir.y * cam->getUp());

                        if(m_RPressed){ //rotate camera
                            cam->rotateAxis(glm::normalize(glm::cross(cam->getFront(), disp)), 0.5f);
                        }else{ //otherwise displace
                            cam->displace(-disp, m_renderer->getDelTime());
                        }

                        //I think this is a design flaw even if sorting when you only need to is more optimal
                        //Maybe sort upon each render call that way you don't lose track of when you sort
                        m_renderer->sortTransparentObjects();
                    }
                }  
                else{
                    m_mouseInWin = false;
                }
                m_lastMouse = mousePos;
                return;
            }    
            case system::eventType::mouseScroll: {
                if(m_mouseInWin){
                    Camera* cam = m_renderer->getCamera();

                    cam->displace(cam->getFront() * 0.75f * e.getData()->floatPairedData.second, m_renderer->getDelTime());
                    m_renderer->sortTransparentObjects();
                }
                return;
            }
        
            default:
                break;
        }
    }

    bool SceneWindow::mouseInWin(ImVec2 mouseRelativeToWin){
        if(mouseRelativeToWin.x < m_currSize.x && mouseRelativeToWin.x > 0.0f && mouseRelativeToWin.y < m_currSize.y && mouseRelativeToWin.y > 0.0f)
            return true;

        return false;
    }

    ManagerWindow::ManagerWindow(const char* name, std::shared_ptr<Renderer> renderer) : GUIWindow(name){
        m_renderer = renderer;
    }

    void ManagerWindow::attachDetector(std::shared_ptr<Renderer> renderer){
        m_renderer = renderer;
    }

    void ManagerWindow::onRender(){
        //ImGui::ColorPicker4("Change Screen", color);
        if(ImGui::Button("Start")){
            startCLI = true;
        }else{
            startCLI = false;
        }
        
        ImGui::Text("Track Duration:");
        ImGui::SameLine();

        if(hitDurIsIndefinite)
            ImGui::BeginDisabled();

        ImGui::PushItemWidth(ImGui::GetFontSize() * 5);
        ImGui::InputInt("##hitDurMin", &hitDurMin);
        ImGui::SameLine();
        ImGui::TextUnformatted("min");
        if(hitDurMin < 0) hitDurMin = 0;

        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetFontSize() * 5);
        ImGui::InputInt("##hitDurSec", &hitDurSec);
        ImGui::SameLine();
        ImGui::TextUnformatted("sec");
        if(hitDurSec < 0) hitDurSec = 0;

        if(hitDurIsIndefinite)
            ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::Checkbox("indefinite", &hitDurIsIndefinite);
        ImGui::PopItemWidth();

        Camera* cam = m_renderer->getCamera();
        ImGui::Text("Camera Speed");
        ImGui::SameLine();
        ImGui::PushItemWidth(200.0f);
        ImGui::SliderFloat("##CameraSpeed", &cam->speed, 1.0f, 100.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::SameLine();
        ImGui::Text("Cycle Path");
        ImGui::SameLine();
        if(goThroughPath)
            ImGui::BeginDisabled();

        ImGui::Checkbox("##CyclePath", &CycleCamPath);
        if(CycleCamPath){
            cam->cyclePath(m_renderer->getDelTime());
        }

        if(goThroughPath)
            ImGui::EndDisabled();

        if(CycleCamPath)
            ImGui::BeginDisabled();

        ImGui::SameLine();
        ImGui::Text("Forward");
        ImGui::SameLine();
        ImGui::Checkbox("##ThroughPath", &goThroughPath);
        if(goThroughPath){
            if(cam->pathForward(m_renderer->getDelTime())){
                cam->restartCamPath();
            }
        }

        if(CycleCamPath)
            ImGui::EndDisabled();

        
        hitDuration = 60 * hitDurMin + hitDurSec;

        std::vector<Chip>& chips = m_renderer->getDetector()->getChips();
        for(int i = 0; i < chips.size(); i++){
            if(ImGui::CollapsingHeader(chips[i].name.c_str())){
                ImGui::Separator();
                ImGui::Text("Name: %s", chips[i].name.c_str());
                ImGui::Text("fe_id: %i", chips[i].fe_id);
                ImGui::Text("Position: (%.2f, %.2f, %.2f)", chips[i].pos.x, chips[i].pos.y, chips[i].pos.z);
                ImGui::Text("Hits: %lu", chips[i].hits);

                if(chips[i].isHitMapEnabled()){
                    ImGui::Image((intptr_t)chips[i].getTexture()->getID(), ImVec2(chips[i].getMaxCols(), chips[i].getMaxRows()));
                }
            }
        } 
    }

    void ConsoleWindow::onRender(){
        ImGui::Text("FPS: %.1f", fps);

        ImGui::Separator();
        if (ImGui::BeginChild("Log", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true)) {
            for (const auto& line : lines) {
                ImGui::TextUnformatted(line.c_str());
            }
            if (ScrollToBottom)
                ImGui::SetScrollHereY(1.0f);
            ScrollToBottom = false;
        }
        ImGui::EndChild();
    }

    void ConsoleWindow::AddLog(const char* fmt, ...) {
        if(lines.size() > 1024){
            lines.clear();
        }

        va_list args;
        va_start(args, fmt);
        char buf[1024];
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        va_end(args);
        lines.push_back(buf);
        ScrollToBottom = true;
    }

    void ScrubberWindow::onRender(){
        // Use the entire window as the scrubber surface
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImVec2 p1 = ImVec2(p0.x + ImGui::GetContentRegionAvail().x,
                        p0.y + ImGui::GetContentRegionAvail().y);
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // Background
        dl->AddRectFilled(p0, p1, IM_COL32(40, 40, 40, 255));

        auto& ebt = m_detector->eventBatchTimestamps;
        bool ebtWidthDefined = m_detector->eventBatchTimestamps.getSize() >= 2;
        double timeline_start = ebtWidthDefined ? std::chrono::duration<double>(ebt[0].second - ApplicationStartTimePoint).count() : 0;
        double timeline_end   =  std::chrono::duration<double>(std::chrono::steady_clock::now()- ApplicationStartTimePoint).count();
        double playhead_time   =  ebtWidthDefined ? std::chrono::duration<double>(m_detector->timeSincePlaybackReset - ApplicationStartTimePoint).count() : timeline_end;

        // Helper: map time to x
        auto TimeToScreen = [&](float t) {
            float norm = (t - timeline_start) / (timeline_end - timeline_start);
            return p0.x + norm * (p1.x - p0.x);
        };
        auto ScreenToTime = [&](float x) {
            float norm = (x - p0.x) / (p1.x - p0.x);
            return timeline_start + norm * (timeline_end - timeline_start);
        };

        // Draw ticks
        float total_range = timeline_end - timeline_start;

        float pixels_per_second = (p1.x - p0.x) / total_range;
        float min_label_spacing_px = 80.0f; // Minimum pixel gap between labels
        float raw_spacing_seconds = min_label_spacing_px / pixels_per_second;

        // Round spacing to a “nice” value
        // Multiples of 1, 3, 5, 10, 15, 30 seconds, then minutes
        float nice_steps[] = {1, 3, 5, 10, 15, 30, 60, 120, 300, 600}; 
        float tick_spacing = nice_steps[0];
        for (float step : nice_steps) {
            if (step >= raw_spacing_seconds) {
                tick_spacing = step;
                break;
            }
        }

        for (float t = floorf(timeline_start / tick_spacing) * tick_spacing; t <= timeline_end; t += tick_spacing){
            float x = TimeToScreen(t);
            dl->AddLine(ImVec2(x, p0.y), ImVec2(x, p1.y), IM_COL32(200, 200, 200, 100));

            char buf[32];
            int minutes = (int)t / 60;
            int seconds = (int)fabsf(t) % 60;
            snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);

            dl->AddText(ImVec2(x + 2, p0.y + 2), IM_COL32(220, 220, 220, 255), buf);
        }

        // Draw events
        for (int i = 0; i < ebt.getSize(); i++) {
            double eventTime = std::chrono::duration<double>(ebt[i].second - ApplicationStartTimePoint).count();
            float ex0 = TimeToScreen(eventTime);
            float ex1 = TimeToScreen(eventTime + 0.1);
            dl->AddRectFilled(ImVec2(ex0, p0.y + 20), ImVec2(ex1, p1.y - 5), IM_COL32(150, 180, 220, 255), 3.0f);
        }

        // Interactions
        ImGuiIO& io = ImGui::GetIO();
        bool hovered = ImGui::IsWindowHovered();

        if (hovered) {
            // Drag playhead
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                playhead_time = ScreenToTime(io.MousePos.x);
                auto sinceFirstOnEBT = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double>(playhead_time - timeline_start));
                if(ebtWidthDefined) m_detector->setPlayback(ebt[0].second + sinceFirstOnEBT);
                if(io.MousePos.x / (p1.x - p0.x) > 0.985){
                    m_detector->setRealtime();
                }
            }
            // Zoom
            if (io.MouseWheel != 0.0f) {
                float zoom_factor = powf(0.9f, io.MouseWheel);
                float mid_time = ScreenToTime(io.MousePos.x);
                float range_left = (mid_time - timeline_start) * zoom_factor;
                float range_right = (timeline_end - mid_time) * zoom_factor;
                timeline_start = mid_time - range_left;
                timeline_end   = mid_time + range_right;
            }
            // Pan
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                float dt = io.MouseDelta.x / (p1.x - p0.x) * (timeline_end - timeline_start);
                timeline_start -= dt;
                timeline_end   -= dt;
            }
        }


        // Draw playhead
        float px = TimeToScreen(playhead_time);
        if(m_detector->isRealtime()){ //Clamp to end if realtime
            px = p1.x;
        }
        dl->AddLine(ImVec2(px, p0.y), ImVec2(px, p1.y), IM_COL32(255, 0, 0, 255), 2.0f);
    }
}
