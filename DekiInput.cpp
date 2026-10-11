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

    if (s_ActiveInputs.find(name) != s_ActiveInputs.end())
    {
        DEKI_LOG_INTERNAL("Input package '%s' already registered", name.c_str());
        return true;
    }

    // Register internal callback to distribute events, tagged with the name
    // the source is known by. The map's key lives as long as the entry.
    auto entry = s_ActiveInputs.emplace(name, std::move(input)).first;
    const char* source = entry->first.c_str();
    entry->second->RegisterEventCallback(
        [source](const InputEvent& event)
        {
            InputEvent tagged = event;
            tagged.source = source;
            DistributeEvent(tagged);
        });
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

size_t DekiInput::RegisterEventCallback(const InputEventCallback& callback)
{
    // A slot a removed callback left, else a new one: ids stay valid, as
    // they are positions.
    for (size_t i = 0; i < s_GlobalCallbacks.size(); ++i)
    {
        if (!s_GlobalCallbacks[i])
        {
            s_GlobalCallbacks[i] = callback;
            return i;
        }
    }
    s_GlobalCallbacks.push_back(callback);
    return s_GlobalCallbacks.size() - 1;
}

void DekiInput::UnregisterEventCallback(size_t id)
{
    if (id < s_GlobalCallbacks.size())
    {
        s_GlobalCallbacks[id] = nullptr;
    }
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
    if (event.type == InputEventType::AppQuit)
    {
        s_ShouldExit = true;
        DEKI_LOG_INTERNAL("DekiInput: AppQuit event received");
    }

    // By position, as a callback may add or remove callbacks; each is called
    // through a copy, so one that removes itself is not destroyed while it
    // runs.
    for (size_t i = 0; i < s_GlobalCallbacks.size(); ++i)
    {
        const InputEventCallback callback = s_GlobalCallbacks[i];
        if (callback)
        {
            callback(event);
        }
    }
}

}  // namespace DekiInput
