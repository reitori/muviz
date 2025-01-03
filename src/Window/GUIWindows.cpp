#include "GUIWindows.h"

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
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_DockSpace);
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
            ImGui::DockBuilderDockWindow("Console", dock_id_down);
            ImGui::DockBuilderDockWindow("Scene", dockspace_id);
            ImGui::DockBuilderFinish(dockspace_id);
        ImGui::End();

        m_appLogger->info("Dockspace initialized");
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

    void SceneWindow::onRender(){
        if(m_renderer){
            ImVec2 windim = ImGui::GetContentRegionAvail();
            ImVec2 winpos = ImGui::GetWindowPos();
            ImVec2 winsize = ImGui::GetWindowSize();

            glViewport(0, 0, windim.x, windim.y);
            if(windim.x != m_renderer->getWidth() || windim.y != m_renderer->getHeight()){
                m_renderer->resize(windim.x, windim.y);
            }

            ImVec2 topRight = ImVec2(winpos.x + winsize.x, winpos.y + winsize.y);
            ImGui::GetWindowDrawList()->AddImage(m_renderer->getTexID(), winpos, topRight, ImVec2(0, 1), ImVec2(1, 0));
        }
    }

    void ManagerWindow::onRender(){
        ImGui::SliderFloat("x", &x, -50.0f, 50.0f);
        ImGui::SliderFloat("y", &y, -50.0f, 50.0f);
        ImGui::SliderFloat("z", &z, -100.0f, 100.0f);
        ImGui::SliderFloat("Worldrotate", &worldRot, 0.0f, 360.0f);
        ImGui::ColorPicker4("Change Screen", color);
    }

    void ConsoleWindow::onRender(){
        ImGui::Text("FPS: %.1f", fps);
    }
}
