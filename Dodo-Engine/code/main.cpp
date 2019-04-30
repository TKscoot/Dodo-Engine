#include "environment/Window.h"
#include "environment/Log.h"
#include "environment/Error.h"
#include "engine/Engine.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>

using namespace Dodo;
using namespace Dodo::Environment;
using namespace Dodo::Engine;

int main()
{
	std::unique_ptr<CEngine> engine = std::make_unique<CEngine>();
	engine->Initialize();
	engine->Run();

	return 0;
}