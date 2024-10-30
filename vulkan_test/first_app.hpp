#pragma once

#include "lve_window.hpp"
#include "lve_game_object.hpp"
#include "lve_device.hpp"
#include "lve_model.hpp"
#include "lve_renderer.hpp"
#include <memory>
#include <vector>

namespace lve {
    class FirstApp {


    public:
        static constexpr int WIDTH = 1000;
        static constexpr int HEIGHT = 1000;

        void run();

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp&) = delete;
        FirstApp& operator=(const FirstApp&) = delete;

    private:
        void loadGameObjects();

        LveWindow lveWindow{ WIDTH, HEIGHT, "Hello Vulkan!" };
        LveDevice lveDevice{ lveWindow };
        LveRenderer lveRenderer{ lveWindow, lveDevice };

        std::vector<LveGameObject> gameObjects;
    };
}