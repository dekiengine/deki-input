#include "DekiInput.h"
#include <deki/LogSystem.h>

namespace DekiInput
{

// Static member definitions
std::map<std::string, std::unique_ptr<IDekiInput>> DekiInput::s_ActiveInputs;
std::vector<InputEventCallback> DekiInput::s_GlobalCallbacks;
bool DekiInput::s_Initialized = false;
bool DekiInput::s_ShouldExit = false;

bool DekiInput::SetInput(std::unique_ptr<IDekiInput> input, const std::string& name)
{
    if (!input)
    {
        DEKI_LOG_ERROR("DekiInput::SetInput: null input");
        return false;
    }

    // Check if already registered
    if (s_ActiveInputs.find(name) != s_ActiveInputs.end())
    {
        DEKI_LOG_INTERNAL("Input package '%s' already registered", name.c_str());
        return true;
    }

    // Register internal callback to distribute events
    input->RegisterEventCallback([](const InputEvent& event) { DistributeEvent(event); });

    // Store the input
    s_ActiveInputs[name] = std::move(input);
    s_Initialized = true;

    DEKI_LOG_INTERNAL("DekiInput: Input '%s' set", name.c_str());
    return true;
}

void DekiInput::Shutdown()
{
    if (!s_Initialized)
    {
        return;
    }

    // Shutdown all active inputs
    for (auto& [name, input] : s_ActiveInputs)
    {
        if (input)
        {
            input->Shutdown();
        }
    }

    s_ActiveInputs.clear();
    s_GlobalCallbacks.clear();
    s_Initialized = false;
    s_ShouldExit = false;

    DEKI_LOG_INTERNAL("DekiInput shutdown");
}

IDekiInput* DekiInput::GetInput(const std::string& name)
{
    auto it = s_ActiveInputs.find(name);
    if (it != s_ActiveInputs.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void DekiInput::RegisterEventCallback(const InputEventCallback& callback)
{
    s_GlobalCallbacks.push_back(callback);
}

void DekiInput::ClearEventCallbacks()
{
    s_GlobalCallbacks.clear();
}

void DekiInput::Update()
{
    if (!s_Initialized)
    {
        return;
    }

    // Update all active input packages
    for (auto& [name, input] : s_ActiveInputs)
    {
        if (input && input->IsInitialized())
        {
            input->Update();
        }
    }
}

bool DekiInput::GetPointerPosition(int32_t* x, int32_t* y)
{
    if (!s_Initialized || !x || !y)
    {
        return false;
    }

    // Try to get position from first available input
    for (auto& [name, input] : s_ActiveInputs)
    {
        if (input && input->IsInitialized())
        {
            if (input->GetPointerPosition(x, y))
            {
                return true;
            }
        }
    }

    return false;
}

bool DekiInput::IsKeyPressed(uint32_t key)
{
    if (!s_Initialized)
    {
        return false;
    }

    // Check if key is pressed in any active input
    for (auto& [name, input] : s_ActiveInputs)
    {
        if (input && input->IsInitialized())
        {
            if (input->IsKeyPressed(key))
            {
                return true;
            }
        }
    }

    return false;
}

bool DekiInput::ShouldExit()
{
    return s_ShouldExit;
}

void DekiInput::SetShouldExit(bool exit)
{
    s_ShouldExit = exit;
}

std::vector<std::string> DekiInput::GetActiveInputs()
{
    std::vector<std::string> names;
    for (const auto& [name, input] : s_ActiveInputs)
    {
        names.push_back(name);
    }
    return names;
}

void DekiInput::DistributeEvent(const InputEvent& event)
{
    // Handle AppQuit events
    if (event.type == InputEventType::AppQuit)
    {
        s_ShouldExit = true;
        DEKI_LOG_INTERNAL("DekiInput: AppQuit event received");
    }

    // Distribute event to all registered global callbacks
    for (const auto& callback : s_GlobalCallbacks)
    {
        if (callback)
        {
            callback(event);
        }
    }
}

}  // namespace DekiInput
