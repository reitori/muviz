#ifndef IMGUIWINDOW_H
#define IMGUIWINDOW_H

#include "Window/Window.h"
#include "OpenGL/Renderer.h"
#include "OpenGL/Scene/Detector.h"
#include "util/include/util.hpp"

#include <cmath>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#include "Events/Event.h"

namespace viz{
    class GUIWindow : public Window{
        public:
            friend class GUIManager;
            GUIWindow() = delete;
            GUIWindow(const char* name, ImGuiWindowFlags windowFlags = 0) : Window(name), m_windowFlags(windowFlags) {}

            virtual void init() = 0;
            void render() override{
                preFrame();
                ImGui::Begin(m_name.c_str(), &m_isOpen, m_windowFlags);
                onRender();
                ImGui::End();
            }

            virtual void onEvent(const system::event& e) = 0;
            inline void setOpen(bool isOpen) {m_isOpen = isOpen;}

            virtual ~GUIWindow() = default;
        protected:
            virtual void onRender() = 0;
            virtual void preFrame() = 0;

            bool m_isOpen = true;
            ImGuiWindowFlags m_windowFlags = ImGuiWindowFlags_None;
    };

    class Dockspace : public GUIWindow{
        public:
            Dockspace() = delete;
            Dockspace(const char* name, ImGuiWindowFlags windowFlags = 0) : GUIWindow(name, windowFlags) {}

            void init() override;
            void onEvent(const system::event& e) override {}

            virtual ~Dockspace() = default;
        private:
            void onRender() override;
            void preFrame() override;
    };

    class SceneWindow : public GUIWindow {
        public:
            SceneWindow() = delete;
            SceneWindow(const char* name, ImGuiWindowFlags windowFlags = 0);

            void init() override {}
            void onEvent(const system::event& e) override;
            std::shared_ptr<Renderer> getRenderer() { return m_renderer; }
           
            virtual ~SceneWindow() = default;
        private:
            std::shared_ptr<Renderer> m_renderer;
            ImVec2 m_lastMouse, m_currMouse, m_currPos, m_currSize; //mouse positions are relative to window
            bool m_mousePressed = false, m_mouseInWin = false, m_RPressed = false;
            float zoomScale = 1.0f;

            void onRender() override;
            void preFrame() override {}

            bool mouseInWin(ImVec2 mouseRelativeToWin);
            inline ImVec2 mouseRelToWin(ImVec2 cursor) { return ImVec2(cursor.x - m_currPos.x, cursor.y - m_currPos.y);}
    };

    class ManagerWindow : public GUIWindow{
        public:
            float x = 0, y = 0, z = 0;
            float color[4] = {0, 0, 0, 1};
            float worldRot = 0.0f;
            float hitDuration = 0.1f;
            bool hitDurIsIndefinite = false;
            bool startCLI = false;
            bool CycleCamPath = false;
            bool goThroughPath = false;

            ManagerWindow() = default;
            ManagerWindow(const char* name, std::shared_ptr<Renderer> renderer);

            void init() override {};
            void onEvent(const system::event& e) override {}

            void attachDetector(std::shared_ptr<Renderer> renderer);

            virtual ~ManagerWindow() = default;
        private:
            int hitDurMin{0}, hitDurSec{1};
            std::shared_ptr<Renderer> m_renderer;

            ImFont* largerFont;
            void onRender() override;
            void preFrame() override {}
    };

    class ConsoleWindow : public GUIWindow{
        public:
            float fps = 0.0f;
            bool ScrollToBottom = false;
            std::vector<std::string> lines;

            ConsoleWindow() = default;
            ConsoleWindow(const char* name) : GUIWindow(name) {}
            void onEvent(const system::event& e) override {}

            void init() {}
            void AddLog(const char* fmt, ...) IM_FMTARGS(2);

            virtual ~ConsoleWindow() = default;
        private:
            void onRender() override;
            void preFrame() override {}
    };

    class ScrubberWindow : public GUIWindow{
        public:

            ScrubberWindow() = default;
            ScrubberWindow(const char* name, std::shared_ptr<Detector> detector) : GUIWindow(name), m_detector(detector) {}
            void onEvent(const system::event& e) override {}

            void init() {}
        private:
            void onRender() override;
            void preFrame() override {}

            std::shared_ptr<Detector> m_detector;
            double playhead_time = 0;
    };
}

#endif